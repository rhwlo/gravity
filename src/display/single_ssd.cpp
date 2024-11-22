#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "../display.h"
#include "single_ssd.h"

SSD1306Display::SSD1306Display(void) :
    ssd(128, 64, &Wire, -1) {
}
// SSD1306Display::SSD1306Display(Adafruit_SSD1306 *d) {
//     ssd = ssd;
// }

void SSD1306Display::begin(void) {
    ssd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void SSD1306Display::prettyPrintTime(unsigned long time) {
    unsigned long remaining_time = time;
    if (remaining_time >= HOUR_MILLIS) {
        ssd.print((int) (remaining_time / HOUR_MILLIS), 10);
    } else {
        ssd.print("0");
    }
    remaining_time %= HOUR_MILLIS;

    ssd.print(':');

    if (remaining_time >= MINUTE_MILLIS * 10) {
        ssd.print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else if (remaining_time >= MINUTE_MILLIS) {
        ssd.print('0');
        ssd.print((int) (remaining_time / MINUTE_MILLIS), 10);
    } else {
        ssd.print("00");
    }
    remaining_time %= MINUTE_MILLIS;

    ssd.print(':');

    if (remaining_time >= SECOND_MILLIS * 10) {
        ssd.print((int) (remaining_time / SECOND_MILLIS), 10);
    } else if (remaining_time >= SECOND_MILLIS) {
        ssd.print('0');
        ssd.print((int) (remaining_time / SECOND_MILLIS), 10);
    } else {
        ssd.print("00");
    }
}

void SSD1306Display::specialToggle(void) {
    return;
}

void SSD1306Display::renderGameState(GameState *game_state) {
    uint16_t white_fg, white_bg;
    ssd.clearDisplay();
    ssd.setCursor(0,0);
    ssd.setTextSize(2);
    if (game_state->whoseTurn == PLAYER_1) {
        white_fg = SSD1306_BLACK;
        white_bg = SSD1306_WHITE;
    } else {
        white_fg = SSD1306_WHITE;
        white_bg = SSD1306_BLACK;
    }
    ssd.fillRect(0, 0, 128, 16, white_bg);
    ssd.setTextColor(white_fg);
    if (game_state->player_states[PLAYER_1].outOfTime) {
        ssd.println("0:00:00 !");
    } else {
        prettyPrintTime(game_state->player_states[PLAYER_1].remainingMillis);
    }
    ssd.println();
    ssd.fillRect(ssd.getCursorX(), ssd.getCursorY(), 128, 16, white_fg);
    ssd.setTextColor(white_bg);
    if (game_state->player_states[PLAYER_2].outOfTime) {
        ssd.println("0:00:00 !");
    } else {
        prettyPrintTime(game_state->player_states[PLAYER_2].remainingMillis);
    }
    ssd.println();
    ssd.display();
}