#include "Settings.h"

#include "Hardware/I2cDevice.h"

namespace
{
    enum class Address : std::size_t
    {
        TouchPadCalibrationData = 0x0000
    };
}

Settings::Settings(Hardware::I2cDevice& i2c)
{
    _eeram.a1 = 0;
    _eeram.a2 = 0;

    _eeram.i2cFunctionArg = &i2c;

    _eeram.i2cStartTransmission = [](
        void* const arg,
        const uint8_t address
    ) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->startTransmission(address);
    };

    _eeram.i2cEndTransmission = [](void* const arg) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->endTransmission();
    };

    _eeram.i2cRead = [](
        void* const arg,
        uint8_t* const data,
        const size_t length,
        const bool noStop
    ) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->read(data, length, noStop);
    };

    _eeram.i2cWrite = [](
        void* const arg,
        const uint8_t* const data,
        const size_t length,
        const bool noStop
    ) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->write(data, length, noStop);
    };
}

Settings::Result<Settings::TouchPadCalibrationData>
Settings::getTouchPadCalibrationData() const
{
    return std::make_pair(false, TouchPadCalibrationData{});
}

bool Settings::setTouchPadCalibrationData(const TouchPadCalibrationData& value) const
{
    return false;
}