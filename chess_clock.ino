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

#define LED_LOW                 220
#define DEBOUNCE_DELAY          100
#define PRINT_INTERVAL          500

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
    analogWrite(PLAYER1_LED_PIN, 255);
    analogWrite(PLAYER2_LED_PIN, 255);
    #endif
    #ifdef USE_BUZZER
    pinMode(BUZZER_PIN, OUTPUT);
    #endif
}

void handlePauseButton(GameState *gs, int buttonState, unsigned long now) {
    if (now - lastDebounceTime[CENTER_IDX] < DEBOUNCE_DELAY) {
        return;
    }
    if (buttonState == lastState[CENTER_IDX]) {
        return;
    }
    lastState[CENTER_IDX] = buttonState;
    lastDebounceTime[CENTER_IDX] = now;
    if (buttonState == HIGH) {
        // handle pause button released
        buttonPresses[CENTER_IDX]++;
        // if we aren't paused, then pause
        if (!gs->clock_mode == PAUSED) {
            gs->pause();
            buttonPresses[CENTER_IDX] = 1;
            #ifdef USE_LEDS
            analogWrite(PLAYER1_LED_PIN, 255);
            analogWrite(PLAYER2_LED_PIN, 255);
            #endif
            #ifdef USE_BUZZER
            singleBeep();
            #endif
            return;
        }
        if (buttonPresses[CENTER_IDX] == 3) {
            gs->reset();
            #ifdef USE_BUZZER
            chirpFifth();
            #endif
        } else if (buttonPresses[CENTER_IDX] >= 4) {
            gs->clock_mode = SELECT_SETTINGS;
            selected_game_settings++;
            selected_game_settings %= GAME_SETTINGS_LEN;
            gs->settings = &(all_game_settings[selected_game_settings]);
            gs->reset();
            #ifdef USE_BUZZER
            chirpFifth();
            #endif
        }
    } // else { } // handle button pressed?
}

void handlePlayer1Button(GameState *gs, int buttonState, unsigned long now) {
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
        if (gs->clock_mode == PAUSED || gs->curr_player_state == &(gs->player_states[0])) {
            gs->setTurn(PLAYER_2);
            #ifdef USE_LEDS
            analogWrite(PLAYER1_LED_PIN, 255);
            analogWrite(PLAYER2_LED_PIN, LED_LOW);
            #endif
            #ifdef USE_BUZZER
            if (gs->settings->turnBeep) {
                singleBeep();
            }
            #endif
        }
    }
}


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
        if (gs->clock_mode == PAUSED || gs->curr_player_state == &(gs->player_states[1])) {
            gs->setTurn(PLAYER_1);
            #ifdef USE_LEDS
            analogWrite(PLAYER1_LED_PIN, LED_LOW);
            analogWrite(PLAYER2_LED_PIN, 255);
            #endif
            #ifdef USE_BUZZER
            if (gs->settings->turnBeep) {
                singleBeep();
            }
            #endif
        }
    }
}

void handleButtonReads(GameState *gs) {
    unsigned long now = millis();

    handlePauseButton(gs, digitalRead(PAUSE_BUTTON_PIN), now);
    handlePlayer1Button(gs, digitalRead(PLAYER1_BUTTON_PIN), now);
    handlePlayer2Button(gs, digitalRead(PLAYER2_BUTTON_PIN), now);
}

void handleTimerIncr(GameState *gs) {
    unsigned long now = millis();
    bool wasZero = false;
    if (gs->clock_mode != PAUSED) {
        if (gs->curr_player_state->gracePeriodMillis > 0) {
            gs->curr_player_state->gracePeriodMillis -= min(
                (now - lastIncr),
                (gs->curr_player_state->gracePeriodMillis)
            );
        } else {
            wasZero = (gs->curr_player_state->remainingMillis == 0);
            gs->curr_player_state->remainingMillis -= min(
                (now - lastIncr),
                (gs->curr_player_state->remainingMillis)
            );
            if (gs->curr_player_state->remainingMillis == 0 && !wasZero) {
                gs->curr_player_state->outOfTime = true;
                #ifdef USE_BUZZER
                if (gs->settings->flagBeep) {
                    tripleBeep();
                }
                #endif // USE_BUZZER
            }
        }
    }
    lastIncr = now;
}

void loop()
{
    unsigned long now = millis();
    handleButtonReads(&game_state);
    handleTimerIncr(&game_state);
    if ((now - lastPrinted) >= PRINT_INTERVAL) {
        display.renderGameState(&game_state);
        lastPrinted = now;
    }
}
