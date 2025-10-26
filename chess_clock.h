// pins
#define PLAYER1_BUTTON_PIN      A0
#define PLAYER1_LED_PIN         5
#define PLAYER2_BUTTON_PIN      A1
#define PLAYER2_LED_PIN         6
#define PAUSE_BUTTON_PIN        A3

// set LED_OFF_HIGH if the LED's non-output wire is wired to 5V rather than ground
#ifdef LED_OFF_HIGH
#define LED_OFF_LEVEL           255
#define LED_ON_LEVEL            220
#else
#define LED_OFF_LEVEL           0
#define LED_ON_LEVEL            35
#endif

// delay values (defined in milliseconds)
#define DEBOUNCE_DELAY          100
#define LONG_PRESS_DELAY        1000
#define PRINT_INTERVAL          75

#define PLAYER1_IDX 0
#define PLAYER2_IDX 1

