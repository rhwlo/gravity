#include <LCD_I2C.h>
#include "game_state.h"
#include "display_lcd.h"

const uint8_t real_bold_digits[10][4] = {
    { 0xFE, 0xFE, 0x03, 0x02 },
    { 0xFE, 0x05, 0xFE, 0x05 },
    { 0x00, 0x02, 0x03, 0x01 },
    { 0x00, 0x02, 0x01, 0x02 },
    { 0x07, 0x01, 0xFE, 0x05 },
    { 0x07, 0x00, 0x01, 0x02 },
    { 0x07, 0x00, 0x06, 0x02 },
    { 0x00, 0x04, 0xFE, 0x05 },
    { 0x03, 0x02, 0x03, 0x02 },
    { 0x03, 0x02, 0x01, 0x04 }
};

LCDDisplay::LCDDisplay(uint8_t addr_1, uint8_t addr_2) :
    player_1(addr_1, 16, 2),
    player_2(addr_2, 16, 2) {
}

void LCDDisplay::begin(void) {
    player_1.begin();
    player_2.begin();
}

void LCDDisplay::printChar(LCD_I2C *lcd, char c, uint8_t col, uint8_t row) {
    switch (c)
    {
    case ' ':
        lcd->setCursor(col, row);
        lcd->write(' ');
        lcd->setCursor(col, row+1);
        lcd->write(' ');
        break;
    case ':':
        lcd->setCursor(col, row);
        lcd->write('.');
        lcd->setCursor(col, row+1);
        lcd->write('.');
        break;
    case '0':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[0][0]);
        lcd->write(real_bold_digits[0][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[0][1]);
        lcd->write(real_bold_digits[0][3]);
        break;
    case '1':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[1][0]);
        lcd->write(real_bold_digits[1][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[1][1]);
        lcd->write(real_bold_digits[1][3]);
        break;
    case '2':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[2][0]);
        lcd->write(real_bold_digits[2][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[2][1]);
        lcd->write(real_bold_digits[2][3]);
        break;
    case '3':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[3][0]);
        lcd->write(real_bold_digits[3][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[3][1]);
        lcd->write(real_bold_digits[3][3]);
        break;
    case '4':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[4][0]);
        lcd->write(real_bold_digits[4][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[4][1]);
        lcd->write(real_bold_digits[4][3]);
        break;
    case '5':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[5][0]);
        lcd->write(real_bold_digits[5][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[5][1]);
        lcd->write(real_bold_digits[5][3]);
        break;
    case '6':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[6][0]);
        lcd->write(real_bold_digits[6][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[6][1]);
        lcd->write(real_bold_digits[6][3]);
        break;
    case '7':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[7][0]);
        lcd->write(real_bold_digits[7][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[7][1]);
        lcd->write(real_bold_digits[7][3]);
        break;
    case '8':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[8][0]);
        lcd->write(real_bold_digits[8][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[8][1]);
        lcd->write(real_bold_digits[8][3]);
        break;
    case '9':
        lcd->setCursor(col, row);
        lcd->write(real_bold_digits[9][0]);
        lcd->write(real_bold_digits[9][2]);
        lcd->setCursor(col, row+1);
        lcd->write(real_bold_digits[9][1]);
        lcd->write(real_bold_digits[9][3]);
        break;
    default:
        break;
    }
}

char charForDigit(uint8_t i) {
    switch (i)
    {
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    default:
        return ' ';
    }
}

// TODO: rework this to load a buffer so that we can write this in a single pass
void LCDDisplay::prettyPrintTime(LCD_I2C *lcd, unsigned long time) {
    unsigned long remaining_time = time;
    if (remaining_time >= HOUR_MILLIS) {
        printChar(lcd, charForDigit((uint8_t) (remaining_time / HOUR_MILLIS)), 0, 0);
    } else {
        printChar(lcd, '0', 0, 0);
    }
    remaining_time %= HOUR_MILLIS;

    printChar(lcd, ':', 2, 0);

    if (remaining_time >= MINUTE_MILLIS * 10) {
        printChar(lcd, charForDigit((uint8_t) (remaining_time / MINUTE_MILLIS / 10)), 3, 0);
    } else {
        printChar(lcd, '0', 3, 0);
    }
    if (remaining_time >= MINUTE_MILLIS) {
        printChar(lcd, charForDigit((uint8_t) (remaining_time / MINUTE_MILLIS % 10)), 5, 0);
    } else {
        printChar(lcd, '0', 5, 0);
    }
    remaining_time %= MINUTE_MILLIS;

    printChar(lcd, ':', 7, 0);

    if (remaining_time >= SECOND_MILLIS * 10) {
        printChar(lcd, charForDigit((uint8_t) (remaining_time / SECOND_MILLIS / 10)), 8, 0);
    } else {
        printChar(lcd, '0', 8, 0);
    }
    if (remaining_time >= SECOND_MILLIS) {
        printChar(lcd, charForDigit((uint8_t) (remaining_time / SECOND_MILLIS % 10)), 10, 0);
    } else {
        printChar(lcd, '0', 10, 0);
    }
}

void LCDDisplay::renderGameState(GameState *game_state) {

}