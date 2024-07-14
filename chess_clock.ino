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
#define PLAYER1_LED_PIN         6
#define PLAYER2_BUTTON_PIN      A1
#define PLAYER2_LED_PIN         5
#define PAUSE_BUTTON_PIN        A2

#define LED_HIGH                128
#define DEBOUNCE_DELAY          50
#define PRINT_INTERVAL          500

unsigned long lastDebounceTime = 0;
unsigned long lastIncr = 0;
unsigned long lastPrinted = 0;

GameState game_state = GameState(&setting_blitz_30s_0);
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
    digitalWrite(PLAYER1_LED_PIN, LOW);
    digitalWrite(PLAYER2_LED_PIN, LOW);
    #endif
    #ifdef USE_BUZZER
    pinMode(BUZZER_PIN, OUTPUT);
    #endif
}


void handleButtonReads(GameState *gs) {
    unsigned long now = millis();
    if ((now - lastDebounceTime) < DEBOUNCE_DELAY) {
        return;
    }
    lastDebounceTime = now;

    if (!digitalRead(PAUSE_BUTTON_PIN)) {
        #ifdef USE_LEDS
        digitalWrite(PLAYER1_LED_PIN, LOW);
        digitalWrite(PLAYER2_LED_PIN, LOW);
        #endif
        #ifdef USE_BUZZER
        singleBeep();
        #endif
        gs->pause();
    } else if (!digitalRead(PLAYER1_BUTTON_PIN)) {
        gs->setTurn(PLAYER_1);
        #ifdef USE_LEDS
        analogWrite(PLAYER1_LED_PIN, 0);
        analogWrite(PLAYER2_LED_PIN, LED_HIGH);
        #endif
    } else if (!digitalRead(PLAYER2_BUTTON_PIN)) {
        gs->setTurn(PLAYER_2);
        #ifdef USE_LEDS
        analogWrite(PLAYER1_LED_PIN, LED_HIGH);
        analogWrite(PLAYER2_LED_PIN, 0);
        #endif
    }
}

void handleTimerIncr(GameState *gs) {
    unsigned long now = millis();
    bool wasZero = false;
    if (!gs->paused) {
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
