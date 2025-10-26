#ifdef USE_MEMORY_DEBUGGING
#include "memory_debugging.h"

#include <LCD_I2C.h>

#include "src/game_state.h"

LCD_I2C lcd(PCF8574_ADDR_0, 16, 2);
extEEPROM eeprom(kbits_2, 1, 8);
#ifdef MEM_DEBUG
byte membuffer[256];
#endif

void eeprom_debug_setup() {
    pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CENTER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RIGHT_LED_PIN, OUTPUT);
    pinMode(LEFT_LED_PIN, OUTPUT);
    lcd.begin();
    lcd.backlight();
    lcd.cursor();
    lcd.clear();

    byte status = eeprom.begin(extEEPROM::twiClock400kHz);
    lcd.setCursor(0, 0);
    lcd.println("begin:");
    lcd.print("    status: 0x");
    lcd.print(status, 16);
    if (status != 0) {
        while (digitalRead(CENTER_BUTTON_PIN) == HIGH) {
            delay(20);
        }
    }
    #ifdef MEM_DEBUG
    // blank the membuffer
    for (uint16_t i = 0; i < 256; i++) {
        membuffer[i] = 0;
    }
    #endif // MEM_DEBUG
}

void do_write_incr_eeprom() {
    unsigned long addr = 0;
    byte value = 0x00, status = 0x00;
    for (addr = 0; addr < eeprom.length(); addr++) {
        value = addr % 256;
        status = 0x00;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("write(");
        lcd.print(addr, 16);
        lcd.print(", ");
        lcd.print(value, 16);
        lcd.print(")");
        lcd.setCursor(0, 1);
        status = eeprom.write(addr, value);
        lcd.print("    status: 0x");
        lcd.print(status, 16);
        if (status != 0x00) {
            while (digitalRead(CENTER_BUTTON_PIN) == HIGH) {
                delay(20);
            }
        }
        delay(500);
    }
}

void print_hex(byte value) {
    if (value < 0x10) {
        lcd.print('0');
    }
    lcd.print(value, HEX);
}

void do_read_eeprom() {
    unsigned long addr = 0;
    int i;
    byte status = 0x00, values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    while (true) {
        status = 0x00;
        status = eeprom.read(addr, values, 8);
        lcd.clear();
        lcd.setCursor(0, 0);
        if (status == 0) {
            print_hex(addr);
            for (i = 0; i < 4; i++) {
                lcd.print(' ');
                print_hex(values[i]);
            }
            lcd.setCursor(0, 1);
            lcd.print("  ");
            for (i = 4; i < 8; i++) {
                lcd.print(' ');
                print_hex(values[i]);
            }
        } else {
            lcd.print("read(");
            lcd.print(addr, 16);
            lcd.print(", values)");
            lcd.setCursor(0, 1);
            lcd.print("    status: 0x");
            lcd.print(status, 16);
        }
        delay(500);
        while (true) {
            if (digitalRead(CENTER_BUTTON_PIN) == LOW) {
                return;
            }
            if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
                if (addr >= 8) {
                    addr -= 8;
                }
                break;
            }
            if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
                if (addr < 0x100) {
                    addr += 8;
                }
                break;
            }
        }
    }
}

void do_blank_eeprom() {
    unsigned long addr;
    for (addr = 0; addr < eeprom.length(); addr++) {
        eeprom.write(addr, 0x00);
    }
}

#ifdef MEM_DEBUG
void do_wipe_bytes() {
    uint16_t i;
    for (i = 0; i < 256; i++) {
        membuffer[i] = 0;
    }
}

void do_view_bytes() {
    unsigned long offset = 0;
    int i;
    while (true) {
        lcd.clear();
        lcd.setCursor(0, 0);
        print_hex(offset);
        lcd.setCursor(2, 0);
        for (i = 0; i < 4; i++) {
            lcd.print(' ');
            print_hex(membuffer[offset + i]);
        }
        lcd.setCursor(2, 1);
        for (i = 4; i < 8; i++) {
            lcd.print(' ');
            print_hex(membuffer[offset + i]);
        }
        delay(500);
        while (true) {
            if (digitalRead(CENTER_BUTTON_PIN) == LOW) {
                return;
            }
            if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
                if (offset >= 8) {
                    offset -= 8;
                }
                break;
            }
            if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
                if (offset < 0x100) {
                    offset += 8;
                }
                break;
            }
        }
    }
}

void mem_debug_loop() {
    bool write_settings = false, view_bytes = false, wipe_bytes = false;
    int status = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wipe bytes?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            wipe_bytes = true;
            break;
        }
    }
    if (wipe_bytes) {
        do_wipe_bytes();
    }
    delay(100);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Write?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            write_settings = true;
            break;
        }
    }
    if (write_settings) {
        status = write_settings_to_bytes(membuffer);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("status: ok");
        lcd.setCursor(0, 1);
        lcd.print(status);
        lcd.print(" bytes");
    }
    delay(100);
    while (digitalRead(CENTER_BUTTON_PIN) == HIGH) {
        delay(20);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Read bytes?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            view_bytes = true;
            break;
        }
    }
    if (view_bytes) {
        do_view_bytes();
    }
    delay(100);
} 
#endif // MEM_DEBUG

void wait_for_state(uint8_t pin, int state) {
    while (digitalRead(pin) != state) {
        delay(20);
    }
    return;
}

void eeprom_debug_loop() {
    bool read_eeprom = false, blank_eeprom = false, write_settings = false;
    int status = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Read eeprom?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            wait_for_state(LEFT_BUTTON_PIN, HIGH);
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            wait_for_state(RIGHT_BUTTON_PIN, HIGH);
            read_eeprom = true;
            break;
        }
    }
    if (read_eeprom) {
        do_read_eeprom();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Blank eeprom?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            wait_for_state(LEFT_BUTTON_PIN, HIGH);
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            blank_eeprom = true;
            wait_for_state(RIGHT_BUTTON_PIN, HIGH);
            break;
        }
    }
    if (blank_eeprom) {
        do_blank_eeprom();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Write settings?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            wait_for_state(LEFT_BUTTON_PIN, HIGH);
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            write_settings = true;
            wait_for_state(RIGHT_BUTTON_PIN, HIGH);
            break;
        }
    }
    if (write_settings) {
        status = write_settings_to_eeprom(&eeprom);
    }
    if (status < 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("status: ");
        lcd.print(status);
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("status: ok");
        lcd.setCursor(0, 1);
        lcd.print(status);
        lcd.print(" bytes");
    }
    wait_for_state(CENTER_BUTTON_PIN, LOW);
    wait_for_state(CENTER_BUTTON_PIN, HIGH);
}
#endif // USE_MEMORY_DEBUGGING
