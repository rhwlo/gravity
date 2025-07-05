#include <Arduino.h>
#include <Wire.h>
#include <extEEPROM.h>

#define USE_EEPROM 1

#include "src/game_state.h"
#include "src/display.h"

#define USE_DISPLAY DISPLAY_DOUBLE_LCD

#include "chess_clock.h"
#include "buzzer.h"

unsigned long lastStateChangeTime[3] = { 0, 0, 0 };
unsigned long lastStateChange[3] = { 0, 0, 0 };
int lastState[3] = { HIGH, HIGH, HIGH };
byte buttonPresses[3] = { 0, 0, 0 };
bool ignoreNextRelease[3] = { false, false, false };
unsigned long lastIncr = 0;
unsigned long lastPrinted = 0;

GameState game_state = GameState(&all_game_settings[selected_game_settings]);
#if USE_DISPLAY == DISPLAY_SERIAL

#include "src/display/serial.h"
SerialDisplay display = SerialDisplay(&Serial);

#elif USE_DISPLAY == DISPLAY_SINGLE_OLED

#include "src/display/single_ssd.h"
SSD1306Display display;

#elif USE_DISPLAY == DISPLAY_DOUBLE_LCD

#include "src/display/double_lcd0216.h"

LCDDisplay display(PCF8574_ADDR_0, PCF8574_ADDR_1);
#endif

extEEPROM eeprom(kbits_2, 1, 8);

void blinkForeverForError(uint8_t code, int status) {
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

void displayAndWait(const char *message) {
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
  blinkForeverForError(1, eeprom.begin(extEEPROM::twiClock400kHz));

  // Read settings from EEPROM; blink forever for error if something goes wrong.
  status = read_settings_from_eeprom(&eeprom);
  if (status == GS_ERR_VALUE_MISMATCH || status == EEPROM_ADDR_ERR) {
    if (status == GS_ERR_VALUE_MISMATCH) {
      displayAndWait("Error reading from EEPROM: Value mismatch");
    } else {
      displayAndWait("Error reading from EEPROM: Address Error");
    }
    delay(DEBOUNCE_DELAY);
    load_default_settings();
  } else {
    blinkForeverForError(2, status);
  }

  // Initialize (reset) the game state
  game_state.reset();
}

/* handlePauseButton returns true if it modifies any state, false otherwise. */
bool handlePauseButton(GameState *gs, int buttonState, unsigned long now) {

  if (
    // Return false (unchanged) early if:
    // 1. we aren't past the debounce delay, or
    (now - lastStateChangeTime[CENTER_IDX] < DEBOUNCE_DELAY)
    // 2. the buttonState hasn't changed from what was last recorded
    || (buttonState == lastState[CENTER_IDX])) {
    return false;
  }

  // Preserve the prior state change time (i.e., the state change before this one)
  unsigned long prevStateChangeTime = lastStateChangeTime[CENTER_IDX];

  // Update the button state and the lastStateChangeTime
  lastState[CENTER_IDX] = buttonState;
  lastStateChangeTime[CENTER_IDX] = now;

  // For the moment, we don't do anything when the button is pressed -- just when released.
  if (buttonState == LOW) {
    return false;
  }

  // handle pause button released

  if (ignoreNextRelease[CENTER_IDX]) {
    ignoreNextRelease[CENTER_IDX] = false;
    return false;
  }

  // long press
  if (now - prevStateChangeTime >= LONG_PRESS_DELAY) {
    // long press : SELECT_SETTINGS -> EDIT_SETTINGS
    if (gs->clock_mode == CM_SELECT_SETTINGS) {
      gs->clock_mode = CM_EDIT_SETTINGS;
      gs->option_index = 0;
      buttonPresses[CENTER_IDX] = 0;
      beep(BE_EDIT_SETTINGS);
      return true;
      // long press, save changes : EDIT_SETTINGS -> SELECT_SETTINGS
    } else if (gs->clock_mode == CM_EDIT_SETTINGS) {
      gs->clock_mode = CM_SELECT_SETTINGS;
      buttonPresses[CENTER_IDX] = 0;
      gs->option_index = -1;
      gs->reset();
      blinkForeverForError(3, write_settings_to_eeprom(&eeprom));
      beep(BE_SAVE_SETTINGS);
      return true;
    }
    // if there's no defined behavior for a long press (i.e., we're not in SELECT SETTINGS or
    // EDIT SETTINGS), it behaves like a short press.
  }
  // short press
  buttonPresses[CENTER_IDX]++;

  // 1x short press : ACTIVE -> PAUSED                      "pause"
  if (gs->clock_mode == CM_ACTIVE) {
    gs->clock_mode = CM_PAUSED;
    // make sure the counter is at 1
    buttonPresses[CENTER_IDX] = 1;
    analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
    analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
    beep(BE_PAUSE);
    return true;
  }

  // 3x short press (or more) : PAUSED -> SELECT_SETTINGS   "reset"
  if (gs->clock_mode == CM_PAUSED && buttonPresses[CENTER_IDX] > 2) {
    gs->clock_mode = CM_SELECT_SETTINGS;
    gs->reset();
    beep(BE_RESET);
    buttonPresses[CENTER_IDX] = 0;
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
    gs->option_index++;
    gs->option_index %= OI_COUNT;
    return true;
  }

  return false;
}

/* handlePlayerButton returns true if it modifies any state, false otherwise. */
bool handlePlayerButton(
  GameState *gs, int buttonState, unsigned long now, uint8_t playerIndex) {
  if (now - lastStateChangeTime[playerIndex] < DEBOUNCE_DELAY) {
    return false;
  }
  if (buttonState == lastState[playerIndex]) {
    return false;
  }
  lastState[playerIndex] = buttonState;
  lastStateChangeTime[playerIndex] = now;

  // We don't take action when buttons are pressed, only released
  if (buttonState == LOW) {
    return false;
  }
  // handle released button

  // if the game is active, and it isn't my turn, then my button does nothing
  if (gs->clock_mode == CM_ACTIVE && gs->whoseTurn != playerIndex) {
    return false;
  }

  // if the center button is pressed, treat this as a "special toggle"
  if (lastState[CENTER_IDX] == LOW) {
    ignoreNextRelease[CENTER_IDX] = true;
    buttonPresses[playerIndex]++;
    if (buttonPresses[playerIndex] >= 3) {
      beep(BE_SPECIAL_TOGGLE);
      display.specialToggle();
      buttonPresses[playerIndex] = 0;
    }
    return true;
  }

  if (ignoreNextRelease[playerIndex]) {
    ignoreNextRelease[playerIndex] = false;
    buttonPresses[playerIndex] = 0;
    return false;
  }

  // if we're in settings editing mode, handle that
  if (gs->clock_mode == CM_EDIT_SETTINGS) {
    game_settings_t *curr_settings = &all_game_settings[selected_game_settings];
    switch (gs->option_index) {
      case -1:
        return false;
      case OI_FLAG_BEEP:
        curr_settings->flagBeep = !curr_settings->flagBeep;
        return true;
      case OI_TURN_BEEP:
        curr_settings->turnBeep = !curr_settings->turnBeep;
        return true;
      case OI_P1_HOURS:
        curr_settings->player_settings[PLAYER1_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 3600000;
        return true;
      case OI_P2_HOURS:
        curr_settings->player_settings[PLAYER2_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 3600000;
        return true;
      case OI_P1_TEN_MINUTES:
        curr_settings->player_settings[PLAYER1_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 600000;
        return true;
      case OI_P1_MINUTES:
        curr_settings->player_settings[PLAYER1_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 60000;
        return true;
      case OI_P2_TEN_MINUTES:
        curr_settings->player_settings[PLAYER2_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 600000;
        return true;
      case OI_P2_MINUTES:
        curr_settings->player_settings[PLAYER2_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 60000;
        return true;
      case OI_P1_TEN_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        return true;
      case OI_P1_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        return true;
      case OI_P2_TEN_SECONDS:
        curr_settings->player_settings[PLAYER2_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        return true;
      case OI_P2_SECONDS:
        curr_settings->player_settings[PLAYER2_IDX].totalMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        return true;
      case OI_TURN_TEN_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].perTurnIncrMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        curr_settings->player_settings[PLAYER2_IDX].perTurnIncrMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 10000;
        return true;
      case OI_TURN_SECONDS:
        curr_settings->player_settings[PLAYER1_IDX].perTurnIncrMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        curr_settings->player_settings[PLAYER2_IDX].perTurnIncrMillis +=
          ((playerIndex == PLAYER2_IDX) ? (-1) : (1)) * 1000;
        return true;
      default:
        return false;
    }
  }

  // Otherwise, we change turns.
  gs->setTurn((playerIndex == PLAYER1_IDX) ? PLAYER2_IDX : PLAYER1_IDX);
  analogWrite(
    PLAYER1_LED_PIN,
    (gs->whoseTurn == PLAYER1_IDX) ? LED_ON_LEVEL : LED_OFF_LEVEL);
  analogWrite(
    PLAYER2_LED_PIN,
    (gs->whoseTurn == PLAYER2_IDX) ? LED_ON_LEVEL : LED_OFF_LEVEL);
  // Timer starts now!
  lastIncr = now;
  if (gs->settings->turnBeep) {
    beep(BE_TURN_CHANGE);
  }
  return true;
}

bool handleButtonReads(GameState *gs, unsigned long now) {
  return (handlePauseButton(gs, digitalRead(PAUSE_BUTTON_PIN), now)
          || handlePlayerButton(gs, digitalRead(PLAYER1_BUTTON_PIN), now, PLAYER1_IDX)
          || handlePlayerButton(gs, digitalRead(PLAYER2_BUTTON_PIN), now, PLAYER2_IDX));
}

/* handleTimerIncr returns true if it modifies any counters, false otherwise. */
bool handleTimerIncr(GameState *gs, unsigned long now) {
  bool countersModified = false;
  if (gs->clock_mode != CM_ACTIVE) {
    return countersModified;
  }
  unsigned long decr = now - lastIncr;
  lastIncr = now;

  // if there's no normal time left, return immediately
  if (gs->curr_player_state->remainingMillis == 0) {
    return countersModified;
  } else if (gs->curr_player_state->remainingMillis > decr) {
    // otherwise, if there's more remaining time than the decrement, decrement it
    // as usual and return true.
    gs->curr_player_state->remainingMillis -= decr;
    return true;
  } else {
    // otherwise, if there's less remaining time than the decrement, go the
    // "out of time" route:
    gs->curr_player_state->remainingMillis = 0;
    gs->curr_player_state->outOfTime = true;
    if (gs->settings->flagBeep) {
      beep(BE_FLAG);
    }
    return true;
  }
}

void loop() {
  unsigned long now = millis();
  // Both handleButtonReads and handleTimerIncr will return true only if they've changed a value
  // in the game_state object.
  bool gameModeChanged = handleButtonReads(&game_state, now);
  bool timersChanged = handleTimerIncr(&game_state, now);

  if (
    // We render the display if:
    // 1. we're in SELECT_SETTINGS or EDIT_SETTINGS modes
    (game_state.clock_mode == CM_SELECT_SETTINGS || game_state.clock_mode == CM_EDIT_SETTINGS)
    // 2. the game mode has changed (ex., player turn changed), or
    || gameModeChanged
    // 3. the timers have changed, and it's been a PRINT_INTERVAL since our last render
    || (timersChanged && (now - lastPrinted) >= PRINT_INTERVAL)
    // 4. the timers have changed, and the current player has no time remaining
    || (timersChanged && game_state.curr_player_state->remainingMillis == 0)) {
    display.renderGameState(&game_state);
    lastPrinted = now;
  }
}
