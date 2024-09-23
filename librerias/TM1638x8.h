#ifndef TM1638_H
#define TM1638_H

#include "mbed.h"

class TM1638 {
public:
    TM1638(PinName dio, PinName clk, PinName stb);
    void init();
    void displayDigit(uint8_t position, uint8_t digit, bool dot = false);
    void setDisplayToDecNumber(uint32_t number, bool leadingZeros = false, bool dot = false);
    void clearDisplay();
    void setBrightness(uint8_t brightness);
    void setLED(uint8_t position, bool state);
    uint8_t readButtons();

private:
    DigitalInOut _dio;
    DigitalOut _clk;
    DigitalOut _stb;

    void sendCommand(uint8_t cmd);
    void sendData(uint8_t address, uint8_t data);
    void start();
    void stop();
    void writeByte(uint8_t data);
    uint8_t readByte();
};

#endif






