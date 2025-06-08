#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include "game_state.h"

#define DISPLAY_SERIAL      1
#define DISPLAY_DOUBLE_LCD  2
#define DISPLAY_SINGLE_OLED 3

class ChessDisplay {
    public:
        // render the given game state
        virtual void renderGameState(GameState *game_state);
        // "special toggle", a customizable signal
        virtual void specialToggle(void);
        // initialize the display
        virtual void begin(void);
};

#endif // DISPLAY_COMMON_H
