#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdio.h"

#include <cstdio>

#include "I2cDevice.h"
#include <EERAM_47xxx.h>
#include <RTC_MCP7940N.h>

#define RTC_SET_DATETIME 0
#define RTC_WRITE_TEST_DATA 0
#define EERAM_WRITE_TEST_DATA 0

namespace Pins::Uart
{
    constexpr auto TX = 12;
    constexpr auto RX = 13;
}

namespace Pins::I2C
{
    constexpr auto SDA = 16;
    constexpr auto SCL = 17;
}

namespace Pins::RTC
{
    constexpr auto MFP = 18;
}

namespace I2cFunctions
{
    bool startTransmission(void* const arg, const uint8_t address)
    {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->startTransmission(address);
    }

    bool endTransmission(void* const arg)
    {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->endTransmission();
    }

    bool write(
        void* const arg,
        const uint8_t* const data,
        const size_t length,
        const bool noStop
    )
    {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->write(data, length, noStop);
    }

    bool read(
        void* const arg,
        uint8_t* const data,
        const size_t length,
        const bool noStop
    )
    {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->read(data, length, noStop);
    }
}

RTC_MCP7940N_Device setupRtc(Hardware::I2cDevice& i2c)
{
    gpio_init(Pins::RTC::MFP);
    gpio_set_dir(Pins::RTC::MFP, false);
    // MFP is open-drain and requires a pull-up to VCC
    gpio_set_pulls(Pins::RTC::MFP, true, false);

    RTC_MCP7940N_Device device;
    device.i2cFunctionArg = &i2c;
    device.i2cStartTransmission = I2cFunctions::startTransmission; 
    device.i2cEndTransmission = I2cFunctions::endTransmission;
    device.i2cRead = I2cFunctions::read;
    device.i2cWrite = I2cFunctions::write;

    return device;
}

EERAM_47xxx_Device setupEeram(Hardware::I2cDevice& i2c)
{
    EERAM_47xxx_Device device;
    device.a1 = 0;
    device.a2 = 0;
    device.i2cFunctionArg = &i2c;
    device.i2cStartTransmission = I2cFunctions::startTransmission; 
    device.i2cEndTransmission = I2cFunctions::endTransmission;
    device.i2cRead = I2cFunctions::read;
    device.i2cWrite = I2cFunctions::write;

    return device;
}

int main()
{
    gpio_set_function(Pins::Uart::RX, GPIO_FUNC_UART);
    gpio_set_function(Pins::Uart::TX, GPIO_FUNC_UART);

    stdio_init_all();

    printf("***\r\nPico RTC MCP7940N and EERAM 47xxx test program\r\n***\r\n");

    Hardware::I2cDevice i2c{ Pins::I2C::SDA, Pins::I2C::SCL };

    const auto rtc = setupRtc(i2c);
    const auto eeram = setupEeram(i2c);

    printf("Enabling RTC battery backup\r\n");
    if (!RTC_MCP7940N_SetBatteryBackupEnabled(&rtc, true)) {
        printf("Failed to enable RTC battery backup\r\n");
    }

#if RTC_SET_DATETIME
    printf("Testing date-time writing...\r\n");

    if (RTC_MCP7940N_SetDateTime(&rtc, 22, 10, 17, 0, 20, 14, 20, false, false)) {
        printf("Date-time set successfully\r\n");
    } else {
        printf("Failed to set date-time\r\n");
    }
#endif

    printf("Testing date-time reading...\r\n");

    uint8_t year = 0;
    uint8_t month = 0;
    uint8_t date = 0;
    uint8_t wd = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    bool mode12h = false;
    bool pm = false;

    const auto ok = RTC_MCP7940N_GetDateTime(&rtc, &year, &month, &date, &wd, &hour, &minute, &second, &mode12h, &pm);

    printf(
        "Read finished: ok=%u, y=%u, mo=%u, d=%u, wd=%u, h=%u, m=%u, s=%u, 12h=%u, pm=%u\r\n",
        ok, year, month, date, wd, hour, minute, second, mode12h, pm
    );

    const char testData[] = "RTC_MCP7940N_WriteSRAM";

#if RTC_WRITE_TEST_DATA
    printf("Testing SRAM reading/writing...\r\n");

    if (
        RTC_MCP7940N_WriteSRAM(
            &rtc,
            0,
            reinterpret_cast<const uint8_t*>(testData),
            sizeof(testData)
        )
    ) {
        printf("SRAM write succeeded, reading...\r\n");
    } else {
        printf("SRAM write failed\r\n");
    }
#endif

    char buffer[64] = { 0 };

    if (
        RTC_MCP7940N_ReadSRAM(
            &rtc,
            0,
            reinterpret_cast<uint8_t*>(buffer),
            sizeof(testData)
        )
    ) {
        printf("SRAM read succeeded, data: %s\r\n", buffer);
    } else {
        printf("SRAM read failed\r\n");
    }

    printf("Setting up Alarm 0...\r\n");

    if (
        !RTC_MCP7940N_SetAlarm(
            &rtc,
            RTC_MCP7940N_ALARM_0,
            RTC_MCP7940N_ALARM_MASK_SECONDS,
            RTC_MCP7940N_ALARM_POLARITY_HIGH,
            0, 0, 0, 0, 0, 0,
            false, false
        )
    ) {
        printf("Failed to setup Alarm 0\r\n");
    }

    if (!RTC_MCP7940N_SetAlarmEnabled(&rtc, RTC_MCP7940N_ALARM_0, true)) {
        printf("Failed to enable Alarm 0\r\n");
    }

    printf("Setting up Alarm 1...\r\n");

    if (
        !RTC_MCP7940N_SetAlarm(
            &rtc,
            RTC_MCP7940N_ALARM_1,
            RTC_MCP7940N_ALARM_MASK_SECONDS,
            RTC_MCP7940N_ALARM_POLARITY_LOW,
            0, 0, 0, 0, 0, 30,
            false, false
        )
    ) {
        printf("Failed to setup Alarm 0\r\n");
    }

    if (!RTC_MCP7940N_SetAlarmEnabled(&rtc, RTC_MCP7940N_ALARM_1, true)) {
        printf("Failed to enable Alarm 1\r\n");
    }

    static const char* Weekdays[7] = {
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday"
    };

    const char eeramTestData[] = "[proc] Executing command: /home/tomikaa/Downloads/cmake-3.24.2-linux-x86_64/bin/cmake --build /home/tomikaa/github/pico-weather-station/Research/pico-rtc-eeram/build --config Debug --target rtc_eeram_test --";

    printf("Setting EERAM ASE enabled\r\n");
    if (!EERAM_47xxx_SetASE(&eeram, true)) {
        printf("Failed to enable EERAM ASE\r\n");
    }

#if EERAM_WRITE_TEST_DATA
    printf("Writing EERAM test data\r\n");
    if (!EERAM_47xxx_WriteBuffer(&eeram, 10, reinterpret_cast<const uint8_t*>(eeramTestData), sizeof(eeramTestData))) {
        printf("Failed to write EERAM test data\r\n");
    }
#endif

    char eeramBuf[sizeof(eeramTestData) + 1] = { 0 };

    printf("Reading EERAM test data\r\n");
    if (!EERAM_47xxx_ReadBuffer(&eeram, 10, reinterpret_cast<uint8_t*>(eeramBuf), sizeof(eeramTestData))) {
        printf("Failed to read EERAM test data\r\n");
    } else {
        printf("EERAM test data: %s\r\n", eeramBuf);
    }

    while (true) {
        tight_loop_contents();
    }

    while (true) {
        sleep_ms(1000);

        if (RTC_MCP7940N_GetDateTime(&rtc, &year, &month, &date, &wd, &hour, &minute, &second, &mode12h, &pm)) {
            printf(
                "RTC time: %s, 20%02u-%02u-%02u %02u:%02u:%02u\r\n",
                Weekdays[wd % 7], year, month, date, hour, minute, second
            );
        } else {
            printf("Failed to read RTC date-time\r\n");
        }

        if (
            RTC_MCP7940N_GetPowerFailTimeStamp(
                &rtc,
                RTC_MCP7940N_POWER_DOWN_TIMESTAMP,
                &minute, &hour, &date, &wd, &month, &mode12h, &pm
            )
        ) {
            printf(
                "RTC power-down time-stamp: %s, %02u-%02u %02u:%02u%s\r\n",
                Weekdays[wd % 7], month, date, hour, minute,
                mode12h ? (pm ? " PM" : " AM") : ""
            );
        } else {
            printf("Failed to read RTC power-down time-stamp\r\n");
        }

        if (
            RTC_MCP7940N_GetPowerFailTimeStamp(
                &rtc,
                RTC_MCP7940N_POWER_UP_TIMESTAMP,
                &minute, &hour, &date, &wd, &month, &mode12h, &pm
            )
        ) {
            printf(
                "RTC power-up time-stamp: %s, %02u-%02u %02u:%02u%s\r\n",
                Weekdays[wd % 7], month, date, hour, minute,
                mode12h ? (pm ? " PM" : " AM") : ""
            );
        } else {
            printf("Failed to read RTC power-up time-stamp\r\n");
        }

        printf("MFP pin: %u\r\n", gpio_get(Pins::RTC::MFP));

        bool alarm = false;
        if (RTC_MCP7940N_GetAlarmInterruptFlag(&rtc, RTC_MCP7940N_ALARM_0, &alarm)) {
            if (alarm) {
                printf("Alarm 0 triggered\r\n");

                if (!RTC_MCP7940N_ClearAlarmInterruptFlag(&rtc, RTC_MCP7940N_ALARM_0)) {
                    printf("Failed to clear Alarm 0 IF\r\n");
                }
            }
        } else {
            printf("Failed to read Alarm 0 IF\r\n");
        }

        alarm = false;
        if (RTC_MCP7940N_GetAlarmInterruptFlag(&rtc, RTC_MCP7940N_ALARM_1, &alarm)) {
            if (alarm) {
                printf("Alarm 1 triggered\r\n");

                if (!RTC_MCP7940N_ClearAlarmInterruptFlag(&rtc, RTC_MCP7940N_ALARM_1)) {
                    printf("Failed to clear Alarm 1 IF\r\n");
                }
            }
        } else {
            printf("Failed to read Alarm 1 IF\r\n");
        }
    }

    return 0;
}