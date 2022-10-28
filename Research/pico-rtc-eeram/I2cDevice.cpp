#include "I2cDevice.h"

#include <hardware/i2c.h>

#include <cstdio>

#define DEBUG_I2C_FUNCTIONS 1

namespace Hardware
{
    I2cDevice::I2cDevice(
        const uint sdaPin,
        const uint sclPin,
        const int clockFreq
    )
    {
        gpio_set_function(sdaPin, GPIO_FUNC_I2C);
        gpio_set_function(sclPin, GPIO_FUNC_I2C);

        const auto baudrate = i2c_init(i2c0, 100 * 1000);
        printf("I2cDevice: baudrate: %u\r\n", baudrate);
    }

    bool I2cDevice::startTransmission(const uint8_t address)
    {
        printf("I2cDevice::startTransmission: address=%u\r\n", address);

        // Another transmission is already in progress
        if (address != _currentAddress && _currentAddress != 0) {
            printf("I2cDevice::startTransmission: another transmission is in progress\r\n`");
            return false;
        }

        if (_currentAddress == 0) {
            i2c0->hw->enable = 0;
            i2c0->hw->tar = address;
            i2c0->hw->enable = 1;
            i2c0->restart_on_next = false;
        } else {
            // Repeated start
            i2c0->restart_on_next = true;
        }

        printf(
            "I2cDevice::startTransmission: i2c0->restart_on_next=%u\r\n",
            i2c0->restart_on_next
        );

        _currentAddress = address;

        return true;
    }

    bool I2cDevice::endTransmission()
    {
        printf("I2cDevice::endTransmission: address=%u\r\n", _currentAddress);

        if (_currentAddress == 0) {
            printf("I2cDevice::endTransmission: no transmission is in progress\r\n");
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
        printf("I2cDevice::write: address=%u, length=%u\r\n", _currentAddress, length);

#if DEBUG_I2C_FUNCTIONS
        printf(
            "I2cDevice::write: data=%p, length=%u, noStop=%u\r\n",
            data,
            length,
            noStop
        );

        printf("I2cDevice::write:");
#endif

        for (auto i = 0u; i < length; ++i) {
            const bool first = i == 0;
            const bool last = i == length - 1;

#if DEBUG_I2C_FUNCTIONS
            printf(" %02x", data[i]);
#endif

            i2c0->hw->data_cmd =
                bool_to_bit(first && i2c0->restart_on_next) << I2C_IC_DATA_CMD_RESTART_LSB |
                bool_to_bit(last && !noStop) << I2C_IC_DATA_CMD_STOP_LSB |
                data[i];

            do {
                tight_loop_contents();
            } while (!(i2c0->hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_EMPTY_BITS));
        }

#if DEBUG_I2C_FUNCTIONS
        printf("\r\nI2cDevice::write: finished\r\n");
#endif

        i2c0->restart_on_next = false;

        return true;
    }

    bool I2cDevice::read(
        uint8_t* const data,
        const size_t length,
        [[maybe_unused]] const bool noStop
    ) {
        printf("I2cDevice::read: address=%u, length=%u\r\n", _currentAddress, length);

        if (_currentAddress == 0) {
            printf("I2cDevice::read: no transmission is in progress");
            return false;
        }

#if DEBUG_I2C_FUNCTIONS
        printf(
            "I2cDevice::read: data=%p, length=%u, noStop=%u\r\n",
            data,
            length,
            noStop
        );
#endif

        auto abort = false;

#if DEBUG_I2C_FUNCTIONS
        printf("I2cDevice::read:");
#endif

        for (auto i = 0u; i < length; ++i) {
            const bool first = i == 0;
            const bool last = i == length - 1;

            while (!i2c_get_write_available(i2c0))
                tight_loop_contents();

            i2c0->hw->data_cmd =
                bool_to_bit(first && i2c0->restart_on_next) << I2C_IC_DATA_CMD_RESTART_LSB |
                bool_to_bit(last && !noStop) << I2C_IC_DATA_CMD_STOP_LSB |
                I2C_IC_DATA_CMD_CMD_BITS;

            do {
                abort = static_cast<bool>(i2c0->hw->clr_tx_abrt);
            } while (!abort && !i2c_get_read_available(i2c0));

            if (abort)
                break;

            data[i] = static_cast<uint8_t>(i2c0->hw->data_cmd);

#if DEBUG_I2C_FUNCTIONS
            printf(" %02x", data[i]);
#endif
        }

#if DEBUG_I2C_FUNCTIONS
        printf("\r\nI2cDevice::read: finished\r\n");
#endif

        i2c0->restart_on_next = false;

        return !abort;
    }

}