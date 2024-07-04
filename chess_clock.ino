#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "game_state.h"
#include "display.h"


#define PLAYER1_BUTTON_PIN      A0
#define PLAYER2_BUTTON_PIN      A1
#define PAUSE_BUTTON_PIN        A2
#define P1_BUTTON(x)            x & 0b001
#define P2_BUTTON(x)            x & 0b010
#define PAUSE_BUTTON(x)         x & 0b100
#define DEBOUNCE_DELAY          50
#define PRINT_INTERVAL          500

unsigned long lastDebounceTime = 0;
unsigned long lastIncr = 0;
unsigned long lastPrinted = 0;

GameState game_state = GameState(&standard_settings);
//SerialDisplay display = SerialDisplay();
Adafruit_SSD1306 sdisplay(128, 64, &Wire, -1);
SSD1306Display display(&sdisplay);

void setup()
{
    unsigned long now = millis();
    lastDebounceTime = now;
    lastIncr = now;
    lastPrinted = now;
    pinMode(PLAYER1_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PLAYER2_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PAUSE_BUTTON_PIN,   INPUT_PULLUP);
    display.begin();
}

void handleButtonReads(GameState *gs) {
    unsigned long now = millis();
    if ((now - lastDebounceTime) < DEBOUNCE_DELAY) {
        return;
    }
    lastDebounceTime = now;

    if (!digitalRead(PAUSE_BUTTON_PIN)) {
        gs->pause();
    } else if (!digitalRead(PLAYER1_BUTTON_PIN)) {
        gs->setTurn(PLAYER_BLACK);
    } else if (!digitalRead(PLAYER2_BUTTON_PIN)) {
        gs->setTurn(PLAYER_WHITE);
    }
}

void handleTimerIncr(GameState *gs) {
    unsigned long now = millis();
    if (!gs->paused) {
        gs->player_states[gs->whoseTurn].remainingMillis -= (now - lastIncr);
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
