#ifndef DISPLAY_LCD8_H
#define DISPLAY_LCD8_H

#include <LCD_I2C.h>
#include "../game_state.h"
#include "../display.h"

#define CHAR_BLANK          ' '
#define COLS                8
#define ROWS                1

#define FLAG_BEEP_CHAR      'F'
#define NO_FLAG_BEEP_CHAR   'f'
#define TURN_BEEP_CHAR      'T'
#define NO_TURN_BEEP_CHAR   't'

#define D_DL18_MODE_HH_MM_SS    0       // HH:MM:SS
#define D_DL18_MODE_MM_SS       1       // MM:SS
#define D_DL18_MODE_MM_SSdSS    2       // MM:SS.ss

#define PCF8574_ADDR_0      0x27        // A0 jumper is soldered
#define PCF8574_ADDR_1      0x26        // no jumpers are soldered

class LCDDisplay8 : public ChessDisplay {
    public:
        LCDDisplay8(uint8_t addr_1, uint8_t addr_2);
        void begin(void);
        void specialToggle(void);
        void renderGameState(GameState *game_state);
        void print(const char *str);
        void print(const char *strA, const char *strB);
    private:
        LCD_I2C player_1, player_2;
        bool backlight;
        char last_displayed[2][COLS];
        int last_cursor_indices[2];
        void applyBuffers(char buffers[2][COLS]);
};

#endif // DISPLAY_LCD8_H
