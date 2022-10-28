#include "RealTimeClock.h"

#include "I2cDevice.h"

namespace Hardware
{
    RealTimeClock::RealTimeClock(I2cDevice& i2c)
        : _i2c{ i2c }
    {
        _device.i2cFunctionArg = &_i2c;

        _device.i2cStartTransmission = [](
            void* const arg,
            const uint8_t address
        ) {
            auto* const i2c = reinterpret_cast<I2cDevice*>(arg);
            return i2c->startTransmission(address);
        };

        _device.i2cEndTransmission = [](void* const arg) {
            auto* const i2c = reinterpret_cast<I2cDevice*>(arg);
            return i2c->endTransmission();
        };

        _device.i2cRead = [](
            void* const arg,
            uint8_t* const data,
            const size_t length,
            const bool noStop
        ) {
            auto* const i2c = reinterpret_cast<I2cDevice*>(arg);
            return i2c->read(data, length, noStop);
        };

        _device.i2cWrite = [](
            void* const arg,
            const uint8_t* const data,
            const size_t length,
            const bool noStop
        ) {
            auto* const i2c = reinterpret_cast<I2cDevice*>(arg);
            return i2c->write(data, length, noStop);
        };
    }
}