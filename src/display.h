#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include "game_state.h"

#define DISPLAY_SERIAL      1
#define DISPLAY_DOUBLE_LCD  2
#define DISPLAY_SINGLE_OLED 3

class ChessDisplay {
    public:
        virtual void renderGameState(GameState *game_state);
        virtual void begin(void);
};

#endif // DISPLAY_COMMON_H