#ifdef USE_MEMORY_DEBUGGING
#include "memory_debugging.h"

#include <LCD_I2C.h>

#include "src/game_state.h"

LCD_I2C lcd(PCF8574_ADDR_0, 16, 2);
extEEPROM eeprom(kbits_2, 1, 8);
#ifdef MEM_DEBUG
byte membuffer[256];
#endif

void eepromDebugSetup() {
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

void doWriteIncrEeprom() {
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

void printHex(byte value) {
    if (value < 0x10) {
        lcd.print('0');
    }
    lcd.print(value, HEX);
}

void doReadEeprom() {
    unsigned long addr = 0;
    int i;
    byte status = 0x00, values[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    while (true) {
        status = 0x00;
        status = eeprom.read(addr, values, 8);
        lcd.clear();
        lcd.setCursor(0, 0);
        if (status == 0) {
            printHex(addr);
            for (i = 0; i < 4; i++) {
                lcd.print(' ');
                printHex(values[i]);
            }
            lcd.setCursor(0, 1);
            lcd.print("  ");
            for (i = 4; i < 8; i++) {
                lcd.print(' ');
                printHex(values[i]);
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

void doBlankEeprom() {
    unsigned long addr;
    for (addr = 0; addr < eeprom.length(); addr++) {
        eeprom.write(addr, 0x00);
    }
}

#ifdef MEM_DEBUG
void doWipeBytes() {
    uint16_t i;
    for (i = 0; i < 256; i++) {
        membuffer[i] = 0;
    }
}

void doViewBytes() {
    unsigned long offset = 0;
    int i;
    while (true) {
        lcd.clear();
        lcd.setCursor(0, 0);
        printHex(offset);
        lcd.setCursor(2, 0);
        for (i = 0; i < 4; i++) {
            lcd.print(' ');
            printHex(membuffer[offset + i]);
        }
        lcd.setCursor(2, 1);
        for (i = 4; i < 8; i++) {
            lcd.print(' ');
            printHex(membuffer[offset + i]);
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

void memDebugLoop() {
    bool writeSettings = false, viewBytes = false, wipeBytes = false;
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
            wipeBytes = true;
            break;
        }
    }
    if (wipeBytes) {
        doWipeBytes();
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
            writeSettings = true;
            break;
        }
    }
    if (writeSettings) {
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
            viewBytes = true;
            break;
        }
    }
    if (viewBytes) {
        doViewBytes();
    }
    delay(100);
} 
#endif // MEM_DEBUG

void waitForState(uint8_t pin, int state) {
    while (digitalRead(pin) != state) {
        delay(20);
    }
    return;
}

void eepromDebugLoop() {
    bool readEeprom = false, blankEeprom = false, writeSettings = false;
    int status = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Read eeprom?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            waitForState(LEFT_BUTTON_PIN, HIGH);
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            waitForState(RIGHT_BUTTON_PIN, HIGH);
            readEeprom = true;
            break;
        }
    }
    if (readEeprom) {
        doReadEeprom();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Blank eeprom?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            waitForState(LEFT_BUTTON_PIN, HIGH);
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            blankEeprom = true;
            waitForState(RIGHT_BUTTON_PIN, HIGH);
            break;
        }
    }
    if (blankEeprom) {
        doBlankEeprom();
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Write settings?");
    lcd.setCursor(0, 1);
    lcd.print("no <-> yes");
    while (true) {
        if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
            waitForState(LEFT_BUTTON_PIN, HIGH);
            break;
        }
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
            writeSettings = true;
            waitForState(RIGHT_BUTTON_PIN, HIGH);
            break;
        }
    }
    if (writeSettings) {
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
    waitForState(CENTER_BUTTON_PIN, LOW);
    waitForState(CENTER_BUTTON_PIN, HIGH);
}
#endif // USE_MEMORY_DEBUGGING