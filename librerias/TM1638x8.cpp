#include "TM1638x8.h"

static const uint8_t digitToSegment[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

TM1638::TM1638(PinName dio, PinName clk, PinName stb) : _dio(dio), _clk(clk), _stb(stb) {
    _stb = 1;
    _clk = 1;
    _dio.output();
}

void TM1638::init() {
    sendCommand(0x8F);  // Display ON, mas vrillo
    clearDisplay();
}

void TM1638::displayDigit(uint8_t position, uint8_t digit, bool dot) {
    sendData(position << 1, digitToSegment[digit] | (dot ? 0x80 : 0x00));
}

void TM1638::setDisplayToDecNumber(uint32_t number, bool leadingZeros, bool dot) {
    for (int i = 0; i < 8; i++) {
        uint8_t digit = number % 10;
        if (number != 0 || i == 0 || leadingZeros) {
            displayDigit(7 - i, digit, (i == 0) ? dot : false);
        } else {
            sendData((7 - i) << 1, 0x00);
        }
        number /= 10;
    }
}

void TM1638::clearDisplay() {
    for (int i = 0; i < 16; i++) {
        sendData(i, 0x00);
    }
}

void TM1638::setLED(uint8_t position, bool state) {
    sendData((position << 1) + 1, state ? 1 : 0);
}

void TM1638::setBrightness(uint8_t brightness) {
    sendCommand(0x87 + brightness);
}

void TM1638::sendCommand(uint8_t cmd) {
    start();
    writeByte(cmd);
    stop();
}

void TM1638::sendData(uint8_t address, uint8_t data) {
    sendCommand(0x44);
    start();
    writeByte(0xC0 | address);
    writeByte(data);
    stop();
}

void TM1638::start() {
    _stb = 0;
}

void TM1638::stop() {
    _stb = 1;
}

void TM1638::writeByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        _clk = 0;
        _dio = (data & 1) ? 1 : 0;
        data >>= 1;
        _clk = 1;
    }
}

uint8_t TM1638::readButtons() {
    uint8_t buttons = 0;
    start();
    writeByte(0x42);  // Read command
    
    _dio.input();
    for (int i = 0; i < 4; i++) {
        uint8_t v = readByte();
        buttons |= v << i;
    }
    _dio.output();
    
    stop();
    return buttons;
}

uint8_t TM1638::readByte() {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        _clk = 0;
        wait_us(1);  
        byte >>= 1;
        if (_dio) {
            byte |= 0x80;
        }
        _clk = 1;
        wait_us(1);  
    }
    return byte;
}








