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


uint8_t selected_game_settings = 0;

void load_default_settings(void) {
    // 30 seconds                   +f+t
    all_game_settings[0].flagBeep = true;
    all_game_settings[0].turnBeep = true;
    all_game_settings[0].player_settings[0].totalMillis = 30 * SECOND_MILLIS;
    all_game_settings[0].player_settings[0].perTurnIncrMillis = 0;
    all_game_settings[0].player_settings[1].totalMillis = 30 * SECOND_MILLIS;
    all_game_settings[0].player_settings[1].perTurnIncrMillis = 0;

    // 5 minutes                    -f+t
    all_game_settings[1].flagBeep = false;
    all_game_settings[1].turnBeep = true;
    all_game_settings[1].player_settings[0].totalMillis = 5 * MINUTE_MILLIS;
    all_game_settings[1].player_settings[0].perTurnIncrMillis = 0;
    all_game_settings[1].player_settings[1].totalMillis = 5 * MINUTE_MILLIS;
    all_game_settings[1].player_settings[1].perTurnIncrMillis = 0;

    // 5 minutes + 3 seconds        -f+t
    all_game_settings[2].flagBeep = false;
    all_game_settings[2].turnBeep = true;
    all_game_settings[2].player_settings[0].totalMillis = 5 * MINUTE_MILLIS;
    all_game_settings[2].player_settings[0].perTurnIncrMillis = 3 * SECOND_MILLIS;
    all_game_settings[2].player_settings[1].totalMillis = 5 * MINUTE_MILLIS;
    all_game_settings[2].player_settings[1].perTurnIncrMillis = 3 * SECOND_MILLIS;

    // 15 minutes                   -f-t
    all_game_settings[3].flagBeep = false;
    all_game_settings[3].turnBeep = false;
    all_game_settings[3].player_settings[0].totalMillis = 15 * MINUTE_MILLIS;
    all_game_settings[3].player_settings[0].perTurnIncrMillis = 0;
    all_game_settings[3].player_settings[1].totalMillis = 15 * MINUTE_MILLIS;
    all_game_settings[3].player_settings[1].perTurnIncrMillis = 0;
}

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


int write_header_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset) {
    byte status = 0;
    status = _eeprom_update(eeprom, *eeprom_offset, VALIDATION_BYTE_0);
    (*eeprom_offset)++;
    if (status != 0) {
        return -status;
    }
    status = _eeprom_update(eeprom, *eeprom_offset, VALIDATION_BYTE_1);
    (*eeprom_offset)++;
    if (status != 0) {
        return -status;
    }
    status = _eeprom_update(eeprom, *eeprom_offset, GAME_SETTINGS_LEN);
    (*eeprom_offset)++;
    if (status != 0) {
        return -status;
    }
    return 0;
}

int write_header_to_bytes(byte bytes[], unsigned long *offset) {
    bytes[*offset] = (byte) VALIDATION_BYTE_0;
    (*offset)++;
    bytes[*offset] = (byte) VALIDATION_BYTE_1;
    (*offset)++;
    bytes[*offset] = (byte) GAME_SETTINGS_LEN;
    (*offset)++;
    return 0;
}

/* Verify that the header matches what we expect:
 *  - the two validation bytes (0xe4 0xc5)
 *  - followed by the game settings length
 */
int read_header_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position) {
    int result = 0;
    result = eeprom->read(*eeprom_position);
    (*eeprom_position)++;

    if (result < 0) {
        return result;
    } else if (((byte) result) != VALIDATION_BYTE_0) {
        return GS_ERR_VALUE_MISMATCH;
    }

    result = eeprom->read(*eeprom_position);
    (*eeprom_position)++;
    if (result < 0) {
        return result;
    } else if (((byte) result) != VALIDATION_BYTE_1) {
        return GS_ERR_VALUE_MISMATCH;
    }

    result = eeprom->read(*eeprom_position);
    (*eeprom_position)++;
    if (result < 0) {
        return result;
    } else if (((byte) result) != GAME_SETTINGS_LEN) {
        return GS_ERR_VALUE_MISMATCH;
    }
    return 3;
}

int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, player_settings_t *value) {
    int status = 0;
    status = eeprom->read(*eeprom_position, (byte *) &(value->perTurnIncrMillis), sizeof(value->perTurnIncrMillis));
    if (status != 0) {
        return -status;
    }
    (*eeprom_position) += sizeof(value->perTurnIncrMillis);
    status = eeprom->read(*eeprom_position, (byte *) &(value->totalMillis), sizeof(value->totalMillis));
    if (status != 0) {
        return -status;
    }
    (*eeprom_position) += sizeof(value->totalMillis);
    return status;
}

int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, game_settings_t *value) {
    int status = 0;

    byte flags = 0;
    status = eeprom->read(*eeprom_position, &flags, 1);
    (*eeprom_position)++;
    if (status < 0) {
        return status;
    }
    value->flagBeep = (flags & 0b01) ? true : false;
    value->turnBeep = (flags & 0b10) ? true : false;

    status = read_from_eeprom(eeprom, eeprom_position, &(value->player_settings[0]));
    if (status < 0) {
        return status;
    }
    (*eeprom_position) += status;
    status = read_from_eeprom(eeprom, eeprom_position, &(value->player_settings[1]));
    if (status < 0) {
        return status;
    }
    (*eeprom_position) += status;
    return status;
}

int write_to_bytes(byte bytes[], unsigned long *offset, player_settings_t *value) {
    memcpy(&(bytes[*offset]), (byte *) &(value->perTurnIncrMillis), sizeof(value->perTurnIncrMillis));
    (*offset) += sizeof(value->perTurnIncrMillis);
    memcpy(&(bytes[*offset]), (byte *) &(value->totalMillis), sizeof(value->totalMillis));
    (*offset) += sizeof(value->totalMillis);
    return 0;
}

int write_to_bytes(byte bytes[], unsigned long *offset, game_settings_t *value) {
    byte flags = 0b00000000;
    if (value->flagBeep) {
        flags |= 0b1;
    }
    if (value->turnBeep) {
        flags |= 0b10;
    }
    memcpy(&(bytes[*offset]), &flags, sizeof(flags));
    (*offset) += sizeof(flags);

    write_to_bytes(bytes, offset, &(value->player_settings[0]));
    write_to_bytes(bytes, offset, &(value->player_settings[1]));
    return 0;
}

int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, player_settings_t *value) {
    int status = 0;

    status = _eeprom_update(eeprom, *eeprom_position, (byte *) &(value->perTurnIncrMillis), sizeof(value->perTurnIncrMillis));
    (*eeprom_position) += sizeof(value->perTurnIncrMillis);
    if (status != 0) {
        return -status;
    }

    status = _eeprom_update(eeprom, *eeprom_position, (byte *) &(value->totalMillis), sizeof(value->totalMillis));
    (*eeprom_position) += sizeof(value->totalMillis);
    if (status != 0) {
        return -status;
    }
    return 0;
}

int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, game_settings_t *value) {
    int status = 0;
    byte flags = 0;

    if (value->flagBeep) {
        flags |= 0b1;
    }
    if (value->turnBeep) {
        flags |= 0b10;
    }

    status = eeprom->write(*eeprom_position, flags);
    (*eeprom_position) += sizeof(flags);
    if (status < 0) {
        return status;
    }

    status = write_to_eeprom(eeprom, eeprom_position, &(value->player_settings[0]));
    if (status < 0) {
        return status;
    }
    status = write_to_eeprom(eeprom, eeprom_position, &(value->player_settings[1]));
    if (status < 0) {
        return status;
    }
    return 0;
}


/* Write the settings state to EEPROM. This includes:
   1. two bytes of validation (0xE4C5)
   2. the number of settings (GAME_SETTINGS_LEN)
   3. the contents of all stored game settings  */
int write_settings_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset) {
    int status = 0;
    int i = 0;
    
    // Write validation
    status = write_header_to_eeprom(eeprom, eeprom_offset);
    if (status < 0) {
        return status;
    }

    // Write the contents of all the settings
    for (i = 0; i < GAME_SETTINGS_LEN; i++) {
        status = write_to_eeprom(eeprom, eeprom_offset, &(all_game_settings[i]));
        if (status < 0) {
            return status;
        }
    }
    return 0;
}

int write_settings_to_eeprom(extEEPROM *eeprom) {
    unsigned long eeprom_offset = 0;
    return write_header_to_eeprom(eeprom, &eeprom_offset);
}

int write_settings_to_bytes(byte bytes[], unsigned long *offset) {
    int status = 0;

    status = write_header_to_bytes(bytes, offset);
    if (status < 0) {
        return status;
    }

    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        status = write_to_bytes(bytes, offset, &(all_game_settings[i]));
        if (status < 0) {
            return status;
        }
    }
    return 0;
}

int write_settings_to_bytes(byte bytes[]) {
    unsigned long offset = 0;

    return write_settings_to_bytes(bytes, &offset);
}

int read_settings_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position) {
    int status = 0;

    status = read_header_from_eeprom(eeprom, eeprom_position);
    if (status < 0) {
        return status;
    }

    for (int i = 0; i < GAME_SETTINGS_LEN; i++) {
        status = read_from_eeprom(eeprom, eeprom_position, &all_game_settings[i]);
        if (status < 0) {
            return status;
        }
    }
    return status;
}

int read_settings_from_eeprom(extEEPROM *eeprom) {
    unsigned long eeprom_position = 0;
    return read_settings_from_eeprom(eeprom, &eeprom_position);
}

const char * stringifyError(int error_status) {
    switch (error_status) {
        case GS_ERR_TOO_LONG:
            return "Message too long";
        case GS_ERR_VALUE_MISMATCH:
            return "Expected value mismatch";
        case GS_ERR_ADDR_SEND_NACK:
            return "Received NACK for ADDR";
        case GS_ERR_DATA_SEND_NACK:
            return "Received NACK for data";
        case GS_ERR_TIMEOUT:
            return "Timed out";
        case GS_ERR_OTHER_TWI_ERR:
            return "Other two-wire interface error";
    }
    return "UNKNOWN ERROR";
}

