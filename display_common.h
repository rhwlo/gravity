#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include "game_state.h"

class ChessDisplay {
    public:
        virtual void renderGameState(GameState *game_state);
        virtual void begin(void);
};

#endif // DISPLAY_COMMON_H