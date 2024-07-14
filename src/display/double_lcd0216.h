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


#define PCF8574_ADDR_0      0x27        // no jumpers are soldered
#define PCF8574_ADDR_1      0x26        // A0 jumper is soldered


// Typefaces are courtesy of https://github.com/upiir/character_display_big_digits
#if FONT == FONT_REAL_BOLD
unsigned char special_00[8] PROGMEM = {B11111,B00000,B00000,B00000,B00000,B00000,B00000,B00000};
unsigned char special_01[8] PROGMEM = {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B11111};
unsigned char special_02[8] PROGMEM = {B11100,B01110,B00110,B00111,B00111,B00110,B01110,B11100};
unsigned char special_03[8] PROGMEM = {B00111,B01110,B01100,B11100,B11100,B01100,B01110,B00111};
unsigned char special_04[8] PROGMEM = {B11110,B00110,B00110,B00110,B01100,B01100,B11000,B11000};
unsigned char special_05[8] PROGMEM = {B11000,B11000,B11000,B11000,B11000,B11000,B11000,B11000};
unsigned char special_06[8] PROGMEM = {B11000,B11000,B11000,B11000,B11100,B01100,B01110,B00111};
unsigned char special_07[8] PROGMEM = {B00011,B00110,B00110,B01100,B01100,B11000,B11000,B11111};
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
unsigned char special_00[8] PROGMEM = {B11111,B11000,B00000,B00000,B00000,B00000,B00000,B00000};
unsigned char special_01[8] PROGMEM = {B00000,B00000,B00000,B00000,B00000,B00000,B11000,B11111};
unsigned char special_02[8] PROGMEM = {B11111,B00011,B00011,B00011,B00011,B00011,B00011,B11111};
unsigned char special_03[8] PROGMEM = {B11111,B11000,B11000,B11000,B11000,B11000,B11000,B11111};
unsigned char special_04[8] PROGMEM = {B00001,B00010,B00100,B00000,B00000,B00000,B00000,B00000};
unsigned char special_05[8] PROGMEM = {B11000,B11000,B11000,B11000,B11000,B11000,B11000,B11000};
unsigned char special_06[8] PROGMEM = {B11111,B00000,B00000,B00000,B00000,B00000,B00000,B11111};
unsigned char special_07[8] PROGMEM = {B11000,B11000,B11000,B11000,B11000,B11000,B11000,B11111};
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
unsigned char special_00[8] PROGMEM = {B11011,B00000,B00000,B00000,B00000,B00000,B00000,B00000};
unsigned char special_01[8] PROGMEM = {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B11011};
unsigned char special_02[8] PROGMEM = {B11011,B00001,B00000,B00001,B00001,B00000,B00001,B11011};
unsigned char special_03[8] PROGMEM = {B11011,B10000,B00000,B10000,B10000,B00000,B10000,B11011};
unsigned char special_04[8] PROGMEM = {B11011,B10000,B00000,B10000,B10000,B00000,B10000,B10000};
unsigned char special_05[8] PROGMEM = {B10000,B10000,B00000,B10000,B10000,B00000,B10000,B10000};
unsigned char special_06[8] PROGMEM = {B11011,B00000,B00000,B00000,B00000,B00000,B00000,B11011};
unsigned char special_07[8] PROGMEM = {B10000,B10000,B00000,B10000,B10000,B00000,B10000,B11011};
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
        void renderGameState(GameState *game_state);
    private:
        LCD_I2C player_1, player_2;
        char last_displayed[2][ROWS][COLS];
};

#endif // DISPLAY_LCD_H