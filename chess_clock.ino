#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "display.h"
#include "game_state.h"

#define CHESS_OLED_DISPLAY
#define USE_LEDS
#define USE_BUZZER


// pins
#define PLAYER1_BUTTON_PIN      A0
#define PLAYER1_LED_PIN         6
#define PLAYER2_BUTTON_PIN      A1
#define PLAYER2_LED_PIN         5
#define PAUSE_BUTTON_PIN        A2
#define BUZZER_PIN              9

#define LED_HIGH                128
#define BUZZER_TONE             662  // Eb
#define DEBOUNCE_DELAY          50
#define PRINT_INTERVAL          500

unsigned long lastDebounceTime = 0;
unsigned long lastIncr = 0;
unsigned long lastPrinted = 0;

GameState game_state = GameState(&setting_blitz_30s_0);
#ifdef CHESS_SERIAL_DISPLAY
SerialDisplay display = SerialDisplay(&Serial);
#endif // CHESS_SERIAL_DISPLAY
#ifdef CHESS_OLED_DISPLAY
Adafruit_SSD1306 sdisplay(128, 64, &Wire, -1);
SSD1306Display display(&sdisplay);
#endif // CHESS_OLED_DISPLAY

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

void singleBeep(void) {
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
}

void tripleBeep(void) {
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
    delay(40);
    tone(BUZZER_PIN, 662);
    delay(110);
    noTone(BUZZER_PIN);
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
        gs->setTurn(PLAYER_BLACK);
        #ifdef USE_LEDS
        analogWrite(PLAYER1_LED_PIN, 0);
        analogWrite(PLAYER2_LED_PIN, LED_HIGH);
        #endif
    } else if (!digitalRead(PLAYER2_BUTTON_PIN)) {
        gs->setTurn(PLAYER_WHITE);
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
