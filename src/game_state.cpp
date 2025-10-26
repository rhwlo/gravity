#include "game_state.h"


GameState::GameState(GameSettings *game_settings) {
    clock_mode = CM_SELECT_SETTINGS;
    whose_turn = 0;
    settings = game_settings;
    option_index = -1;
    curr_player_state = &(player_states[whose_turn]);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        player_states[i] = {
            false,
            game_settings->player_settings[i].total_millis
        };
    }
}

void GameState::reset(void) {
    whose_turn = 0;
    curr_player_state = &(player_states[whose_turn]);

    for (int i = 0; i < NUM_PLAYERS; i++) {
        player_states[i] = {
            false,
            settings->player_settings[i].total_millis
        };
    }
}

bool GameState::set_turn(unsigned short new_turn) {
    if (new_turn >= NUM_PLAYERS) {
        return false;       // this would be an error if we did those
    }
    // unpause, regardless of whether we're changing turns
    clock_mode = CM_ACTIVE;
    if (new_turn == whose_turn) {
        return true;        // setting the turn to the current player just unpauses
    }
    whose_turn = new_turn;
    curr_player_state = &(player_states[whose_turn]);
    if (!curr_player_state->out_of_time) {
        curr_player_state->remaining_millis += settings->player_settings[whose_turn].per_turn_incr_millis;
    }
    return true;
}


uint8_t selected_game_settings = 0;

void load_default_settings(void) {
    // 30 seconds                   +f+t
    all_game_settings[0].flag_beep = true;
    all_game_settings[0].turn_beep = true;
    all_game_settings[0].player_settings[0].total_millis = 30 * SECOND_MILLIS;
    all_game_settings[0].player_settings[0].per_turn_incr_millis = 0;
    all_game_settings[0].player_settings[1].total_millis = 30 * SECOND_MILLIS;
    all_game_settings[0].player_settings[1].per_turn_incr_millis = 0;

    // 5 minutes                    -f+t
    all_game_settings[1].flag_beep = false;
    all_game_settings[1].turn_beep = true;
    all_game_settings[1].player_settings[0].total_millis = 5 * MINUTE_MILLIS;
    all_game_settings[1].player_settings[0].per_turn_incr_millis = 0;
    all_game_settings[1].player_settings[1].total_millis = 5 * MINUTE_MILLIS;
    all_game_settings[1].player_settings[1].per_turn_incr_millis = 0;

    // 5 minutes + 3 seconds        -f+t
    all_game_settings[2].flag_beep = false;
    all_game_settings[2].turn_beep = true;
    all_game_settings[2].player_settings[0].total_millis = 5 * MINUTE_MILLIS;
    all_game_settings[2].player_settings[0].per_turn_incr_millis = 3 * SECOND_MILLIS;
    all_game_settings[2].player_settings[1].total_millis = 5 * MINUTE_MILLIS;
    all_game_settings[2].player_settings[1].per_turn_incr_millis = 3 * SECOND_MILLIS;

    // 15 minutes                   -f-t
    all_game_settings[3].flag_beep = false;
    all_game_settings[3].turn_beep = false;
    all_game_settings[3].player_settings[0].total_millis = 15 * MINUTE_MILLIS;
    all_game_settings[3].player_settings[0].per_turn_incr_millis = 0;
    all_game_settings[3].player_settings[1].total_millis = 15 * MINUTE_MILLIS;
    all_game_settings[3].player_settings[1].per_turn_incr_millis = 0;
}

GameSettings all_game_settings[GAME_SETTINGS_LEN] = {

    {   // 30 seconds               +f+t
        {
            {
                30 * SECOND_MILLIS, // total_millis
                0                   // per_turn_incr_millis,
            },
            {
                30 * SECOND_MILLIS, // total_millis
                0                   // per_turn_incr_millis,
            },
        },
        true,                       // flag beep
        true                        // turn beep
    },

    {   // 5 minutes                -f+t
        {
            {
                5 * MINUTE_MILLIS,  // total_millis
                0                   // per_turn_incr_millis,
            },
            {
                5 * MINUTE_MILLIS,  // total_millis
                0                   // per_turn_incr_millis,
            },
        },
        false,                      // flag beep
        true                        // turn beep
    },

    {   // 5 minutes + 3 seconds    -f+t
        {
            {
                5 * MINUTE_MILLIS,  // total_millis
                3 * SECOND_MILLIS   // per_turn_incr_millis,
            },
            {
                5 * MINUTE_MILLIS,  // total_millis
                3 * SECOND_MILLIS   // per_turn_incr_millis,
            },
        },
        false,                      // flag beep
        true                        // turn beep
    },

    {   // 15 minutes               -f-t
        {
            {
                15 * MINUTE_MILLIS, // total_millis
                0                   // per_turn_incr_millis,
            },
            {
                15 * MINUTE_MILLIS, // total_millis
                0                   // per_turn_incr_millis,
            },
        },
        false,                      // flag beep
        false                       // turn beep
    }
};

byte _eeprom_update(extEEPROM *eeprom, unsigned long addr, byte value) {
    return eeprom->update(addr, value);
}

byte _eeprom_update(extEEPROM *eeprom, unsigned long addr, byte *values, unsigned int n_bytes) {
    unsigned int i;
    byte status = 0;
    for (i = 0; i < n_bytes; i++) {
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

int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, PlayerSettings *value) {
    int status = 0;
    status = eeprom->read(*eeprom_position, (byte *) &(value->per_turn_incr_millis), sizeof(value->per_turn_incr_millis));
    if (status != 0) {
        return -status;
    }
    (*eeprom_position) += sizeof(value->per_turn_incr_millis);
    status = eeprom->read(*eeprom_position, (byte *) &(value->total_millis), sizeof(value->total_millis));
    if (status != 0) {
        return -status;
    }
    (*eeprom_position) += sizeof(value->total_millis);
    return status;
}

int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, GameSettings *value) {
    int status = 0;

    byte flags = 0;
    status = eeprom->read(*eeprom_position, &flags, 1);
    (*eeprom_position)++;
    if (status < 0) {
        return status;
    }
    value->flag_beep = (flags & 0b01) ? true : false;
    value->turn_beep = (flags & 0b10) ? true : false;

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

int write_to_bytes(byte bytes[], unsigned long *offset, PlayerSettings *value) {
    memcpy(&(bytes[*offset]), (byte *) &(value->per_turn_incr_millis), sizeof(value->per_turn_incr_millis));
    (*offset) += sizeof(value->per_turn_incr_millis);
    memcpy(&(bytes[*offset]), (byte *) &(value->total_millis), sizeof(value->total_millis));
    (*offset) += sizeof(value->total_millis);
    return 0;
}

int write_to_bytes(byte bytes[], unsigned long *offset, GameSettings *value) {
    byte flags = 0b00000000;
    if (value->flag_beep) {
        flags |= 0b1;
    }
    if (value->turn_beep) {
        flags |= 0b10;
    }
    memcpy(&(bytes[*offset]), &flags, sizeof(flags));
    (*offset) += sizeof(flags);

    write_to_bytes(bytes, offset, &(value->player_settings[0]));
    write_to_bytes(bytes, offset, &(value->player_settings[1]));
    return 0;
}

int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, PlayerSettings *value) {
    int status = 0;

    status = _eeprom_update(eeprom, *eeprom_position, (byte *) &(value->per_turn_incr_millis), sizeof(value->per_turn_incr_millis));
    (*eeprom_position) += sizeof(value->per_turn_incr_millis);
    if (status != 0) {
        return -status;
    }

    status = _eeprom_update(eeprom, *eeprom_position, (byte *) &(value->total_millis), sizeof(value->total_millis));
    (*eeprom_position) += sizeof(value->total_millis);
    if (status != 0) {
        return -status;
    }
    return 0;
}

int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position, GameSettings *value) {
    int status = 0;
    byte flags = 0;

    if (value->flag_beep) {
        flags |= 0b1;
    }
    if (value->turn_beep) {
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
    return write_settings_to_eeprom(eeprom, &eeprom_offset);
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

const char * stringify_error(int error_status) {
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

