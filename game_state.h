#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "chess_time.h"

#define NUM_PLAYERS     2   // because it's chess
#define PLAYER_WHITE    0
#define PLAYER_BLACK    1

struct player_settings_t
{
    unsigned long  totalMillis;
    unsigned short perTurnIncrMillis;
    unsigned short gracePeriodMillis;
};

struct game_settings_t
{
    player_settings_t player_settings[NUM_PLAYERS];
    bool warningBeep;   // warn when we cross a threshold
    bool flagBeep;      // beep when a player runs out of time
};

struct player_state_t {
    bool outOfTime;
    unsigned long remainingMillis;
    unsigned short gracePeriodMillis;
};

class GameState {
    public:
        GameState(game_settings_t *game_settings);
        // ~GameState(void);
        bool setTurn(unsigned short newTurn);
        bool paused;
        void pause(void);
        unsigned short whoseTurn;
        player_state_t player_states[NUM_PLAYERS];
    private:
        game_settings_t *settings;
};

struct game_state_t
{
    bool paused;
    bool blacksTurn;
    player_state_t white;
    player_state_t black;
    game_settings_t *settings;
};

extern game_settings_t setting_blitz_5_0, standard_settings;


#endif  // GAME_STATE_H