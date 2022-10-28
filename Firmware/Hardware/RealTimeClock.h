#pragma once

#include "../Drivers/RTC_MCP7940N.h"

namespace Hardware
{
    class I2cDevice;

    class RealTimeClock
    {
    public:
        RealTimeClock(I2cDevice& i2c);

        // TODO only for testing, remove this
        const RTC_MCP7940N_Device* device() const
        {
            return &_device;
        }

    private:
        I2cDevice& _i2c;
        RTC_MCP7940N_Device _device;
    };
}