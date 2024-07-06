#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "common.h"
#include "display.h"


SerialDisplay::SerialDisplay(Stream *s) {
    serial = s;
}

void SerialDisplay::begin(void) {
}

void SerialDisplay::prettyPrintTime(unsigned long time) {
    unsigned long remaining_time = time;
    if (remaining_time >= HOUR_MILLIS * 10) {
        serial->print((int) (remaining_time / HOUR_MILLIS), 10);
    } else if (remaining_time >= HOUR_MILLIS) {
        serial->print("0");
        serial->print((int) (remaining_time / HOUR_MILLIS), 10);
    } else {
        serial->print("00");
    }
    remaining_time %= HOUR_MILLIS;

    serial->print(':');

    if (remaining_time >= MINUTE_MILLIS * 10) {
        serial->print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else if (remaining_time >= MINUTE_MILLIS) {
        serial->print("0");
        serial->print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else {
        serial->print("00");
    }
    remaining_time %= MINUTE_MILLIS;

    serial->print(':');

    if (remaining_time >= SECOND_MILLIS * 10) {
        serial->print((int) (remaining_time / SECOND_MILLIS), 10);
    } else if (remaining_time >= SECOND_MILLIS) {
        serial->print("0");
        serial->print((int) (remaining_time / SECOND_MILLIS), 10);
    } else {
        serial->print("00");
    }
}

void SerialDisplay::renderGameState(GameState *game_state) {
    if (game_state->paused) {
        serial->println("PAUSED");
    }
    if (game_state->player_states[0].outOfTime || game_state->player_states[1].outOfTime) {
        serial->print("Out of time: ");
        if (game_state->player_states[PLAYER_WHITE].outOfTime) {
            serial->print("W ");
        }
        if (game_state->player_states[PLAYER_WHITE].outOfTime) {
            serial->print("B ");
        }
        serial->println();
    }
    if (game_state->whoseTurn == PLAYER_WHITE) {
        serial->println("White to move");
        prettyPrintTime(game_state->player_states[PLAYER_WHITE].remainingMillis);
        serial->println();
    } else {
        serial->println("Black to move");
        prettyPrintTime(game_state->player_states[PLAYER_BLACK].remainingMillis);
        serial->println();
    }
}

SSD1306Display::SSD1306Display(Adafruit_SSD1306 *d) {
    display = d;
}

void SSD1306Display::begin(void) {
    display->begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void SSD1306Display::prettyPrintTime(unsigned long time) {
    unsigned long remaining_time = time;
    if (remaining_time >= HOUR_MILLIS) {
        display->print((int) (remaining_time / HOUR_MILLIS), 10);
    } else {
        display->print("0");
    }
    remaining_time %= HOUR_MILLIS;

    display->print(':');

    if (remaining_time >= MINUTE_MILLIS * 10) {
        display->print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else if (remaining_time >= MINUTE_MILLIS) {
        display->print('0');
        display->print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else {
        display->print("00");
    }
    remaining_time %= MINUTE_MILLIS;

    display->print(':');

    if (remaining_time >= SECOND_MILLIS * 10) {
        display->print((int) (remaining_time / SECOND_MILLIS), 10);
    } else if (remaining_time >= SECOND_MILLIS) {
        display->print('0');
        display->print((int) (remaining_time / SECOND_MILLIS), 10);
    } else {
        display->print("00");
    }
}


void SSD1306Display::renderGameState(GameState *game_state) {
    uint16_t white_fg, white_bg;
    display->clearDisplay();
    display->setCursor(0,0);
    display->setTextSize(2);
    if (game_state->whoseTurn == PLAYER_WHITE) {
        white_fg = SSD1306_BLACK;
        white_bg = SSD1306_WHITE;
    } else {
        white_fg = SSD1306_WHITE;
        white_bg = SSD1306_BLACK;
    }
    display->fillRect(0, 0, 128, 16, white_bg);
    display->setTextColor(white_fg);
    if (game_state->player_states[PLAYER_WHITE].outOfTime) {
        display->println("0:00:00 !");
    } else {
        prettyPrintTime(game_state->player_states[PLAYER_WHITE].remainingMillis);
    }
    display->println();
    display->fillRect(display->getCursorX(), display->getCursorY(), 128, 16, white_fg);
    display->setTextColor(white_bg);
    if (game_state->player_states[PLAYER_BLACK].outOfTime) {
        display->println("0:00:00 !");
    } else {
        prettyPrintTime(game_state->player_states[PLAYER_BLACK].remainingMillis);
    }
    display->println();
    display->display();
}