#include "EnvironmentSensor.h"

#include "I2cDevice.h"

#include <pico/time.h>

#include <cstdio>

namespace Hardware
{
    EnvironmentSensor::EnvironmentSensor(I2cDevice& i2c)
        : _i2c{ i2c }
    {
        if (const auto result = setupBme280(); result != 0) {
            printf("setupBme280: failed, result=%d\r\n", result);
        } 
    }

    float EnvironmentSensor::temperatureCelsius() const
    {
        return _bme280.lastReading.temperature
            + _bme280.pcbHeatTemperatureOffset;
    }

    float EnvironmentSensor::pressurePa() const
    {
        return _bme280.lastReading.pressure;
    }

    float EnvironmentSensor::relativeHumidity() const
    {
        return _bme280.lastReading.humidity;
    }

    int EnvironmentSensor::bme280InitResult() const
    {
        return _bme280.initResult;
    }

    void EnvironmentSensor::run()
    {
        const auto timestamp = to_ms_since_boot(get_absolute_time());

        if (timestamp - _bme280.updateTimestamp >= _bme280.updateIntervalMs) {
            _bme280.updateTimestamp = timestamp;

            if (const auto result = updateBme280Data(); result != 0) {
                printf("updateBme280Data: failed, result=%d\r\n", result);
            }
        }
    }

    int EnvironmentSensor::setupBme280()
    {
        _bme280.dev.intf_ptr = &_i2c;

        _bme280.dev.intf = BME280_I2C_INTF;

        _bme280.dev.read = [](
            const uint8_t reg_addr,
            uint8_t* const reg_data,
            const uint32_t len,
            void* const intf_ptr
        ) -> BME280_INTF_RET_TYPE {
            auto* i2c = reinterpret_cast<Hardware::I2cDevice*>(intf_ptr);
            if (!i2c->startTransmission(BME280_I2C_ADDR_PRIM)) {
                printf("_bme280.dev.read: start failed\r\n");
            }
            if (!i2c->write(&reg_addr, 1)) {
                printf("_bme280.dev.read: write failed\r\n");
            }
            if (!i2c->startTransmission(BME280_I2C_ADDR_PRIM)) {
                printf("_bme280.dev.read: restart failed\r\n");
            }
            if (!i2c->read(reg_data, len)) {
                printf("_bme280.dev.read: read failed\r\n");
            }
            if (!i2c->endTransmission()) {
                printf("_bme280.dev.read: end failed\r\n");
            }
            printf("_bme280.dev.read: a=%02x, l=%u, d=", reg_addr, len);
            for (auto i = 0u; i < len; ++i) {
                printf("%02x", reg_data[i]);
                if (i < len - 1) {
                    printf(",");
                }
            }
            printf("\r\n");
            return BME280_INTF_RET_SUCCESS;
        };

        _bme280.dev.write = [](
            const uint8_t reg_addr,
            const uint8_t* const reg_data,
            const uint32_t len,
            void* const intf_ptr
        ) -> BME280_INTF_RET_TYPE {
            auto* i2c = reinterpret_cast<Hardware::I2cDevice*>(intf_ptr);
            printf("_bme280.dev.write: a=%x, l=%u, d=", reg_addr, len);
            for (auto i = 0u; i < len; ++i) {
                printf("%02x", reg_data[i]);
                if (i < len - 1) {
                    printf(",");
                }
            }
            printf("\r\n");
            if (!i2c->startTransmission(BME280_I2C_ADDR_PRIM)) {
                printf("_bme280.dev.write: start failed\r\n");
            }
            if (!i2c->write(&reg_addr, 1, true)) {
                printf("_bme280.dev.write: write reg addr failed\r\n");
            }
            if (!i2c->write(reg_data, len)) {
                printf("_bme280.dev.write: write data failed\r\n");
            }
            if (!i2c->endTransmission()) {
                printf("_bme280.dev.read: end failed\r\n");
            }
            return BME280_INTF_RET_SUCCESS;
        };

        _bme280.dev.delay_us = [](
            const uint32_t period,
            [[maybe_unused]] void* const intf_ptr
        ) {
            sleep_us(period);
        };

        printf("Initializing BME280\r\n");
        _bme280.initResult = bme280_init(&_bme280.dev);

        if (_bme280.initResult != 0) {
            return _bme280.initResult;
        }

        /* Recommended mode of operation: Weather monitoring */
        _bme280.dev.settings.osr_h = BME280_OVERSAMPLING_1X;
        _bme280.dev.settings.osr_p = BME280_OVERSAMPLING_1X;
        _bme280.dev.settings.osr_t = BME280_OVERSAMPLING_1X;
        _bme280.dev.settings.filter = BME280_FILTER_COEFF_OFF;

        const uint8_t settingsSel = BME280_OSR_PRESS_SEL
            | BME280_OSR_TEMP_SEL
            | BME280_OSR_HUM_SEL
            | BME280_FILTER_SEL;

        printf("Configuring BME280 sensor settings\r\n");

        /* Set the sensor settings */
        auto result = bme280_set_sensor_settings(settingsSel, &_bme280.dev);
        if (result != BME280_OK)
        {
            printf("Failed to set sensor settings (code %+d).\r\n", result);

            return result;
        }

        /* Calculate the minimum delay required between consecutive
           measurement based upon the sensor enabled 
           and the oversampling configuration. */
        _bme280.requiredMeasurementDelay
            = bme280_cal_meas_delay(&_bme280.dev.settings);

        printf("req_delay: %u\r\n", _bme280.requiredMeasurementDelay);

        return 0;
    }

    int EnvironmentSensor::updateBme280Data()
    {
        /* Variable to define the result */
        int8_t result = BME280_OK;

        /* Set the sensor to forced mode */
        result = bme280_set_sensor_mode(BME280_FORCED_MODE, &_bme280.dev);
        if (result != BME280_OK)
        {
            printf("Failed to set sensor mode (code %+d).\r\n", result);
            return 1;
        }

        /* Wait for the measurement to complete and print data */
        sleep_ms(_bme280.requiredMeasurementDelay);

        /* Structure to get the pressure, temperature and humidity values */
        struct bme280_data data;
        result = bme280_get_sensor_data(BME280_ALL, &data, &_bme280.dev);
        if (result != BME280_OK)
        {
            printf("Failed to get sensor data (code %+d).\r\n", result);
            return 1;
        }

        _bme280.lastReading = data;

        return result;
    }
}