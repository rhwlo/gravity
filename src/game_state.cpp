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

    {   // 5 minutes                -f+t
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

byte _eeprom_update(extEEPROM *eeprom, unsigned long addr, byte value) {
    return eeprom->update(addr, value);
}

byte _eeprom_update(extEEPROM *eeprom, unsigned long addr, byte *values, unsigned int nBytes) {
    unsigned int i;
    byte status = 0;
    for (i = 0; i < nBytes; i++) {
        status = eeprom->update(addr + i, *(values + i));
        if (status != 0) {
            return status;
        }
    }
    return 0;
}


int write_header_to_eeprom(extEEPROM *eeprom, int eeprom_offset) {
    _eeprom_update(eeprom, eeprom_offset,        VALIDATION_BYTE_0);
    _eeprom_update(eeprom, eeprom_offset + 1,    VALIDATION_BYTE_1);
    _eeprom_update(eeprom, eeprom_offset + 2,    GAME_SETTINGS_LEN);
    return 3;
}

int write_header_to_bytes(byte bytes[], int offset) {
    bytes[offset + 0] = (byte) VALIDATION_BYTE_0;
    bytes[offset + 1] = (byte) VALIDATION_BYTE_1;
    bytes[offset + 2] = (byte) GAME_SETTINGS_LEN;
    return 3;
}

int read_header_from_eeprom(extEEPROM *eeprom, int eeprom_offset) {
    if (eeprom->read(eeprom_offset + 0) != VALIDATION_BYTE_0) {
        return -1;
    }
    if (eeprom->read(eeprom_offset + 1) != VALIDATION_BYTE_1) {
        return -1;
    }
    if (eeprom->read(eeprom_offset + 2) != GAME_SETTINGS_LEN) {
        return -1;
    }
    return 3;
}

int read_from_eeprom(extEEPROM *eeprom, unsigned long addr, player_settings_t *value) {
    unsigned long i = 0;
    int status = 0;
    status = eeprom->read(addr + i, (byte *) &(value->perTurnIncrMillis), sizeof(value->perTurnIncrMillis));
    if (status != 0) {
        return -status;
    }
    i += sizeof(value->perTurnIncrMillis);
    status = eeprom->read(addr + i, (byte *) &(value->totalMillis), sizeof(value->totalMillis));
    if (status != 0) {
        return -status;
    }
    i += sizeof(value->totalMillis);
    return i;
}

int read_from_eeprom(extEEPROM *eeprom, unsigned long addr, game_settings_t *value) {
    unsigned long i = 0;
    int status = 0;
    byte flags = eeprom->read(addr + (i++));
    value->flagBeep = (flags & 0b01) ? true : false;
    value->turnBeep = (flags & 0b10) ? true : false;

    status = read_from_eeprom(eeprom, addr + i, &(value->player_settings[0]));
    if (status < 0) {
        return status;
    }
    i += status;
    status = read_from_eeprom(eeprom, addr + i, &(value->player_settings[1]));
    if (status < 0) {
        return status;
    }
    i += status;
    return i;
}

int write_to_bytes(byte bytes[], unsigned long offset, player_settings_t *value) {
    unsigned long i = 0;
    memcpy(&(bytes[offset + i]), (byte *) &(value->perTurnIncrMillis), sizeof(value->perTurnIncrMillis));
    i += sizeof(value->perTurnIncrMillis);
    memcpy(&(bytes[offset + i]), (byte *) &(value->totalMillis), sizeof(value->totalMillis));
    i += sizeof(value->totalMillis);
    return i;
}

int write_to_bytes(byte bytes[], unsigned long offset, game_settings_t *value) {
    unsigned long i = 0;
    byte flags = 0b00000000;
    if (value->flagBeep) {
        flags |= 0b1;
    }
    if (value->turnBeep) {
        flags |= 0b10;
    }
    memcpy(&(bytes[offset + i]), &flags, sizeof(flags));
    i += sizeof(flags);

    i += write_to_bytes(bytes, offset + i, &(value->player_settings[0]));
    i += write_to_bytes(bytes, offset + i, &(value->player_settings[1]));
    return i;
}

int write_to_eeprom(extEEPROM *eeprom, unsigned long addr, player_settings_t *value) {
    unsigned long i = 0;
    _eeprom_update(eeprom, addr + i, (byte *) &(value->perTurnIncrMillis), sizeof(value->perTurnIncrMillis));
    i += sizeof(value->perTurnIncrMillis);
    _eeprom_update(eeprom, addr + i, (byte *) &(value->totalMillis), sizeof(value->totalMillis));
    i += sizeof(value->totalMillis);
    return i;
}

int write_to_eeprom(extEEPROM *eeprom, unsigned long addr, game_settings_t *value) {
    unsigned long i = 0;
    int status;
    byte flags = 0;
    if (value->flagBeep) {
        flags |= 0b1;
    }
    if (value->turnBeep) {
        flags |= 0b10;
    }

    eeprom->write(addr + i, flags);
    i += sizeof(flags);
    status = write_to_eeprom(eeprom, addr + i, &(value->player_settings[0]));
    if (status <= 0) {
        return status;
    }
    i += status;
    status = write_to_eeprom(eeprom, addr + i, &(value->player_settings[1]));
    if (status <= 0) {
        return status;
    }
    i += status;
    return i;
}


/* Write the settings state to EEPROM. This includes:
   1. two bytes of validation (0xE4C5)
   2. the number of settings (GAME_SETTINGS_LEN)
   3. the contents of all stored game settings  */
int write_settings_to_eeprom(extEEPROM *eeprom) {
    int status = 0;
    int eeprom_offset = 0;
    int i = 0;
    
    // Write validation
    status = write_header_to_eeprom(eeprom, eeprom_offset);
    if (status < 0) {
        return status;
    }
    eeprom_offset += status;

    // Write the contents of all the settings
    for (i = 0; i < GAME_SETTINGS_LEN; i++) {
        status = write_to_eeprom(eeprom, eeprom_offset, &(all_game_settings[i]));
        if (status <= 0) {
            return status;
        }
        eeprom_offset += status;
    }
    return eeprom_offset;
}

int write_settings_to_bytes(byte bytes[]) {
    int status = 0;
    int offset = 0;

    status = write_header_to_bytes(bytes, offset);
    if (status < 0) {
        return status;
    }
    offset += status;

    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        status = write_to_bytes(bytes, offset, &(all_game_settings[i]));
        if (status < 0) {
            return status;
        }
        offset += status;
    }
    return offset;
}

int read_settings_from_eeprom(extEEPROM *eeprom) {
    int eeprom_offset = 0;
    int status = 0;

    status = read_header_from_eeprom(eeprom, eeprom_offset);
    if (status < 0) {
        return status;
    }
    eeprom_offset += status;

    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        status = read_from_eeprom(eeprom, eeprom_offset, &all_game_settings[i]);
        if (status < 0) {
            return status;
        } else {
            eeprom_offset += status;
        }
    }
    return eeprom_offset;
}