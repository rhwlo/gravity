#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "common.h"
#include <extEEPROM.h>


#define PLAYER_RIGHT_IDX 0
#define PLAYER_LEFT_IDX  1

#define VALIDATION_BYTE_0 0xE4
#define VALIDATION_BYTE_1 0xC5
#define GAME_SETTINGS_LEN 4

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

enum clock_mode_t {
    CM_PAUSED,
    CM_ACTIVE,
    CM_SELECT_SETTINGS,
    CM_EDIT_SETTINGS
};

struct player_settings_t
{
    unsigned long  totalMillis;
    unsigned short perTurnIncrMillis;
};

struct game_settings_t
{
    player_settings_t player_settings[NUM_PLAYERS];
    bool flagBeep;      // beep when a player runs out of time
    bool turnBeep;      // beep when we change whose turn it is
};


int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, player_settings_t *value);
int write_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, game_settings_t *value);
int write_to_bytes(byte bytes[], unsigned long *offset, player_settings_t *value);
int write_to_bytes(byte bytes[], unsigned long *offset, game_settings_t *value);
int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, player_settings_t *value);
int read_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_offset, game_settings_t *value);

int write_settings_to_eeprom(extEEPROM *eeprom);
int write_settings_to_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position);
int write_settings_to_bytes(byte bytes[]);
int write_settings_to_bytes(byte bytes[], unsigned long *offset);
int read_settings_from_eeprom(extEEPROM *eeprom);
int read_settings_from_eeprom(extEEPROM *eeprom, unsigned long *eeprom_position);

void load_default_settings(void);

struct player_state_t {
    bool outOfTime;
    unsigned long remainingMillis;
};

class GameState {
    public:
        GameState(game_settings_t *game_settings);
        bool setTurn(unsigned short newTurn);
        clock_mode_t clock_mode;
        int option_index;
        void reset(void);
        unsigned short whoseTurn;
        player_state_t player_states[NUM_PLAYERS];
        player_state_t *curr_player_state;
        game_settings_t *settings;
};

extern game_settings_t all_game_settings[GAME_SETTINGS_LEN];
extern uint8_t selected_game_settings;


#endif  // GAME_STATE_H
