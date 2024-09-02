#include <Arduino.h>
#include <Wire.h>

#include "src/game_state.h"
#include "src/display.h"

#define USE_DISPLAY             DISPLAY_DOUBLE_LCD
#define USE_LEDS
#define USE_BUZZER

#ifdef USE_BUZZER
#include "buzzer.h"
#endif

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
    #ifdef USE_LEDS
    pinMode(PLAYER1_LED_PIN, OUTPUT);
    pinMode(PLAYER2_LED_PIN, OUTPUT);
    analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
    analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
    #endif
    #ifdef USE_BUZZER
    pinMode(BUZZER_PIN, OUTPUT);
    #endif
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
    // if we aren't paused, then pause
    if (!gs->clock_mode == CM_PAUSED) {
        gs->pause();
        buttonPresses[CENTER_IDX] = 1;
        #ifdef USE_LEDS
        analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
        analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
        #endif
        #ifdef USE_BUZZER
        beep(BE_PAUSE);
        #endif
        return true;
    }

    // If the button has been hit three times, then reset
    if (buttonPresses[CENTER_IDX] == 3) {
        gs->clock_mode = CM_SELECT_SETTINGS;
        gs->reset();
        #ifdef USE_BUZZER
        beep(BE_RESET);
        #endif
        return true;
    }

    // If the button has been hit 3+ times, then cycle to the next settings
    if (buttonPresses[CENTER_IDX] >= 4) {
        gs->clock_mode = CM_SELECT_SETTINGS;
        selected_game_settings = (selected_game_settings + 1) % GAME_SETTINGS_LEN;
        gs->settings = &(all_game_settings[selected_game_settings]);
        gs->reset();
        #ifdef USE_BUZZER
        beep(BE_SELECT_SETTINGS);
        #endif
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

    // if the game is not paused, or it isn't my turn, then my button does nothing
    if (gs->clock_mode != CM_PAUSED && gs->whoseTurn != playerIndex) {
        return false;
    }

    // Otherwise, we change turns.
    gs->setTurn((playerIndex == PLAYER1_IDX) ? PLAYER2_IDX : PLAYER1_IDX);
    #ifdef USE_LEDS
    analogWrite((playerIndex == PLAYER1_IDX) ? PLAYER1_LED_PIN : PLAYER2_LED_PIN, LED_OFF_LEVEL);
    analogWrite((playerIndex == PLAYER1_IDX) ? PLAYER2_LED_PIN : PLAYER1_LED_PIN, LED_ON_LEVEL);
    #endif
    #ifdef USE_BUZZER
    if (gs->settings->turnBeep) {
        beep(BE_TURN_CHANGE);
    }
    #endif
    return true;
}

/*
bool handlePlayer1Button(GameState *gs, int buttonState, unsigned long now) {
    if (now - lastDebounceTime[PLAYER1_IDX] < DEBOUNCE_DELAY) {
        return;
    }
    if (buttonState == lastState[PLAYER1_IDX]) {
        return;
    }
    lastState[PLAYER1_IDX] = buttonState;
    lastDebounceTime[PLAYER1_IDX] = now;
    if (buttonState == HIGH) {
        // handle released
        if (gs->clock_mode == CM_PAUSED || gs->curr_player_state == &(gs->player_states[0])) {
            gs->setTurn(PLAYER_2);
            #ifdef USE_LEDS
            analogWrite(PLAYER1_LED_PIN, LED_OFF_LEVEL);
            analogWrite(PLAYER2_LED_PIN, LED_ON_LEVEL);
            #endif
            #ifdef USE_BUZZER
            if (gs->settings->turnBeep) {
                singleBeep();
            }
            #endif
        }
    }
}
*/

/*
void handlePlayer2Button(GameState *gs, int buttonState, unsigned long now) {
    if (now - lastDebounceTime[PLAYER2_IDX] < DEBOUNCE_DELAY) {
        return;
    }
    if (buttonState == lastState[PLAYER2_IDX]) {
        return;
    }
    lastState[PLAYER2_IDX] = buttonState;
    lastDebounceTime[PLAYER2_IDX] = now;
    if (buttonState == HIGH) {
        // handle released
        if (gs->clock_mode == CM_PAUSED || gs->curr_player_state == &(gs->player_states[1])) {
            gs->setTurn(PLAYER_1);
            #ifdef USE_LEDS
            analogWrite(PLAYER1_LED_PIN, LED_ON_LEVEL);
            analogWrite(PLAYER2_LED_PIN, LED_OFF_LEVEL);
            #endif
            #ifdef USE_BUZZER
            if (gs->settings->turnBeep) {
                singleBeep();
            }
            #endif
        }
    }
}
*/

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

    // If the decrement in time is less than or equal to the remaining grace
    // period, decrement only the grace period and return true (the counter was
    // modified)
    if (gs->curr_player_state->gracePeriodMillis >= decr) {
        gs->curr_player_state->gracePeriodMillis -= decr;
        return true;
    } else if (gs->curr_player_state->gracePeriodMillis > 0) {
        // If the decrement in time is greater than the remaining grace period, subtract
        // what you can of the decrement, indicate that the counters were modified, and
        // continue to the next step.
        decr -= gs->curr_player_state->gracePeriodMillis;
        gs->curr_player_state->gracePeriodMillis = 0;
        countersModified = true;
    }

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
        #ifdef USE_BUZZER
        if (gs->settings->flagBeep) {
            beep(BE_FLAG);
        }
        #endif // USE_BUZZER
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
    // 1. the game mode has changed (ex., player turn changed), or
    // 2. the timers have changed, and it's been a PRINT_INTERVAL since our last print
    if (gameModeChanged || (timersChanged && (now - lastPrinted) >= PRINT_INTERVAL)) {
        display.renderGameState(&game_state);
        lastPrinted = now;
    }
}