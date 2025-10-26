#ifdef USE_MEMORY_DEBUGGING
#ifndef MEMORY_DEBUGGING_H
#define MEMORY_DEBUGGING_H

#define PCF8574_ADDR_0          0x27        // A0 jumper is soldered
#define RIGHT_BUTTON_PIN        A0
#define RIGHT_LED_PIN           5
#define LEFT_BUTTON_PIN         A1
#define LEFT_LED_PIN            6
#define CENTER_BUTTON_PIN       A3

void eeprom_debug_loop();
void eeprom_debug_setup();

#endif // MEMORY_DEBUGGING_H
#endif // USE_MEMORY_DEBUGGING
