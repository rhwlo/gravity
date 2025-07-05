#include <LCD_I2C.h>
#include "../game_state.h"
#include "double_lcd0216.h"


#if FONT == FONT_REAL_BOLD
unsigned char special_00[8] = {B11111,B00000,B00000,B00000,B00000,B00000,B00000,B00000};
unsigned char special_01[8] = {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B11111};
unsigned char special_02[8] = {B11100,B01110,B00110,B00111,B00111,B00110,B01110,B11100};
unsigned char special_03[8] = {B00111,B01110,B01100,B11100,B11100,B01100,B01110,B00111};
unsigned char special_04[8] = {B11110,B00110,B00110,B00110,B01100,B01100,B11000,B11000};
unsigned char special_05[8] = {B11000,B11000,B11000,B11000,B11000,B11000,B11000,B11000};
unsigned char special_06[8] = {B11000,B11000,B11000,B11000,B11100,B01100,B01110,B00111};
unsigned char special_07[8] = {B00011,B00110,B00110,B01100,B01100,B11000,B11000,B11111};
#elif FONT == FONT_CLASSIC
unsigned char special_00[8] = {B11111,B11000,B00000,B00000,B00000,B00000,B00000,B00000};
unsigned char special_01[8] = {B00000,B00000,B00000,B00000,B00000,B00000,B11000,B11111};
unsigned char special_02[8] = {B11111,B00011,B00011,B00011,B00011,B00011,B00011,B11111};
unsigned char special_03[8] = {B11111,B11000,B11000,B11000,B11000,B11000,B11000,B11111};
unsigned char special_04[8] = {B00001,B00010,B00100,B00000,B00000,B00000,B00000,B00000};
unsigned char special_05[8] = {B11000,B11000,B11000,B11000,B11000,B11000,B11000,B11000};
unsigned char special_06[8] = {B11111,B00000,B00000,B00000,B00000,B00000,B00000,B11111};
unsigned char special_07[8] = {B11000,B11000,B11000,B11000,B11000,B11000,B11000,B11111};
#elif FONT == FONT_DASHED
unsigned char special_00[8] = {B11011,B00000,B00000,B00000,B00000,B00000,B00000,B00000};
unsigned char special_01[8] = {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B11011};
unsigned char special_02[8] = {B11011,B00001,B00000,B00001,B00001,B00000,B00001,B11011};
unsigned char special_03[8] = {B11011,B10000,B00000,B10000,B10000,B00000,B10000,B11011};
unsigned char special_04[8] = {B11011,B10000,B00000,B10000,B10000,B00000,B10000,B10000};
unsigned char special_05[8] = {B10000,B10000,B00000,B10000,B10000,B00000,B10000,B10000};
unsigned char special_06[8] = {B11011,B00000,B00000,B00000,B00000,B00000,B00000,B11011};
unsigned char special_07[8] = {B10000,B10000,B00000,B10000,B10000,B00000,B10000,B11011};
#elif FONT == FONT_WIDE
unsigned char special_00[8] = {B00111,B01111,B11111,B11111,B11111,B11111,B11111,B11111};
unsigned char special_01[8] = {B11111,B11111,B11111,B00000,B00000,B00000,B00000,B00000};
unsigned char special_02[8] = {B11100,B11110,B11111,B11111,B11111,B11111,B11111,B11111};
unsigned char special_03[8] = {B11111,B11111,B11111,B11111,B11111,B11111,B01111,B00111};
unsigned char special_04[8] = {B00000,B00000,B00000,B00000,B00000,B11111,B11111,B11111};
unsigned char special_05[8] = {B11111,B11111,B11111,B11111,B11111,B11111,B11110,B11100};
unsigned char special_06[8] = {B11111,B11111,B11111,B00000,B00000,B00000,B11111,B11111};
unsigned char special_07[8] = {B00000,B00000,B01100,B10110,B00101,B11111,B10100,B11111};
#endif

#if FONT == FONT_WIDE
const uint8_t font_digits[10][6] = {
#else
const uint8_t font_digits[10][4] = {
#endif
    DIGIT_0_MAP,
    DIGIT_1_MAP,
    DIGIT_2_MAP,
    DIGIT_3_MAP,
    DIGIT_4_MAP,
    DIGIT_5_MAP,
    DIGIT_6_MAP,
    DIGIT_7_MAP,
    DIGIT_8_MAP,
    DIGIT_9_MAP
};

void blankBuffer(char buffer[ROWS][COLS], char blank) {
    for (uint8_t i = 0; i < ROWS; i++) {
        for (uint8_t j = 0; j < COLS; j++) {
            buffer[i][j] = blank;
        }
    }
}

void blankBuffers(char buffers[2][ROWS][COLS], char blank) {
    blankBuffer(buffers[0], blank);
    blankBuffer(buffers[1], blank);
}

void blankBuffers(char buffers[2][ROWS][COLS]) {
    blankBuffers(buffers, CHAR_BLANK);
}

LCDDisplay::LCDDisplay(uint8_t addr_1, uint8_t addr_2) :
    player_1(addr_1, COLS, ROWS),
    player_2(addr_2, COLS, ROWS) {
    blankBuffers(last_displayed);
    last_cursor_indices[0] = -1;
    last_cursor_indices[1] = -1;
    backlight = false;
}

void LCDDisplay::begin(void) {
    player_1.begin();
    player_1.noCursor();
    player_1.clear();
    player_2.begin();
    player_2.noCursor();
    player_2.clear();
    // ideally, we wouldn't have to program the special characters each time, but oh well
    player_1.createChar(0, special_00);
    player_2.createChar(0, special_00);
    player_1.createChar(1, special_01);
    player_2.createChar(1, special_01);
    player_1.createChar(2, special_02);
    player_2.createChar(2, special_02);
    player_1.createChar(3, special_03);
    player_2.createChar(3, special_03);
    player_1.createChar(4, special_04);
    player_2.createChar(4, special_04);
    player_1.createChar(5, special_05);
    player_2.createChar(5, special_05);
    player_1.createChar(6, special_06);
    player_2.createChar(6, special_06);
    player_1.createChar(7, special_07);
    player_2.createChar(7, special_07);
    delay(500);
    player_1.clear();
    player_1.setCursor(0, 0);
    player_2.clear();
    player_2.setCursor(0, 0);
    if (backlight) {
        player_1.backlight();
        player_2.backlight();
    } else {
        player_1.noBacklight();
        player_2.noBacklight();
    }
}

void pushDigit(char buffer[ROWS][COLS], uint8_t digit, uint8_t x_offset, uint8_t y_offset) {
    if (digit >= 0 && digit <= 9) {
#if FONT == FONT_WIDE
        buffer[y_offset + 0][x_offset + 0] = font_digits[digit][0];
        buffer[y_offset + 0][x_offset + 1] = font_digits[digit][1];
        buffer[y_offset + 0][x_offset + 2] = font_digits[digit][2];
        buffer[y_offset + 1][x_offset + 0] = font_digits[digit][3];
        buffer[y_offset + 1][x_offset + 1] = font_digits[digit][4];
        buffer[y_offset + 1][x_offset + 2] = font_digits[digit][5];
#else
        buffer[y_offset][x_offset] = font_digits[digit][0];
        buffer[y_offset][x_offset + 1] = font_digits[digit][1];
        buffer[y_offset + 1][x_offset] = font_digits[digit][2];
        buffer[y_offset + 1][x_offset + 1] = font_digits[digit][3];
#endif
    }
}

int makeSettingsDisplayBuffer(char buffer[2][16], GameState *state, uint8_t player_number, game_settings_t *gs) {
    // Display settings like "00h30m00s   +00s"
    //                       "            -f-t"
    static const char default_buffer[2][16] = {
        {'0','0','h','0','0','m','0','0','s',' ',' ',' ','+','0','0','s'},
        {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','-','f','-','t'}
    };
    memcpy(buffer, default_buffer, 2 * 16 * sizeof(default_buffer[0][0]));
    uint8_t hours=(gs->player_settings[player_number].totalMillis / 1000 / 3600) % 100,
        minutes=(gs->player_settings[player_number].totalMillis / 1000 / 60) % 60,
        seconds=(gs->player_settings[player_number].totalMillis / 1000) % 60,
        incr_seconds=gs->player_settings[player_number].perTurnIncrMillis / 1000;
    byte beeps = (gs->flagBeep ? 0b010 : 0b000)
               | (gs->turnBeep ? 0b001 : 0b000);
    if (hours / 10) {
        buffer[0][0] += (char) (hours / 10);
    }
    if (hours % 10) {
        buffer[0][1] += (char) (hours % 10);
    }
    if (minutes / 10) {
        buffer[0][3] += (char) (minutes / 10);
    }
    if (minutes % 10) {
        buffer[0][4] += (char) (minutes % 10);
    }
    if (seconds / 10) {
        buffer[0][6] += (char) (seconds / 10);
    }
    if (seconds % 10) {
        buffer[0][7] += (char) (seconds % 10);
    }
    if (incr_seconds / 10) {
        buffer[0][13] += (char) (incr_seconds / 10);
    }
    if (incr_seconds % 10) {
        buffer[0][14] += (char) (incr_seconds % 10);
    }
    if (beeps & 0b010) {
        buffer[1][12] = '+';
    }
    if (beeps & 0b001) {
        buffer[1][14] = '+';
    }
    if (state->clock_mode != CM_EDIT_SETTINGS) {
        return -1;
    }
    if (state->option_index == OI_TURN_TEN_SECONDS) {
        return 13;
    } else if (state->option_index == OI_TURN_SECONDS) {
        return 14;
    } else if (state->option_index == OI_FLAG_BEEP) {
        return 28;
    } else if (state->option_index == OI_TURN_BEEP) {
        return 30;
    } else if ((state->option_index == OI_P1_HOURS && player_number == 0)
            || (state->option_index == OI_P2_HOURS && player_number == 1)) {
        return 1;
    } else if ((state->option_index == OI_P1_TEN_MINUTES && player_number == 0)
            || (state->option_index == OI_P2_TEN_MINUTES && player_number == 1)) {
        return 3;
    } else if ((state->option_index == OI_P1_MINUTES && player_number == 0)
            || (state->option_index == OI_P2_MINUTES && player_number == 1)) {
        return 4;
    } else if ((state->option_index == OI_P1_TEN_SECONDS && player_number == 0)
            || (state->option_index == OI_P2_TEN_SECONDS && player_number == 1)) {
        return 6;
    } else if ((state->option_index == OI_P1_SECONDS && player_number == 0)
            || (state->option_index == OI_P2_SECONDS && player_number == 1)) {
        return 7;
    } else {
        return -1;
    }
}

void makeOutOfTimeBuffer(char buffer[2][16]) {
    blankBuffer(buffer, CHAR_BLANK);
    buffer[0][1] = 0x03;
    buffer[0][2] = 0x02;
    buffer[0][3] = 'u';
    buffer[0][4] = 't';
    buffer[1][6] = 0x03;
    buffer[1][7] = 0x02;
    buffer[1][8] = 'f';
    buffer[0][10] = 0x00;
    buffer[0][11] = 0x05;
    buffer[0][12] = 'i';
    buffer[0][13] = 'm';
    buffer[0][14] = 'e';
}

void makeTimeDisplayBuffer(char buffer[2][16], unsigned long time) {
#if FONT == FONT_WIDE
    unsigned long rt = time;
    uint8_t digit;
    buffer[0][7] = CHAR_COLON_TOP;
    buffer[1][7] = CHAR_COLON_BOTTOM;
    buffer[0][0] = buffer[1][0] = ' ';
    buffer[0][14] = buffer[1][14] = buffer[0][15] = buffer[1][15] = ' ';
    // if we have more than 99 minutes of time, do HH:MM
    if (time > SECOND_MILLIS * 60 * 99) {
        // unit minutes
        rt /= SECOND_MILLIS * 60;
        digit = rt % 10;
        pushDigit(buffer, digit, 11, 0);
        // tens of minutes
        digit = (rt % 60) / 10;
        pushDigit(buffer, digit, 8, 0);

        // unit hours
        rt /= 60;
        digit = rt % 10;
        pushDigit(buffer, digit, 4, 0);
        // tens of hours
        digit = (rt % 100) / 10;
        pushDigit(buffer, digit, 1, 0);
    // if we have less than 99 minutes of time, do MM:SS
    } else {
        // unit seconds
        rt /= SECOND_MILLIS;
        digit = rt % 10;
        pushDigit(buffer, digit, 11, 0);
        // tens of seconds
        digit = (rt % 60) / 10;
        pushDigit(buffer, digit, 8, 0);

        // unit minutes
        rt /= 60;
        digit = rt % 10;
        pushDigit(buffer, digit, 4, 0);
        // tens of minutes
        digit = (rt % 100) / 10;
        pushDigit(buffer, digit, 1, 0);
    }
#else
    unsigned long rt = time;
    uint8_t digit;

    buffer[0][4] = CHAR_COLON_TOP;
    buffer[1][4] = CHAR_COLON_BOTTOM;
    buffer[0][9] = CHAR_COLON_TOP;
    buffer[1][9] = CHAR_COLON_BOTTOM;

    // unit seconds
    rt /= SECOND_MILLIS;
    digit = rt % 10;
    pushDigit(buffer, digit, 12, 0);
    // tens of seconds
    digit = (rt % 60) / 10;
    pushDigit(buffer, digit, 10, 0);
    
    // unit minutes
    rt /= 60;
    digit = rt % 10;
    pushDigit(buffer, digit, 7, 0);
    // tens of minutes
    digit = (rt % 60) / 10;
    pushDigit(buffer, digit, 5, 0);

    // unit hours
    rt /= 60;
    digit = rt % 10;
    pushDigit(buffer, digit, 2, 0);
#endif
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
    int cursor_indices[2] = {-1, -1};
    blankBuffers(new_buffers, CHAR_BLANK);

    if (game_state->clock_mode == CM_SELECT_SETTINGS ||
            game_state->clock_mode == CM_EDIT_SETTINGS) {
        last_cursor_indices[0] = cursor_indices[0];
        last_cursor_indices[1] = cursor_indices[1];
        cursor_indices[0] = makeSettingsDisplayBuffer(
            new_buffers[0], game_state, 0, game_state->settings
        );
        cursor_indices[1] = makeSettingsDisplayBuffer(
            new_buffers[1], game_state, 1, game_state->settings
        );
    } else {
        if (game_state->player_states[0].remainingMillis > 0) {
            makeTimeDisplayBuffer(new_buffers[0], game_state->player_states[0].remainingMillis);
        } else {
            makeOutOfTimeBuffer(new_buffers[0]);
        }
        if (game_state->player_states[1].remainingMillis > 0) {
            makeTimeDisplayBuffer(new_buffers[1], game_state->player_states[1].remainingMillis);
        } else {
            makeOutOfTimeBuffer(new_buffers[1]);
        }
    }

    if (displayBuffersDiffer(new_buffers[0], last_displayed[0])) {
        applyDisplayBuffer(&player_1, last_displayed[0], new_buffers[0]);
        memcpy(last_displayed[0], new_buffers[0], ROWS * COLS * sizeof(new_buffers[0][0][0]));
    }
    if (cursor_indices[0] == -1) {
        player_1.noCursor();
    } else {
        player_1.cursor();
        player_1.setCursor(cursor_indices[0] % 16, (uint8_t) (cursor_indices[0] / 16));
    }
    if (displayBuffersDiffer(new_buffers[1], last_displayed[1])) {
        applyDisplayBuffer(&player_2, last_displayed[1], new_buffers[1]);
        memcpy(last_displayed[1], new_buffers[1], ROWS * COLS * sizeof(new_buffers[0][0][0]));
    }
    if (cursor_indices[1] == -1) {
        player_2.noCursor();
    } else {
        player_2.cursor();
        player_2.setCursor(cursor_indices[1] % 16, (uint8_t) (cursor_indices[1] / 16));
    }
}

/* LCDDisplay interprets the "specialToggle" event to mean that it should toggle both backlights. */
void LCDDisplay::specialToggle(void) {
    backlight = !backlight;
    if (backlight) {
        player_1.backlight();
        player_2.backlight();
    } else {
        player_1.noBacklight();
        player_2.noBacklight();
    }
}

/* Display text directly on the screens */
void LCDDisplay::print(const char *str) {
    char new_buffers[2][ROWS][COLS];
    blankBuffers(new_buffers);
    uint8_t i;
    for (i = 0; i < ROWS * COLS * 2; i++) {
        if (str[i] == 0x00) {
            break;
        }
        new_buffers[1 - (i / (ROWS * COLS))][(i / COLS) % ROWS][i % COLS] = str[i];
    }
    applyDisplayBuffer(&player_1, last_displayed[0], new_buffers[0]);
    applyDisplayBuffer(&player_2, last_displayed[1], new_buffers[1]);
    memcpy(last_displayed[0], new_buffers[0], ROWS * COLS * sizeof(new_buffers[0][0][0]));
    memcpy(last_displayed[1], new_buffers[1], ROWS * COLS * sizeof(new_buffers[0][0][0]));
}
