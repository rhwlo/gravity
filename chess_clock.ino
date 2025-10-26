#include <Arduino.h>
#include <Wire.h>
#include <extEEPROM.h>

#define USE_EEPROM 1

#include "src/game_state.h"
#include "src/display.h"

#define USE_DISPLAY DISPLAY_DOUBLE_LCD_0108

#include "chess_clock.h"
#include "buzzer.h"

// TODO: refactor button presses into a struct to track this better
unsigned long last_state_changed_time[3] = { 0, 0, 0 };
int last_state[3] = { HIGH, HIGH, HIGH };
byte unprocessed_presses[3] = { 0, 0, 0 };
bool ignore_next_release[3] = { false, false, false };

unsigned long last_incremented_at = 0;
unsigned long last_printed_at = 0;

GameState game_state = GameState(&all_game_settings[selected_game_settings]);

#if USE_DISPLAY == DISPLAY_SERIAL

#include "src/display/serial.h"
SerialDisplay display = SerialDisplay(&Serial);

#elif USE_DISPLAY == DISPLAY_SINGLE_OLED

#include "src/display/single_ssd.h"
SSD1306Display display;

#elif USE_DISPLAY == DISPLAY_DOUBLE_LCD_0216

#include "src/display/double_lcd0216.h"

LCDDisplay display(PCF8574_ADDR_0, PCF8574_ADDR_1);

#elif USE_DISPLAY == DISPLAY_DOUBLE_LCD_0108

#include "src/display/double_lcd0108.h"

LCDDisplay8 display(PCF8574_ADDR_0, PCF8574_ADDR_1);

#endif

extEEPROM eeprom(kbits_2, 1, 8);

void blink_forever_for_error(uint8_t code, int status) {
  // if status == 0, return immediately; there was no error.
  if (status == 0) {
    return;
  }

  // if status is negative, turn it positive instead
  if (status < 0) {
    status *= -1;
  }

  // blink the left LED to indicate where the exception was raised,
  // then blink the right LED #status times
  // then wait for 1 second.
  while (1) {
    for (int i = 0; i < code; i++) {
      analogWrite(PLAYER2_LED_PIN, LED_ON_LEVEL);
      delay(300);
      analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
      delay(200);
    }
    for (int i = 0; i < status; i++) {
      analogWrite(PLAYER1_LED_PIN, LED_ON_LEVEL);
      delay(300);
      analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
      delay(200);
    }
    delay(1000);
  }
}

void display_and_wait(const char *message) {
  display.print(message);
  while (digitalRead(PAUSE_BUTTON_PIN) == HIGH
         && digitalRead(PLAYER1_BUTTON_PIN) == HIGH
         && digitalRead(PLAYER2_BUTTON_PIN) == HIGH) {
    delay(200);
  }
}

void setup() {
  int status;

  // initialize input & output pins
  pinMode(PLAYER1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAYER2_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAYER1_LED_PIN, OUTPUT);
  pinMode(PLAYER2_LED_PIN, OUTPUT);
  analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
  analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);

  display.begin();
  pinMode(BUZZER_PIN, OUTPUT);

  // Connect to EEPROM; blink forever for error if something goes wrong.
  blink_forever_for_error(1, eeprom.begin(extEEPROM::twiClock400kHz));

  // Read settings from EEPROM; print some things and blink forever for error if
  // something goes wrong.
  status = read_settings_from_eeprom(&eeprom);
  if (status != 0) {
    if (status == GS_ERR_VALUE_MISMATCH) {
      display_and_wait("Header mismatch");
    } else if (status == -EEPROM_ADDR_ERR) {
      display_and_wait("read out of bounds");
    } else {
      display.print("Error reading from EEPROM");
      blink_forever_for_error(2, status);
    }
    delay(DEBOUNCE_DELAY);
    load_default_settings();
  }

  // Initialize (reset) the game state
  game_state.reset();
}

/* handles a state changed the pause button's state changing;
 * returns: true if it modifies any state, false otherwise. */
bool handle_pause_button(GameState *gs, int button_state, unsigned long now) {

  if (
    // Return false (unchanged) early if:
    // 1. we aren't past the debounce delay, or
    (now - last_state_changed_time[CENTER_IDX] < DEBOUNCE_DELAY)
    // 2. the button_state hasn't changed from what was last recorded
    || (button_state == last_state[CENTER_IDX])) {
    return false;
  }

  // Preserve the prior state change time (i.e., the state change before this one)
  unsigned long prev_state_changed_time = last_state_changed_time[CENTER_IDX];

  // Update the last button state and the last state changed time
  last_state[CENTER_IDX] = button_state;
  last_state_changed_time[CENTER_IDX] = now;

  // For the moment, we don't do anything when the button is pressed -- just when released.
  if (button_state == LOW) {
    return false;
  }

  // handle pause button released

  if (ignore_next_release[CENTER_IDX]) {
    ignore_next_release[CENTER_IDX] = false;
    return false;
  }

  // long press
  if (now - prev_state_changed_time >= LONG_PRESS_DELAY) {
    // long press : SELECT_SETTINGS -> EDIT_SETTINGS
    if (gs->clock_mode == CM_SELECT_SETTINGS) {
      gs->option_index = OI_P2_HOURS;
      gs->clock_mode = CM_EDIT_SETTINGS;
      unprocessed_presses[CENTER_IDX] = 0;
      beep(BE_EDIT_SETTINGS);
      return true;
      // long press, save changes : EDIT_SETTINGS -> SELECT_SETTINGS
    } else if (gs->clock_mode == CM_EDIT_SETTINGS) {
      gs->clock_mode = CM_SELECT_SETTINGS;
      unprocessed_presses[CENTER_IDX] = 0;
      gs->option_index = -1;
      gs->reset();
      blink_forever_for_error(3, write_settings_to_eeprom(&eeprom));
      beep(BE_SAVE_SETTINGS);
      return true;
    }
    // if there's no defined behavior for a long press (i.e., we're not in SELECT SETTINGS or
    // EDIT SETTINGS), it behaves like a short press.
  }
  // short press
  unprocessed_presses[CENTER_IDX]++;

  // 1x short press : ACTIVE -> PAUSED                      "pause"
  if (gs->clock_mode == CM_ACTIVE) {
    gs->clock_mode = CM_PAUSED;
    // make sure the counter is at 1
    unprocessed_presses[CENTER_IDX] = 1;
    analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
    analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
    beep(BE_PAUSE);
    return true;
  }

  // 3x short press (or more) : PAUSED -> SELECT_SETTINGS   "reset"
  if (gs->clock_mode == CM_PAUSED && unprocessed_presses[CENTER_IDX] > 2) {
    gs->clock_mode = CM_SELECT_SETTINGS;
    gs->reset();
    beep(BE_RESET);
    unprocessed_presses[CENTER_IDX] = 0;
    return true;
  }

  // 1x short press : SELECT_SETTINGS -> SELECT_SETTINGS    "next preset"
  if (gs->clock_mode == CM_SELECT_SETTINGS) {
    selected_game_settings = (selected_game_settings + 1) % GAME_SETTINGS_LEN;
    gs->settings = &(all_game_settings[selected_game_settings]);
    gs->reset();
    beep(BE_SELECT_SETTINGS);
    return true;
  }

  // 1x short press : EDIT_SETTINGS -> EDIT_SETTINGS        "next option"
  if (gs->clock_mode == CM_EDIT_SETTINGS) {
    gs->option_index = (gs->option_index + 1) % OI_COUNT;
    return true;
  }

  return false;
}

/* handle press/release events for the player buttons;
 * returns: true if the state has changed, false otherwise. */
bool handle_player_button(
  GameState *gs, int button_state, unsigned long now, uint8_t player_index) {
  if (now - last_state_changed_time[player_index] < DEBOUNCE_DELAY) {
    return false;
  }
  if (button_state == last_state[player_index]) {
    return false;
  }
  last_state[player_index] = button_state;
  last_state_changed_time[player_index] = now;

  // We don't take action when buttons are pressed, only released
  if (button_state == LOW) {
    return false;
  }
  // handle released button

  // if the game is active, and it isn't my turn, then my button does nothing
  if (gs->clock_mode == CM_ACTIVE && gs->whose_turn != player_index) {
    return false;
  }

  // if the center button is pressed, treat this as a "special toggle"
  if (last_state[CENTER_IDX] == LOW) {
    ignore_next_release[CENTER_IDX] = true;
    unprocessed_presses[player_index]++;
    if (unprocessed_presses[player_index] >= 3) {
      beep(BE_SPECIAL_TOGGLE);
      display.special_toggle();
      unprocessed_presses[player_index] = 0;
    }
    return true;
  }

  if (ignore_next_release[player_index]) {
    ignore_next_release[player_index] = false;
    unprocessed_presses[player_index] = 0;
    return false;
  }

  // if we're in settings editing mode, handle that
  if (gs->clock_mode == CM_EDIT_SETTINGS) {
    GameSettings *curr_settings = &all_game_settings[selected_game_settings];
    switch (gs->option_index) {
      case -1:
        return false;
      case OI_FLAG_BEEP:
        curr_settings->flag_beep = !curr_settings->flag_beep;
        return true;
      case OI_TURN_BEEP:
        curr_settings->turn_beep = !curr_settings->turn_beep;
        return true;
      case OI_P1_HOURS:
        curr_settings->player_settings[PLAYER1_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 3600000;
        return true;
      case OI_P2_HOURS:
        curr_settings->player_settings[PLAYER2_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 3600000;
        return true;
      case OI_P1_TEN_MINUTES:
        curr_settings->player_settings[PLAYER1_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 600000;
        return true;
      case OI_P1_MINUTES:
        curr_settings->player_settings[PLAYER1_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 60000;
        return true;
      case OI_P2_TEN_MINUTES:
        curr_settings->player_settings[PLAYER2_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 600000;
        return true;
      case OI_P2_MINUTES:
        curr_settings->player_settings[PLAYER2_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 60000;
        return true;
      case OI_P1_TEN_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        return true;
      case OI_P1_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        return true;
      case OI_P2_TEN_SECONDS:
        curr_settings->player_settings[PLAYER2_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        return true;
      case OI_P2_SECONDS:
        curr_settings->player_settings[PLAYER2_IDX].total_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        return true;
      case OI_TURN_TEN_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].per_turn_incr_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        curr_settings->player_settings[PLAYER2_IDX].per_turn_incr_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        return true;
      case OI_TURN_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].per_turn_incr_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        curr_settings->player_settings[PLAYER2_IDX].per_turn_incr_millis +=
          ((player_index == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        return true;
      default:
        return false;
    }
  }

  // Otherwise, we change turns.
  gs->set_turn((player_index == PLAYER1_IDX) ? PLAYER2_IDX : PLAYER1_IDX);
  analogWrite(
    PLAYER1_LED_PIN,
    (gs->whose_turn == PLAYER1_IDX) ? LED_ON_LEVEL : LED_OFF_LEVEL);
  analogWrite(
    PLAYER2_LED_PIN,
    (gs->whose_turn == PLAYER2_IDX) ? LED_ON_LEVEL : LED_OFF_LEVEL);
  // Timer starts now!
  last_incremented_at = now;
  if (gs->settings->turn_beep) {
    beep(BE_TURN_CHANGE);
  }
  return true;
}

bool handle_button_reads(GameState *gs, unsigned long now) {
  return (handle_pause_button(gs, digitalRead(PAUSE_BUTTON_PIN), now)
          || handle_player_button(gs, digitalRead(PLAYER1_BUTTON_PIN), now, PLAYER1_IDX)
          || handle_player_button(gs, digitalRead(PLAYER2_BUTTON_PIN), now, PLAYER2_IDX));
}

/* handle the timer incrementing;
 * returns: true if any counters are modified; otherwise it returns false. */
bool handle_timer_incr(GameState *gs, unsigned long now) {
  bool counters_modified = false;
  if (gs->clock_mode != CM_ACTIVE) {
    return counters_modified;
  }
  unsigned long decr = now - last_incremented_at;
  last_incremented_at = now;

  // if there's no normal time left, return immediately
  if (gs->curr_player_state->remaining_millis == 0) {
    return counters_modified;
  } else if (gs->curr_player_state->remaining_millis > decr) {
    // otherwise, if there's more remaining time than the decrement, decrement it
    // as usual and return true.
    gs->curr_player_state->remaining_millis -= decr;
    return true;
  } else {
    // otherwise, if there's less remaining time than the decrement, go the
    // "out of time" route:
    gs->curr_player_state->remaining_millis = 0;
    gs->curr_player_state->out_of_time = true;
    if (gs->settings->flag_beep) {
      beep(BE_FLAG);
    }
    return true;
  }
}

void loop() {
  unsigned long now = millis();
  // Both hangle_button_reads and handle_timer_incr will return true only if they've changed a value
  // in the game_state object.
  bool game_mode_changed = handle_button_reads(&game_state, now);
  bool timers_changed = handle_timer_incr(&game_state, now);

  if (
    // We render the display if:
    // 1. we're in SELECT_SETTINGS or EDIT_SETTINGS modes
    (game_state.clock_mode == CM_SELECT_SETTINGS || game_state.clock_mode == CM_EDIT_SETTINGS)
    // 2. the game mode has changed (ex., player turn changed), or
    || game_mode_changed
    // 3. the timers have changed, and it's been a PRINT_INTERVAL since our last render
    || (timers_changed && (now - last_printed_at) >= PRINT_INTERVAL)
    // 4. the timers have changed, and the current player has no time remaining
    || (timers_changed && game_state.curr_player_state->remaining_millis == 0)) {
    display.render_game_state(&game_state);
    last_printed_at = now;
  }
}
