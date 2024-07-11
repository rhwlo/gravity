#ifndef DISPLAY_LCD_H
#define DISPLAY_LCD_H

#include <LCD_I2C.h>
#include "game_state.h"
#include "display_common.h"

#define CHAR_DOT            '.'
#define CHAR_COLON_TOP      ' '
#define CHAR_COLON_BOTTOM   ':'
#define CHAR_BLANK          ' '
#define COLS                16
#define ROWS                2

class LCDDisplay : public ChessDisplay {
    public:
        LCDDisplay(uint8_t addr_1, uint8_t addr_2);
        void begin(void);
        void renderGameState(GameState *game_state);
    private:
        void prettyPrintTime(LCD_I2C *lcd, unsigned long time);
        void printChar(LCD_I2C *lcd, char c, uint8_t col, uint8_t row);
        LCD_I2C player_1, player_2;
        char last_displayed[2][ROWS][COLS];
};

#endif // DISPLAY_LCD_H