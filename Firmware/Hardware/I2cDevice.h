#pragma once

#include <hardware/gpio.h>

#include <cstdint>

namespace Hardware
{
    class I2cDevice
    {
    public:
        I2cDevice(uint sdaPin, uint sclPin, int clockFreq = 100 * 1000);

        bool startTransmission(uint8_t address);
        bool endTransmission();

        bool write(const uint8_t* data, size_t length, bool noStop = false);
        bool read(uint8_t* data, size_t length, bool noStop = false);

    private:
        uint8_t _currentAddress = 0;
    };
}