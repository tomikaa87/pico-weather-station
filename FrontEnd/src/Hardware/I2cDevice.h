#pragma once

#include "Wire.h"

namespace Hardware
{
    class I2cDevice
    {
    public:
        I2cDevice(pin_size_t sdaPin, pin_size_t sclPin, int clockFreq = 100 * 1000);

        bool startTransmission(uint8_t address);
        bool endTransmission();

        bool write(const uint8_t* data, size_t length, bool noStop = false);
        bool read(uint8_t* data, size_t length, bool noStop = false);

    private:
        TwoWire _i2c;
        uint8_t _currentAddress = 0;
    };
}