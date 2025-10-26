#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include "game_state.h"

#define DISPLAY_SERIAL          1
#define DISPLAY_DOUBLE_LCD_0216 2
#define DISPLAY_SINGLE_OLED     3
#define DISPLAY_DOUBLE_LCD_0108 4

class ChessDisplay {
    public:
        // render the given game state
        virtual void render_game_state(GameState *game_state);
        // "special toggle", a customizable signal
        virtual void special_toggle(void);
        // initialize the display
        virtual void begin(void);
        // write text directly to the display
        virtual void print(const char *str);
};

#endif // DISPLAY_COMMON_H
