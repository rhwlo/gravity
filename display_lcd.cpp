#include <LCD_I2C.h>
#include "game_state.h"
#include "display_lcd.h"

// Real Bold digits courtesy of https://github.com/upiir/character_display_big_digits
const uint8_t real_bold_digits[10][4] = {
    { 0x20, 0x20, 0x03, 0x02 },
    { 0x20, 0x05, 0xFE, 0x05 },
    { 0x00, 0x02, 0x03, 0x01 },
    { 0x00, 0x02, 0x01, 0x02 },
    { 0x07, 0x01, 0x20, 0x05 },
    { 0x07, 0x00, 0x01, 0x02 },
    { 0x07, 0x00, 0x06, 0x02 },
    { 0x00, 0x04, 0x20, 0x05 },
    { 0x03, 0x02, 0x03, 0x02 },
    { 0x03, 0x02, 0x01, 0x04 }
};

LCDDisplay::LCDDisplay(uint8_t addr_1, uint8_t addr_2) :
    player_1(addr_1, COLS, ROWS),
    player_2(addr_2, COLS, ROWS) {
    for (uint8_t i = 0; i < ROWS; i++) {
        for (uint8_t j = 0; j < COLS; i++) {
            last_displayed[0][i][j] = CHAR_BLANK;
            last_displayed[1][i][j] = CHAR_BLANK;
        }
    }
}

void LCDDisplay::begin(void) {
    player_1.begin();
    player_2.begin();
}

void pushDigit(char buffer[ROWS][COLS], uint8_t digit, uint8_t x_offset, uint8_t y_offset) {
    if (digit >= 0 && digit <= 9) {
        buffer[y_offset][x_offset] = real_bold_digits[digit][0];
        buffer[y_offset][x_offset + 1] = real_bold_digits[digit][1];
        buffer[y_offset + 1][x_offset] = real_bold_digits[digit][2];
        buffer[y_offset + 1][x_offset + 1] = real_bold_digits[digit][3];
    }
}

void makeDisplayBuffer(char buffer[2][16], unsigned long time) {
    unsigned long rt = time;
    uint8_t digit;

    buffer[0][2] = CHAR_COLON_TOP;
    buffer[1][2] = CHAR_COLON_BOTTOM;
    buffer[0][7] = CHAR_COLON_TOP;
    buffer[1][7] = CHAR_COLON_BOTTOM;

    // unit seconds
    rt /= SECOND_MILLIS;
    digit = rt % 10;
    pushDigit(buffer, digit, 10, 0);
    // tens of seconds
    digit = (rt % 60) / 10;
    pushDigit(buffer, digit, 8, 0);
    
    // unit minutes
    rt /= 60;
    digit = rt % 10;
    pushDigit(buffer, digit, 5, 0);
    // tens of minutes
    digit = (rt % 60) / 10;
    pushDigit(buffer, digit, 3, 0);

    // unit hours
    rt /= 60;
    digit = rt % 10;
    pushDigit(buffer, digit, 0, 0);
}

bool displayBuffersDiffer(char buf1[ROWS][COLS], char buf2[ROWS][COLS]) {
    uint8_t i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (buf1[i][j] != buf2[i][j]) {
                return true;
            }
        }
    }
    return false;
}

void applyDisplayBuffer(LCD_I2C *lcd, char old_buffer[ROWS][COLS], char buffer[ROWS][COLS]) {
    uint8_t i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (buffer[i][j] != old_buffer[i][j]) {
                lcd->setCursor(j, i);
                lcd->write(buffer[i][j]);
            }
        }
    }
}

void LCDDisplay::renderGameState(GameState *game_state) {
    char new_buffers[2][ROWS][COLS];
    uint8_t i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < ROWS; j++) {
            new_buffers[0][i][j] = CHAR_BLANK;
            new_buffers[1][i][j] = CHAR_BLANK;
        }
    }

    if (!game_state->player_states[0].outOfTime) {
        makeDisplayBuffer(new_buffers[0], game_state->player_states[0].remainingMillis);
    } else {
        strcpy(new_buffers[0][0], "out of time");
    }
    if (displayBuffersDiffer(new_buffers[0], last_displayed[0])) {
        applyDisplayBuffer(&player_1, last_displayed[0], new_buffers[0]);
        memcpy(last_displayed[0], new_buffers[0], ROWS * COLS * sizeof(new_buffers[0][0][0]));
    }

    if (!game_state->player_states[1].outOfTime) {
        makeDisplayBuffer(new_buffers[1], game_state->player_states[1].remainingMillis);
    } else {
        strcpy(new_buffers[1][0], "out of time");
    }

    if (displayBuffersDiffer(new_buffers[1], last_displayed[1])) {
        applyDisplayBuffer(&player_2, last_displayed[1], new_buffers[1]);
        memcpy(last_displayed[1], new_buffers[1], ROWS * COLS * sizeof(new_buffers[0][0][0]));
    }
}