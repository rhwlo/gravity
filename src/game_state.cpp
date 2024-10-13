#include "game_state.h"


GameState::GameState(game_settings_t *game_settings) {
    clock_mode = CM_SELECT_SETTINGS;
    whoseTurn = 0;
    settings = game_settings;
    option_index = -1;
    curr_player_state = &(player_states[whoseTurn]);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        player_states[i] = {
            false,
            game_settings->player_settings[i].totalMillis
        };
    }
}

void GameState::reset(void) {
    whoseTurn = 0;
    curr_player_state = &(player_states[whoseTurn]);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        player_states[i] = {
            false,
            settings->player_settings[i].totalMillis
        };
    }
}

bool GameState::setTurn(unsigned short newTurn) {
    if (newTurn >= NUM_PLAYERS) {
        return false;       // this would be an error if we did those
    }
    // unpause, regardless of whether we're changing turns
    clock_mode = CM_ACTIVE;
    if (newTurn == whoseTurn) {
        return true;        // setting the turn to the current player just unpauses
    }
    whoseTurn = newTurn;
    curr_player_state = &(player_states[whoseTurn]);
    if (!curr_player_state->outOfTime) {
        curr_player_state->remainingMillis += settings->player_settings[whoseTurn].perTurnIncrMillis;
    }
    return true;
}


uint8_t selected_game_settings = 1;

game_settings_t all_game_settings[GAME_SETTINGS_LEN] = {

    {   // 30 seconds               +f+t
        {
            {
                30 * SECOND_MILLIS, // totalMillis
                0                   // perTurnIncrMillis,
            },
            {
                30 * SECOND_MILLIS, // totalMillis
                0                   // perTurnIncrMillis,
            },
        },
        true,                       // flag beep
        true                        // turn beep
    },

    {   // 5 minutes                +f+t
        {
            {
                5 * MINUTE_MILLIS,  // totalMillis
                0                   // perTurnIncrMillis,
            },
            {
                5 * MINUTE_MILLIS,  // totalMillis
                0                   // perTurnIncrMillis,
            },
        },
        false,                      // flag beep
        true                        // turn beep
    },

    {   // 5 minutes + 3 seconds    -f+t
        {
            {
                5 * MINUTE_MILLIS,  // totalMillis
                3 * SECOND_MILLIS   // perTurnIncrMillis,
            },
            {
                5 * MINUTE_MILLIS,  // totalMillis
                3 * SECOND_MILLIS   // perTurnIncrMillis,
            },
        },
        false,                      // flag beep
        true                        // turn beep
    },

    {   // 15 minutes               -f-t
        {
            {
                15 * MINUTE_MILLIS, // totalMillis
                0                   // perTurnIncrMillis,
            },
            {
                15 * MINUTE_MILLIS, // totalMillis
                0                   // perTurnIncrMillis,
            },
        },
        false,                      // flag beep
        false                       // turn beep
    }
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


#ifdef USE_EEPROM
/* Write the settings state to EEPROM. This includes:
   1. the two-byte validation code (0xE4C5)
   2. one byte of the uint8_t selected_game_settings;
   3. the contents of all stored game settings  */
void write_state_to_eeprom(EEPROMClass *eeprom) {
    int eeprom_offset = 0;
    
    // Write validation
    eeprom_offset += write_validation_to_eeprom(eeprom, eeprom_offset);

    // Write the index of the currently-selected game settings
    eeprom->write(eeprom_offset, selected_game_settings);
    eeprom_offset++;
    
    // Write the contents of all the settings
    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        eeprom_offset += write_setting_to_eeprom(&all_game_settings[i], eeprom, eeprom_offset);
    }
}

bool read_state_from_eeprom(EEPROMClass *eeprom) {
    int eeprom_offset = 0;

    if (!read_validation_from_eeprom(eeprom, eeprom_offset)) {
        return false;
    }
    eeprom_offset += 2;

    selected_game_settings = eeprom->read(eeprom_offset);
    eeprom_offset++;

    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        eeprom_offset += read_setting_from_eeprom(
            &all_game_settings[i],
            eeprom,
            eeprom_offset
        );
    }
    return true;
}
#else
void write_state_to_eeprom(EEPROMClass *eeprom) {
    return;
}

bool read_state_from_eeprom(EEPROMClass *eeprom) {
    return true;
}
#endif
