#pragma once

#include "../IRunnable.h"

#include "../Drivers/bme280.h"

#include <cstdint>

namespace Hardware
{
    class I2cDevice;

    class EnvironmentSensor : public IRunnable
    {
    public:
        EnvironmentSensor(I2cDevice& i2c);

        [[nodiscard]] float temperatureCelsius() const;
        [[nodiscard]] float pressurePa() const;
        [[nodiscard]] float relativeHumidity() const;

        [[nodiscard]] int bme280InitResult() const;

        void run() override;

    private:
        I2cDevice& _i2c;

        struct Bme280 {
            struct bme280_dev dev{};
            struct bme280_data lastReading{};
            int initResult = -1;
            uint32_t requiredMeasurementDelay = 0;
            uint32_t updateTimestamp = 0;
            uint32_t updateIntervalMs = 5000;
            float pcbHeatTemperatureOffset = 0;
        } _bme280;

        int setupBme280();
        int updateBme280Data();
    };
}