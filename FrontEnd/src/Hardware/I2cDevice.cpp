#include "I2cDevice.h"

#include "Arduino.h"

namespace Hardware
{
    I2cDevice::I2cDevice(
        const pin_size_t sdaPin,
        const pin_size_t sclPin,
        const int clockFreq
    )
        : _i2c{ i2c0, sdaPin, sclPin }
    {
        _i2c.setClock(clockFreq);
        _i2c.begin();
    }

    bool I2cDevice::startTransmission(const uint8_t address)
    {
        Serial.printf("I2cDevice::startTransmission: address=%u\r\n", address);

        // Another transmission is already in progress
        if (address != _currentAddress && _currentAddress != 0) {
            Serial.println("I2cDevice::startTransmission: another transmission is in progress");
            return false;
        }

        // Repeated start
        if (_currentAddress != 0) {
            if (_i2c.endTransmission(false) != 0) {
                Serial.println("I2cDevice::startTransmission: failed to end before repeated start");
                _currentAddress = 0;
                return false;
            }
        } else {
            // Probably a requestFrom() call will follow, no need to call this in case of a repeated start
            _i2c.beginTransmission(address);
        }

        _currentAddress = address;

        return true;
    }

    bool I2cDevice::endTransmission()
    {
        Serial.printf("I2cDevice::endTransmission: address=%u\r\n", _currentAddress);

        if (_currentAddress == 0) {
            Serial.println("I2cDevice::endTransmission: no transmission is in progress");
            return false;
        }

        _currentAddress = 0;

        return true;
    }

    bool I2cDevice::write(
        const uint8_t* const data,
        const size_t length,
        [[maybe_unused]] const bool noStop
    ) {
        Serial.printf("I2cDevice::write: address=%u, length=%u\r\n", _currentAddress, length);

        if (_currentAddress == 0) {
            Serial.println("I2cDevice::write: no transmission is in progress");
            return false;
        }

        const auto written = _i2c.write(data, length);
        Serial.printf("I2cDevice::write: length=%u, written=%u\r\n", length, written);
        return written == length;
    }

    bool I2cDevice::read(
        uint8_t* const data,
        const size_t length,
        [[maybe_unused]] const bool noStop
    ) {
        Serial.printf("I2cDevice::read: address=%u, length=%u\r\n", _currentAddress, length);

        if (_currentAddress == 0) {
            Serial.println("I2cDevice::read: no transmission is in progress");
            return false;
        }

        const auto reqLength = _i2c.requestFrom(_currentAddress, length);
        const auto readLength = _i2c.readBytes(data, reqLength);

        Serial.printf("I2cDevice::read: length=%u, reqLength=%u, readLength=%u\r\n",
            length, reqLength, readLength);

        return readLength == length;
    }

}