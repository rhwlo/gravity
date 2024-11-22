#ifndef DISPLAY_LCD_H
#define DISPLAY_LCD_H

#include <LCD_I2C.h>
#include "../game_state.h"
#include "../display.h"

#define CHAR_DOT            '.'
#define CHAR_COLON_TOP      '.'
#define CHAR_COLON_BOTTOM   B10100101
#define CHAR_BLANK          ' '
#define COLS                16
#define ROWS                2

#define FONT_REAL_BOLD      1
#define FONT_CLASSIC        2 
#define FONT_DASHED_BOLD    3
#define FONT                FONT_REAL_BOLD


#define PCF8574_ADDR_0      0x27        // A0 jumper is soldered
#define PCF8574_ADDR_1      0x26        // no jumpers are soldered


// Typefaces are courtesy of https://github.com/upiir/character_display_big_digits
#if FONT == FONT_REAL_BOLD
#define DIGIT_0_MAP         { CHAR_BLANK, CHAR_BLANK, 0x03,       0x02 }
#define DIGIT_1_MAP         { CHAR_BLANK, 0x05,       CHAR_BLANK, 0x05 }
#define DIGIT_2_MAP         { 0x00,       0x02,       0x03,       0x01 }
#define DIGIT_3_MAP         { 0x00,       0x02,       0x01,       0x02 }
#define DIGIT_4_MAP         { 0x07,       0x01,       CHAR_BLANK, 0x05 }
#define DIGIT_5_MAP         { 0x07,       0x00,       0x01,       0x02 }
#define DIGIT_6_MAP         { 0x07,       0x00,       0x06,       0x02 }
#define DIGIT_7_MAP         { 0x00,       0x04,       0x20,       0x05 }
#define DIGIT_8_MAP         { 0x03,       0x02,       0x03,       0x02 }
#define DIGIT_9_MAP         { 0x03,       0x02,       0x01,       0x04 }
#elif FONT == FONT_CLASSIC
#define DIGIT_0_MAP         { CHAR_BLANK, CHAR_BLANK, 0x03,       0x02 }
#define DIGIT_1_MAP         { 0x04,       0x05,       CHAR_BLANK, 0x05 }
#define DIGIT_2_MAP         { 0x06,       0x02,       0x03,       0x06 }
#define DIGIT_3_MAP         { 0x00,       0x02,       0x01,       0x02 }
#define DIGIT_4_MAP         { 0x07,       0x01,       CHAR_BLANK, 0x05 }
#define DIGIT_5_MAP         { 0x03,       0x06,       0x06,       0x02 }
#define DIGIT_6_MAP         { 0x03,       0x06,       0x03,       0x02 }
#define DIGIT_7_MAP         { 0x00,       0x02,       CHAR_BLANK, 0x05 }
#define DIGIT_8_MAP         { 0x03,       0x02,       0x03,       0x02 }
#define DIGIT_9_MAP         { 0x03,       0x02,       0x06,       0x02 }
#elif FONT == FONT_DASHED
#define DIGIT_0_MAP         { 0x04,       0x05,       0x07,       0x05 }
#define DIGIT_1_MAP         { CHAR_BLANK, 0x05,       CHAR_BLANK, 0x05 }
#define DIGIT_2_MAP         { 0x06,       0x02,       0x03,       0x06 }
#define DIGIT_3_MAP         { 0x00,       0x02,       0x01,       0x02 }
#define DIGIT_4_MAP         { 0x07,       0x01,       CHAR_BLANK, 0x05 }
#define DIGIT_5_MAP         { 0x03,       0x06,       0x06,       0x02 }
#define DIGIT_6_MAP         { 0x03,       0x06,       0x03,       0x02 }
#define DIGIT_7_MAP         { 0x00,       0x02,       CHAR_BLANK, 0x05 }
#define DIGIT_8_MAP         { 0x03,       0x02,       0x03,       0x02 }
#define DIGIT_9_MAP         { 0x03,       0x02,       0x06,       0x02 }
#endif

class LCDDisplay : public ChessDisplay {
    public:
        LCDDisplay(uint8_t addr_1, uint8_t addr_2);
        void begin(void);
        void specialToggle(void);
        void renderGameState(GameState *game_state);
    private:
        LCD_I2C player_1, player_2;
        bool backlight;
        char last_displayed[2][ROWS][COLS];
        int last_cursor_indices[2];
};

#endif // DISPLAY_LCD_H