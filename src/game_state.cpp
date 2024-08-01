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

void GameState::reset(void) {
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
    if (!curr_player_state->outOfTime) {
        curr_player_state->gracePeriodMillis = settings->player_settings[whoseTurn].gracePeriodMillis;
        curr_player_state->remainingMillis += settings->player_settings[whoseTurn].perTurnIncrMillis;
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
    false,                      // flag beep
    true                        // turn beep
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
    false,                      // flag beep
    true                        // turn beep
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
    true,                       // flag beep
    true                        // turn beep
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
    false,                      // flag beep
    false                       // turn beep
};

int write_validation_to_eeprom(EEPROMClass *eeprom, int eeprom_offset) {
    eeprom->write(eeprom_offset,        VALIDATION_BYTE_0);
    eeprom->write(eeprom_offset + 1,    VALIDATION_BYTE_1);
    return 2;
}

bool read_validation_from_eeprom(EEPROMClass *eeprom, int eeprom_offset) {
    uint8_t val1=eeprom->read(eeprom_offset), val2=eeprom->read(eeprom_offset + 1);
    return (val1 == VALIDATION_BYTE_0 && val2 == VALIDATION_BYTE_1);
}

int write_setting_to_eeprom(game_settings_t *game_settings, EEPROMClass *eeprom, int eeprom_offset) {
    int i = 0;
    uint8_t *ptr=0;
    for (
        i = 0, ptr = (uint8_t *) (game_settings + i);
        i < sizeof(game_settings) && (i + eeprom_offset) < eeprom->length();
        i++
    ) {
        eeprom->write(i + eeprom_offset, *ptr);
    }
    return i;
}

int read_setting_from_eeprom(game_settings_t *game_settings, EEPROMClass *eeprom, int eeprom_offset) {
    int i = 0;
    uint8_t *ptr=0;
    for (
        i = 0, ptr = (uint8_t *) (game_settings + i);
        i < sizeof(game_settings) && (i + eeprom_offset) < eeprom->length();
        i++
    ) {
        *ptr = eeprom->read(i + eeprom_offset);
    }
    return i;
}

game_settings_t game_settings[GAME_SETTINGS_LEN] = {
    setting_blitz_30s_0,
    setting_blitz_5m_0,
    setting_blitz_5m_3s,
    standard_settings,
};

uint8_t selected_game_settings = 1;

void write_settings_to_eeprom(EEPROMClass *eeprom) {
    int eeprom_offset = 0;
    eeprom_offset += write_validation_to_eeprom(eeprom, eeprom_offset);
    eeprom->write(eeprom_offset, selected_game_settings);
    eeprom_offset++;
    
    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        eeprom_offset += write_setting_to_eeprom(&game_settings[i], eeprom, eeprom_offset);
    }
}

bool read_all_settings_from_eeprom(EEPROMClass *eeprom) {
    int eeprom_offset = 0;

    if (!read_validation_from_eeprom(eeprom, eeprom_offset)) {
        return false;
    }
    eeprom_offset += 2;

    selected_game_settings = eeprom->read(eeprom_offset);
    eeprom_offset++;

    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        eeprom_offset += read_setting_from_eeprom(
            &game_settings[i],
            eeprom,
            eeprom_offset
        );
    }
    return true;
}