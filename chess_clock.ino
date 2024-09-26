#include <Arduino.h>
#include <Wire.h>

#include "src/game_state.h"
#include "src/display.h"

#define USE_DISPLAY             DISPLAY_DOUBLE_LCD

#include "buzzer.h"

// pins
#define PLAYER1_BUTTON_PIN      A0
#define PLAYER1_LED_PIN         5
#define PLAYER2_BUTTON_PIN      A1
#define PLAYER2_LED_PIN         6
#define PAUSE_BUTTON_PIN        A3

// set LED_OFF_HIGH if the LED's non-output wire is wired to 5V rather than ground
#ifdef LED_OFF_HIGH
#define LED_OFF_LEVEL           255
#define LED_ON_LEVEL            220
#else
#define LED_OFF_LEVEL           0
#define LED_ON_LEVEL            35
#endif

#define DEBOUNCE_DELAY          100
#define PRINT_INTERVAL          200

#define PLAYER1_IDX 0
#define PLAYER2_IDX 1
#define CENTER_IDX  2

unsigned long lastDebounceTime[3] = {0, 0, 0};
int lastState[3] = {HIGH, HIGH, HIGH};
byte buttonPresses[3] = {0, 0, 0};
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

void setup()
{
    pinMode(PLAYER1_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PLAYER2_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PAUSE_BUTTON_PIN,   INPUT_PULLUP);
    display.begin();
    pinMode(PLAYER1_LED_PIN, OUTPUT);
    pinMode(PLAYER2_LED_PIN, OUTPUT);
    analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
    analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
    pinMode(BUZZER_PIN, OUTPUT);
    read_state_from_eeprom(&EEPROM);
}

/* handlePauseButton returns true if it modifies any state, false otherwise. */
bool handlePauseButton(GameState *gs, int buttonState, unsigned long now) {
    if (now - lastDebounceTime[CENTER_IDX] < DEBOUNCE_DELAY) {
        return false;
    }
    if (buttonState == lastState[CENTER_IDX]) {
        return false;
    }
    lastState[CENTER_IDX] = buttonState;
    lastDebounceTime[CENTER_IDX] = now;
    // For the moment, we don't do anything when the button is pressed -- just when released.
    if (buttonState == LOW) {
        return false;
    }
    // handle pause button released
    buttonPresses[CENTER_IDX]++;
    // if we are active, then pause
    if (gs->clock_mode == CM_ACTIVE) {
        gs->clock_mode = CM_PAUSED;
        buttonPresses[CENTER_IDX] = 1;
        analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
        analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
        beep(BE_PAUSE);
        return true;
    }

    // If the button has been hit three times, then reset
    if (buttonPresses[CENTER_IDX] == 3) {
        gs->clock_mode = CM_SELECT_SETTINGS;
        gs->reset();
        beep(BE_RESET);
        return true;
    }

    // If the button has been hit 3+ times, then cycle to the next settings
    if (buttonPresses[CENTER_IDX] >= 4) {
        gs->clock_mode = CM_SELECT_SETTINGS;
        selected_game_settings = (selected_game_settings + 1) % GAME_SETTINGS_LEN;
        gs->settings = &(all_game_settings[selected_game_settings]);
        gs->reset();
        beep(BE_SELECT_SETTINGS);
        write_state_to_eeprom(&EEPROM);
        return true;
    }
    return false;
}

/* handlePlayerButton returns true if it modifies any state, false otherwise. */
bool handlePlayerButton(
    GameState *gs, int buttonState, unsigned long now, uint8_t playerIndex
) {
    if (now - lastDebounceTime[playerIndex] < DEBOUNCE_DELAY) {
        return false;
    }
    if (buttonState == lastState[playerIndex]) {
        return false;
    }
    lastState[playerIndex] = buttonState;
    lastDebounceTime[playerIndex] = now;

    // We don't take action when buttons are pressed, only released
    if (buttonState == LOW) {
        return false;
    }
    // handle released button

    // if the game is active, and it isn't my turn, then my button does nothing
    if (gs->clock_mode == CM_ACTIVE && gs->whoseTurn != playerIndex) {
        return false;
    }

    // Otherwise, we change turns.
    gs->setTurn((playerIndex == PLAYER1_IDX) ? PLAYER2_IDX : PLAYER1_IDX);
    analogWrite(
        PLAYER1_LED_PIN,
        (gs->whoseTurn == PLAYER1_IDX) ? LED_ON_LEVEL : LED_OFF_LEVEL
    );
    analogWrite(
        PLAYER2_LED_PIN,
        (gs->whoseTurn == PLAYER2_IDX) ? LED_ON_LEVEL : LED_OFF_LEVEL
    );
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
        || handlePlayerButton(gs, digitalRead(PLAYER2_BUTTON_PIN), now, PLAYER2_IDX)
    );
}

/* handleTimerIncr returns true if it modifies any counters, false otherwise. */
bool handleTimerIncr(GameState *gs, unsigned long now) {
    bool countersModified = false;
    if (gs->clock_mode == CM_PAUSED) {
        return countersModified;
    }
    unsigned long decr = now - lastIncr;
    lastIncr = now;

    // if there's no normal time left, return immediately
    if (gs->curr_player_state->remainingMillis == 0) {
        return countersModified;
    } else if (gs->curr_player_state->remainingMillis >= decr) {
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

void loop()
{
    unsigned long now = millis();
    // Both handleButtonReads and handleTimerIncr will return true only if they've changed a value
    // in the game_state object.
    bool gameModeChanged = handleButtonReads(&game_state, now);
    bool timersChanged = handleTimerIncr(&game_state, now);
    // If nothing has changed, we can return early.
    if (!gameModeChanged && !timersChanged) {
        return;
    }
    // Otherwise, we re-render the display if either:
    // 0. we haven't rendered the display yet (lastPrinted == 0), or
    // 1. the game mode has changed (ex., player turn changed), or
    // 2. the timers have changed, and it's been a PRINT_INTERVAL since our last render
    if (lastPrinted == 0 || gameModeChanged || (timersChanged && (now - lastPrinted) >= PRINT_INTERVAL)) {
        display.renderGameState(&game_state);
        lastPrinted = now;
    }
}