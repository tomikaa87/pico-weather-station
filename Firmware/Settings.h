#pragma once

#include "Drivers/EERAM_47xxx.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

namespace Hardware {
    class I2cDevice;
}

class Settings
{
public:
    Settings(Hardware::I2cDevice& i2cDevice);

    template <typename T>
    using Result = std::pair<bool, T>;

    struct TouchPadCalibrationData {
        int16_t xMin{ std::numeric_limits<int16_t>::max() };
        int16_t xMax{ std::numeric_limits<int16_t>::min() };
        int16_t yMin{ std::numeric_limits<int16_t>::max() };
        int16_t yMax{ std::numeric_limits<int16_t>::min() };
    };

    Result<TouchPadCalibrationData> getTouchPadCalibrationData() const;
    bool setTouchPadCalibrationData(const TouchPadCalibrationData& value) const;

private:
    EERAM_47xxx_Device _eeram;
};