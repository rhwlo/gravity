#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"
#include <extEEPROM.h>


#define PLAYER_RIGHT_IDX 0
#define PLAYER_LEFT_IDX  1

#define VALIDATION_BYTE_0 0xE4
#define VALIDATION_BYTE_1 0xC5
#define GAME_SETTINGS_LEN 4

// Option Indexes (OI_...)
#define OI_P2_HOURS             0
#define OI_P2_TEN_MINUTES       1
#define OI_P2_MINUTES           2
#define OI_P2_TEN_SECONDS       3
#define OI_P2_SECONDS           4
#define OI_P1_HOURS             5
#define OI_P1_TEN_MINUTES       6
#define OI_P1_MINUTES           7
#define OI_P1_TEN_SECONDS       8
#define OI_P1_SECONDS           9
#define OI_TURN_TEN_SECONDS     10
#define OI_TURN_SECONDS         11
#define OI_FLAG_BEEP            12
#define OI_TURN_BEEP            13
#define OI_COUNT                14


// EEPROM communication error codes
#define GS_ERR_TOO_LONG         -1
#define GS_ERR_ADDR_SEND_NACK   -2
#define GS_ERR_DATA_SEND_NACK   -3
#define GS_ERR_OTHER_TWI_ERR    -4
#define GS_ERR_TIMEOUT          -5
#define GS_ERR_VALUE_MISMATCH   -6

enum ClockMode {
    CM_PAUSED,
    CM_ACTIVE,
    CM_SELECT_SETTINGS,
    CM_EDIT_SETTINGS
};

struct PlayerSettings
{
    unsigned long  total_millis;
    unsigned short per_turn_incr_millis;
};

struct GameSettings
{
    PlayerSettings player_settings[NUM_PLAYERS];
    bool flag_beep;      // beep when a player runs out of time
    bool turn_beep;      // beep when we change whose turn it is
};


int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, PlayerSettings *value);
int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, GameSettings *value);
int write_to_bytes(byte bytes[], unsigned long *offset, PlayerSettings *value);
int write_to_bytes(byte bytes[], unsigned long *offset, GameSettings *value);
int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, PlayerSettings *value);
int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, GameSettings *value);

int write_settings_to_eeprom(extEEPROM *eeprom);
int write_settings_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position);
int write_settings_to_bytes(byte bytes[]);
int write_settings_to_bytes(byte bytes[], unsigned long *offset);
int read_settings_from_eeprom(extEEPROM *eeprom);
int read_settings_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position);

void load_default_settings(void);

struct PlayerState {
    bool out_of_time;
    unsigned long remaining_millis;
};

class GameState {
    public:
        GameState(GameSettings *game_settings);

        bool set_turn(unsigned short new_turn);
        void reset(void);

        ClockMode clock_mode;
        int option_index;
        unsigned short whose_turn;
        PlayerState player_states[NUM_PLAYERS];
        PlayerState *curr_player_state;
        GameSettings *settings;
};

extern GameSettings all_game_settings[GAME_SETTINGS_LEN];
extern uint8_t selected_game_settings;


#endif  // GAME_STATE_H
