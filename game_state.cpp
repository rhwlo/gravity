#include "game_state.h"


GameState::GameState(game_settings_t *game_settings) {
    paused = true;
    whoseTurn = 0;
    settings = game_settings;
    curr_player_state = &(player_states[whoseTurn]);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        player_states[i] = {
            false,
            game_settings->player_settings[i].totalMillis,
            game_settings->player_settings[i].gracePeriodMillis,
        };
    }
}

bool GameState::setTurn(unsigned short newTurn) {
    if (newTurn >= NUM_PLAYERS) {
        return false;       // this would be an error if we did those
    }
    // unpause, regardless of whether we're changing turns
    paused = false;
    if (newTurn == whoseTurn) {
        return true;        // setting the turn to the current player just unpauses
    }
    whoseTurn = newTurn;
    curr_player_state = &(player_states[whoseTurn]);
    paused = false;
    if (!player_states[whoseTurn].outOfTime) {
        player_states[whoseTurn].gracePeriodMillis = settings->player_settings[whoseTurn].gracePeriodMillis;
        player_states[whoseTurn].remainingMillis += settings->player_settings[whoseTurn].perTurnIncrMillis;
    }
    return true;
}

void GameState::pause(void) {
    paused = true;
}

game_settings_t setting_blitz_5m_0 = {
    {
        {
            5 * MINUTE_MILLIS,  // totalMillis
            0,                  // perTurnIncrMillis,
            0                   // gracePeriodMillis
        },
        {
            5 * MINUTE_MILLIS,  // totalMillis
            0,                  // perTurnIncrMillis,
            0                   // gracePeriodMillis
        },
    },
    false,                      // warning beep
    false                       // flag beep
};

game_settings_t setting_blitz_5m_3s = {
    {
        {
            5 * MINUTE_MILLIS,  // totalMillis
            3 * SECOND_MILLIS,  // perTurnIncrMillis,
            0                   // gracePeriodMillis
        },
        {
            5 * MINUTE_MILLIS,  // totalMillis
            3 * SECOND_MILLIS,  // perTurnIncrMillis,
            0                   // gracePeriodMillis
        },
    },
    false,                      // warning beep
    false                       // flag beep
};

game_settings_t setting_blitz_30s_0 = {
    {
        {
            30 * SECOND_MILLIS, // totalMillis
            0,                  // perTurnIncrMillis,
            0                   // gracePeriodMillis
        },
        {
            30 * SECOND_MILLIS, // totalMillis
            0,                  // perTurnIncrMillis,
            0                   // gracePeriodMillis
        },
    },
    false,                      // warning beep
    true                        // flag beep
};

game_settings_t standard_settings = {
    {
        {
            45 * MINUTE_MILLIS, // totalMillis
            0,                  // perTurnIncrMillis,
            5                   // gracePeriodMillis
        },
        {
            45 * MINUTE_MILLIS, // totalMillis
            0,                  // perTurnIncrMillis,
            5                   // gracePeriodMillis
        },
    },
    false,                      // warning beep
    false                       // flag beep
};