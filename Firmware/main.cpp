#include "Fonts.h"
#include "Graphics.h"
#include "Icons.h"

#include "Screens/WeatherStation.h"

#include "Widgets/Display.h"
#include "Widgets/Image.h"
#include "Widgets/Label.h"
#include "Widgets/Painter.h"
#include "Widgets/ProgressBar.h"
#include "Widgets/Widget.h"

#include <algorithm>
#include <string>

#include <fmt/core.h>

#include "Drivers/nrf24.h"
#include "Drivers/bme280.h"
#include "Hardware/I2cDevice.h"
#include "Hardware/RealTimeClock.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <pico/stdio.h>

#include "Drivers/EERAM_47xxx.h"

#define ENABLE_DRAW_TEST 0
#define ENABLE_DIAG_SCREEN 1

#define TEST_EERAM_PROGRAM_DATA 0

class DiagScreen;

struct TouchPanelControllerData
{
    struct Raw
    {
        int16_t z1 = 0;
        int16_t z2 = 0;
        int16_t x = 0;
        int16_t y = 0;
        int16_t temp = 0;
    } raw;
};

namespace
{
    std::unique_ptr<Display> display;
    std::unique_ptr<Screens::WeatherStation> weatherStation;
    std::unique_ptr<Painter> painter;
    // Screens::WeatherStation weatherStation{ &display };
    // Painter painter;

    // int temperature = -30;
    // int pressure = 1000;
    // int humidity = 50;
    // int windSpeed = 50;

    // int32_t lastUpdateMillis = 0;
    // auto updateLedOn = true;

    // std::unique_ptr<SPIClassRP2040> spiPeri;
    std::unique_ptr<Hardware::I2cDevice> i2c;
    std::unique_ptr<Hardware::RealTimeClock> rtc;
    EERAM_47xxx_Device eeram;

    nrf24_t nrf;
    struct bme280_dev bme;
    int bmeInitResult = 0;

    std::unique_ptr<DiagScreen> diagScreen;
}

class DiagScreen : public Widget
{
public:
    DiagScreen()
        : Widget{ display.get() }
        , _titleLabel{ "Pico Weather Station - Diagnostics", this }
        , _bme280Label{ this }
        , _nrf24Label{ this }
        , _nrf24FifoLabel{ this }
        , _touchPanelLabel{ this }
        , _rtcDateTimeLabel{ this }
        , _rtcPowerDownTimeStampLabel{ this }
        , _rtcPowerUpTimeStampLabel{ this }
        , _eeramStatusLabel{ this }
    {
        setRect(Rect{ 0, 0, 240, 160 });

        auto nextLineY = 0;
        auto getNextLineRect = [&nextLineY] {
            const auto y = nextLineY;
            nextLineY += 10;
            return Rect{ 0, y, 240, 10 };
        };

        _titleLabel.setRect(getNextLineRect());
        _titleLabel.setFont(Font{ Font::Family::BitCell });

        nextLineY = 20;

        _bme280Label.setRect(getNextLineRect());
        setupLabel(_bme280Label);

        _nrf24Label.setRect(getNextLineRect());
        _nrf24FifoLabel.setRect(getNextLineRect());
        setupLabel(_nrf24Label);
        setupLabel(_nrf24FifoLabel);

        _touchPanelLabel.setRect(getNextLineRect());
        setupLabel(_touchPanelLabel);

        _rtcDateTimeLabel.setRect(getNextLineRect());
        _rtcPowerDownTimeStampLabel.setRect(getNextLineRect());
        _rtcPowerUpTimeStampLabel.setRect(getNextLineRect());
        setupLabel(_rtcDateTimeLabel);
        setupLabel(_rtcPowerDownTimeStampLabel);
        setupLabel(_rtcPowerUpTimeStampLabel);

        _eeramStatusLabel.setRect(getNextLineRect());
        setupLabel(_eeramStatusLabel);
    }

    void updateBme280Data(const struct bme280_data& data)
    {
        _bme280Label.setText(
            fmt::format(
                "BME280: init={}, t={:0.2f}C, p={:0.2f}hPa, h={:0.2f}%",
                bmeInitResult,
                data.temperature,
                data.pressure * 0.01,
                data.humidity
            )
        );
    }

    void updateNrf24Data()
    {
        const auto status = nrf24_get_status(&nrf);
        const auto fifoStatus = nrf24_get_fifo_status(&nrf);

        _nrf24Label.setText(
            fmt::format(
                "NRF24: TX_FULL={},RX_P_NO={},MAX_RT={},TX_DS={},RX_DR={}",
                status.TX_FULL,
                status.RX_P_NO,
                status.MAX_RT,
                status.TX_DS,
                status.RX_DR
            )
        );

        _nrf24FifoLabel.setText(
            fmt::format(
                "NRF24: RX_E={},RX_F={},TX_E={},TX_F={},TX_REUSE={}",
                fifoStatus.RX_EMPTY,
                fifoStatus.RX_FULL,
                fifoStatus.TX_EMPTY,
                fifoStatus.TX_FULL,
                fifoStatus.TX_REUSE
            )
        );
    }

    void updateTouchPanelControllerData(const TouchPanelControllerData& data)
    {
        _touchPanelLabel.setText(
            fmt::format(
                "Touch: Z1={}, Z2={}, X={}, Y={}, Tmp={}",
                data.raw.z1,
                data.raw.z2,
                data.raw.x,
                data.raw.y,
                data.raw.temp
            )
        );
    }

    void updateRtcData()
    {
        uint8_t y, mo, d, wd, h, m, s;
        bool m12h, pm;

        if (
            !RTC_MCP7940N_GetDateTime(
                rtc->device(),
                &y, &mo, &d, &wd, &h, &m, &s, &m12h, &pm
            )
        ) {
            _rtcDateTimeLabel.setText("RTC: GetDateTime failed");
            return;
        }

        printf("updateRtcData: GetDateTime succeeded\r\n");

        static const char* Weekdays[] = {
            "Monday",
            "Tuesday",
            "Wednesday",
            "Thursday",
            "Friday",
            "Saturday",
            "Sunday"
        };

        _rtcDateTimeLabel.setText(
            // In this environment, automatic IDs doesn't work in fmtlib
            fmt::format(
                "RTC: 20{0:02}-{1:02}-{2:02} {3:02}:{4:02}:{5:02} {6}",
                y, mo, d, h, m, s, Weekdays[wd]
            )
        );

        if (
            !RTC_MCP7940N_GetPowerFailTimeStamp(
                rtc->device(),
                RTC_MCP7940N_POWER_DOWN_TIMESTAMP,
                &m, &h, &d, &wd, &mo, &m12h, &pm
            )
        ) {
            _rtcPowerDownTimeStampLabel.setText("RTC: Pwr-Dn GetPowerFailTimeStamp failed");
            return;
        }

        _rtcPowerDownTimeStampLabel.setText(
            // In this environment, automatic IDs doesn't work in fmtlib
            fmt::format(
                "RTC: Pwr-Dn {0:02}-{1:02} {2:02}:{3:02} {4}",
                mo, d, h, m, Weekdays[wd]
            )
        );

        if (
            !RTC_MCP7940N_GetPowerFailTimeStamp(
                rtc->device(),
                RTC_MCP7940N_POWER_UP_TIMESTAMP,
                &m, &h, &d, &wd, &mo, &m12h, &pm
            )
        ) {
            _rtcPowerUpTimeStampLabel.setText("RTC: Pwr-Up GetPowerFailTimeStamp failed");
            return;
        }

        // auto powerFailed = false;
        // if (
        //     !RTC_MCP7940N_Get(
        //         rtc->device(),
        //         RTC_MCP7940N_POWER_UP_TIMESTAMP,
        //         &m, &h, &d, &wd, &mo, &m12h, &pm
        //     )
        // ) {
        //     _rtcPowerUpTimeStampLabel.setText("RTC: Pwr-Up GetPowerFailTimeStamp failed");
        //     return;
        // }

        _rtcPowerUpTimeStampLabel.setText(
            // In this environment, automatic IDs doesn't work in fmtlib
            fmt::format(
                "RTC: Pwr-Up {0:02}-{1:02} {2:02}:{3:02} {4}",
                mo, d, h, m, Weekdays[wd]
            )
        );
    }

    void updateEeramData()
    {
        auto ase = false;
        const auto aseRead = EERAM_47xxx_GetASE(
            &eeram,
            &ase
        );

        char data[5] = { 0 };
        const auto dataRead = EERAM_47xxx_ReadBuffer(
            &eeram,
            0,
            reinterpret_cast<uint8_t*>(data),
            4
        );

        _eeramStatusLabel.setText(
            fmt::format(
                "EE: ASE=(r={},v={}), data=(r={},v={})",
                aseRead, ase, dataRead, data
            )
        );
    }

private:
    Label _titleLabel;
    Label _bme280Label;
    Label _nrf24Label;
    Label _nrf24FifoLabel;
    Label _touchPanelLabel;
    Label _rtcDateTimeLabel;
    Label _rtcPowerDownTimeStampLabel;
    Label _rtcPowerUpTimeStampLabel;
    Label _eeramStatusLabel;

    void setupLabel(Label& label)
    {
        label.setFont(
            Font{
                Font::Family::PfTempesta7,
                Font::Style::Condensed
            }
        );
    }
};

namespace Pins
{
    namespace BME280
    {
        constexpr auto CSB = 4;
    }

    namespace NRF24
    {
        constexpr auto IRQ = 16;
        constexpr auto CE = 17;
        constexpr auto CSN = 18;
    }

    namespace Touch
    {
        constexpr auto PENIRQ = 13;
        constexpr auto CS = 14;
        constexpr auto BUSY = 15;
    }

    namespace SPIPeri
    {
        constexpr auto SCK = 10;
        constexpr auto TX = 11;
        constexpr auto RX = 12;
        constexpr auto CS = 13;
    }
}

#if 0
void drawFancyProgressBar(
    u8g2_t* display,
    const int x,
    const int y,
    const int w,
    const int h,
    const int progress,
    const int min = 0,
    const int max = 100
) {
    auto font = u8g2_font_p01type_tn;
    auto startY = y;

    u8g2_ClearBuffer(display);

    // Numbers and Ticks
    u8g2_SetDrawColor(display, 1);
    u8g2_SetFont(display, font);
    const auto ticks = 11;
    const auto startPos = x + 2;
    const auto endPos = x + w - 4;
    const auto textStartY = startY;
    startY += 7;
    const auto ticksStartY = startY;
    for (auto i = 0; i < ticks; ++i) {
        const auto percent = (max - min) / (ticks - 1) * i;
        const auto pos = startPos + (endPos - startPos) * percent / 100;
        const auto height = [](const int pct) {
            if (pct == 0 || pct == 100) {
                return 3;
            }
            if (pct == 50) {
                return 2;
            }
            return 1;
        }(percent);
        const auto maxHeight = 3;
        u8g2_DrawLine(display, pos, ticksStartY + maxHeight - height, pos, ticksStartY + maxHeight);
        if (percent == 0) {
            u8g2_DrawStr(display, pos, ticksStartY - 2, std::to_string(min).c_str());
        } else if (percent == 50) {
            const auto s = std::to_string(min + (max - min) / 2);
            u8g2_DrawStr(display, pos - u8g2_GetStrWidth(display, s.c_str()) / 2, ticksStartY - 2, s.c_str());
        } else if (percent == 100) {
            const auto s = std::to_string(max);
            u8g2_DrawStr(display, pos - u8g2_GetStrWidth(display, s.c_str()) + 1, ticksStartY - 2, s.c_str());
        }
    }
    startY += 5;

    // Frame
    u8g2_SetDrawColor(display, 1);
    u8g2_DrawRBox(display, x, startY, w - 1, h, 1);

    // Bar background
    u8g2_SetDrawColor(display, 0);
    u8g2_DrawRBox(display, x + 1, startY + 1, w - 3, h - 2, 1);

    // Bar
    u8g2_SetDrawColor(display, 1);
    const auto barStartX = x + 2;
    const auto barEndX = x + w - 3;
    const auto barStartY = startY + 2;
    const auto barEndY = startY + h - 2;
    const auto barPercent = std::max(min, std::min(max, progress)) * 100 / (max - min);
    const auto barLength = (barEndX - barStartX) * barPercent / 100;
    const auto barHeight = barEndY - barStartY;
    for (auto i = 0; i < barLength; ++i) {
        for (auto j = i % 2 == 0 ? 0 : 1; j < barHeight; j += 2) {
            u8g2_DrawPixel(display, barStartX + i, barStartY + j);
        }
    }

    // u8g2_SendBuffer(display);
}
#endif

/*!
 * @brief This API used to print the sensor temperature, pressure and humidity data.
 */
void print_sensor_data(struct bme280_data *comp_data)
{
    float temp, press, hum;

#ifdef BME280_FLOAT_ENABLE
    temp = comp_data->temperature;
    press = 0.01 * comp_data->pressure;
    hum = comp_data->humidity;
#else
#ifdef BME280_64BIT_ENABLE
    temp = 0.01f * comp_data->temperature;
    press = 0.0001f * comp_data->pressure;
    hum = 1.0f / 1024.0f * comp_data->humidity;
#else
    temp = 0.01f * comp_data->temperature;
    press = 0.01f * comp_data->pressure;
    hum = 1.0f / 1024.0f * comp_data->humidity;
#endif
#endif
    printf("%0.2lf deg C, %0.2lf hPa, %0.2lf%%\r\n", temp, press, hum);
}

/*!
 * @brief This API reads the sensor temperature, pressure and humidity data in forced mode.
 */
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev)
{
    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    /* Variable to define the selecting sensors */
    uint8_t settings_sel = 0;

    /* Variable to store minimum wait time between consecutive measurement in force mode */
    uint32_t req_delay;

    /* Structure to get the pressure, temperature and humidity values */
    struct bme280_data comp_data;

    /* Recommended mode of operation: Weather monitoring */
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_1X;
    dev->settings.osr_t = BME280_OVERSAMPLING_1X;
    dev->settings.filter = BME280_FILTER_COEFF_OFF;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    /* Set the sensor settings */
    rslt = bme280_set_sensor_settings(settings_sel, dev);
    if (rslt != BME280_OK)
    {
        printf("Failed to set sensor settings (code %+d).\r\n", rslt);

        return rslt;
    }

    // printf("Temperature, Pressure, Humidity\r\n");

    /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
     *  and the oversampling configuration. */
    req_delay = bme280_cal_meas_delay(&dev->settings);

    printf("req_delay: %u\r\n", req_delay);

    /* Continuously stream sensor data */
    do
    {
        /* Set the sensor to forced mode */
        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
        if (rslt != BME280_OK)
        {
            printf("Failed to set sensor mode (code %+d).\r\n", rslt);
            break;
        }

        /* Wait for the measurement to complete and print data */
        sleep_ms(req_delay);
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
        if (rslt != BME280_OK)
        {
            printf("Failed to get sensor data (code %+d).\r\n", rslt);
            break;
        }

        print_sensor_data(&comp_data);

        diagScreen->updateBme280Data(comp_data);
    } while (false);

    return rslt;
}

TouchPanelControllerData readTouchPanelController()
{
    const auto spiBaudrate = spi_get_baudrate(spi1);
    spi_set_baudrate(spi1, 500'000);

    auto readAdc = [](const uint8_t ctrl) -> int16_t {
        gpio_put(Pins::Touch::CS, false);

        spi_write_blocking(spi1, &ctrl, 1);

        // FIXME figure out the BUSY signal
        // busy_wait_us(10);

        // auto count = 0u;
        // while (gpio_get(Pins::Touch::BUSY) && count < 10000u) {
        //     busy_wait_us(1);
        //     ++count;
        // }

        // if (count >= 500) {
        //     printf("%s: busy wait timed out\r\n", __FUNCTION__);
        // }
        
        uint8_t data[2] = { 0 };
        spi_read_blocking(spi1, 0, data, 2);

        gpio_put(Pins::Touch::CS, true);

        return (static_cast<int16_t>(data[0]) << 8 | data[1]) >> 3;
        // return ((data[0] << 8) | data[1]) >> 3;
    };

    // A2 A1 A0
    //  0  0  1 -> XP+IN Y-position
    //  0  1  1 -> XP+IN Z1-position
    //  1  0  0 -> YN+IN Z2-position
    //  1  0  1 -> YP+IN X-position

    // Control byte:
    //      S A2 A1 A0 MODE SFR/nDFR PD1 PD0
    // B1 = 1  0  1  1    0        0   0   1

    // Dummy read to turn off the internal reference
    readAdc(0b10110001);

    TouchPanelControllerData data{
        .raw = TouchPanelControllerData::Raw{
            .z1 =   readAdc(0b10110001),
            .z2 =   readAdc(0b11000001),
            .x =    readAdc(0b11010001),
            .y =    readAdc(0b10010001),
            .temp = readAdc(0b10000111)
        }
    };

    spi_set_baudrate(spi1, spiBaudrate);

    return data;
}

int main()
{
    stdio_init_all();

    // busy_wait_ms(3000);

    printf("Initializing...\r\n");

    display = std::make_unique<Display>();
    display->clear();
    display->setBacklightLevel(20);

#if 0
    weatherStation = std::make_unique<Screens::WeatherStation>(display.get());
    painter = std::make_unique<Painter>();

    display->clear();

    // weatherStation.setCurrentTemperature(-88);
    weatherStation->setCurrentSensorTemperature(-88);
    // weatherStation.setCurrentPressure(8888);
    weatherStation->setCurrentHumidity(888);
    weatherStation->setCurrentWindSpeed(888);
    weatherStation->setCurrentWindGustSpeed(818);
    weatherStation->setInternalSensorHumidity(88.8);
    weatherStation->setInternalSensorTemperature(88.8);
    weatherStation->setClockTime(12, 34);
    weatherStation->setClockDate(23, "Wed");
#endif

    gpio_set_function(Pins::SPIPeri::TX, GPIO_FUNC_SPI);
    gpio_set_function(Pins::SPIPeri::SCK, GPIO_FUNC_SPI);
    gpio_set_function(Pins::SPIPeri::RX, GPIO_FUNC_SPI);

    spi_init(spi1, 4'000'000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // i2c->begin();
    i2c = std::make_unique<Hardware::I2cDevice>(20, 21);
    rtc = std::make_unique<Hardware::RealTimeClock>(*i2c);

    eeram.a1 = 0;
    eeram.a2 = 0;

    eeram.i2cFunctionArg = i2c.get();

    eeram.i2cStartTransmission = [](
        void* const arg,
        const uint8_t address
    ) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->startTransmission(address);
    };

    eeram.i2cEndTransmission = [](void* const arg) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->endTransmission();
    };

    eeram.i2cRead = [](
        void* const arg,
        uint8_t* const data,
        const size_t length,
        const bool noStop
    ) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->read(data, length, noStop);
    };

    eeram.i2cWrite = [](
        void* const arg,
        const uint8_t* const data,
        const size_t length,
        const bool noStop
    ) {
        auto* const i2c = reinterpret_cast<Hardware::I2cDevice*>(arg);
        return i2c->write(data, length, noStop);
    };

#if TEST_EERAM_PROGRAM_DATA
    printf("Programming EERAM test data\r\n");
    if (
        !EERAM_47xxx_WriteBuffer(
            &eeram,
            0,
            reinterpret_cast<const uint8_t*>("Test"),
            4
        )
    ) {
        printf("Failed to program EERAM test data\r\n");
    }

    printf("Setting EERAM ASE flag\r\n");
    if (!EERAM_47xxx_SetASE(&eeram, true)) {
        printf("Failed to set EERAM ASE flag\r\n");
    }
#endif

    // Touch controller pins
    gpio_init(Pins::Touch::CS);
    gpio_set_dir(Pins::Touch::CS, true);
    gpio_put(Pins::Touch::CS, true);
    gpio_init(Pins::Touch::PENIRQ);
    gpio_set_dir(Pins::Touch::PENIRQ, false);
    gpio_init(Pins::Touch::BUSY);
    gpio_set_dir(Pins::Touch::BUSY, false);
    gpio_set_pulls(Pins::Touch::BUSY, false, true);

    gpio_init(Pins::NRF24::CE);
    gpio_set_dir(Pins::NRF24::CE, true);
    gpio_put(Pins::NRF24::CE, true);
    gpio_init(Pins::NRF24::CSN);
    gpio_set_dir(Pins::NRF24::CSN, true);
    gpio_put(Pins::NRF24::CSN, true);
    gpio_init(Pins::NRF24::IRQ);
    gpio_set_dir(Pins::NRF24::IRQ, false);
    gpio_set_pulls(Pins::NRF24::IRQ, false, true);

    nrf24_init(
        &nrf,
        // set_ce
        [] (const nrf24_state_t state) {
            gpio_put(Pins::NRF24::CE, state == NRF24_HIGH ? true : false);
        },
        // set_csn
        [] (const nrf24_state_t state) {
            gpio_put(Pins::NRF24::CSN, state == NRF24_HIGH ? true : false);
        },
        // spi_exchange
        [] (const uint8_t data) {
            uint8_t received = 0;
            spi_write_read_blocking(spi1, &data, &received, 1);
            return received;
        }
    );

    nrf24_rf_setup_t rf_setup;
    memset(&rf_setup, 0, sizeof (nrf24_rf_setup_t));
    rf_setup.P_RF_DR_HIGH = 0;
    rf_setup.P_RF_DR_LOW = 1;
    rf_setup.P_RF_PWR = NRF24_RF_OUT_PWR_0DBM;
    nrf24_set_rf_setup(&nrf, rf_setup);

    nrf24_set_rf_channel(&nrf, 12);

    nrf24_setup_retr_t setup_retr;
    setup_retr.ARC = 15;
    setup_retr.ARD = 15;
    nrf24_set_setup_retr(&nrf, setup_retr);

    nrf24_set_rx_payload_len(&nrf, 0, NRF24_DEFAULT_PAYLOAD_LEN);

    nrf24_clear_interrupts(&nrf);

    bme.intf = BME280_I2C_INTF;
    bme.read = [](
        const uint8_t reg_addr,
        uint8_t* const reg_data,
        const uint32_t len,
        [[maybe_unused]] void* const intf_ptr
    ) -> BME280_INTF_RET_TYPE {
        if (!i2c->startTransmission(BME280_I2C_ADDR_PRIM)) {
            printf("bme.read: start failed\r\n");
        }
        if (!i2c->write(&reg_addr, 1)) {
            printf("bme.read: write failed\r\n");
        }
        if (!i2c->startTransmission(BME280_I2C_ADDR_PRIM)) {
            printf("bme.read: restart failed\r\n");
        }
        if (!i2c->read(reg_data, len)) {
            printf("bme.read: read failed\r\n");
        }
        if (!i2c->endTransmission()) {
            printf("bme.read: end failed\r\n");
        }
        printf("bme.read: a=%02x, l=%u, d=", reg_addr, len);
        for (auto i = 0u; i < len; ++i) {
            printf("%02x", reg_data[i]);
            if (i < len - 1) {
                printf(",");
            }
        }
        printf("\r\n");
        return BME280_INTF_RET_SUCCESS;
    };
    bme.write = [](
        const uint8_t reg_addr,
        const uint8_t* const reg_data,
        const uint32_t len,
        [[maybe_unused]] void* const intf_ptr
    ) -> BME280_INTF_RET_TYPE {
        printf("bme.write: a=%x, l=%u, d=", reg_addr, len);
        for (auto i = 0u; i < len; ++i) {
            printf("%02x", reg_data[i]);
            if (i < len - 1) {
                printf(",");
            }
        }
        printf("\r\n");
        if (!i2c->startTransmission(BME280_I2C_ADDR_PRIM)) {
            printf("bme.write: start failed\r\n");
        }
        if (!i2c->write(&reg_addr, 1, true)) {
            printf("bme.write: write reg addr failed\r\n");
        }
        if (!i2c->write(reg_data, len)) {
            printf("bme.write: write data failed\r\n");
        }
        if (!i2c->endTransmission()) {
            printf("bme.read: end failed\r\n");
        }
        return BME280_INTF_RET_SUCCESS;
    };
    bme.delay_us = [](
        const uint32_t period,
        [[maybe_unused]] void* const intf_ptr
    ) {
        sleep_us(period);
    };
    bmeInitResult = bme280_init(&bme);

    diagScreen = std::make_unique<DiagScreen>();

#if SET_RTC_DATETIME
    if (!
        RTC_MCP7940N_SetDateTime(
            rtc->device(),
            22, 10, 20, 3, 22, 46, 00, false, false
        )
    ) {
        printf(PSTR("RTC SetDateTime failed\r\n"));
    }
#endif

    if (
        !RTC_MCP7940N_SetExternalOscillatorEnabled(
            rtc->device(),
            false
        )
    ) {
        printf("RTC SetExternalOscillatorDisabled failed\r\n");
    }

    if (
        !RTC_MCP7940N_SetBatteryBackupEnabled(
            rtc->device(),
            true
        )
    ) {
        printf("RTC SetBatteryBackupEnabled failed\r\n");
    }

    printf("Initialization finished\r\n");

    while (true) {
#if ENABLE_DIAG_SCREEN
        static auto updateMillis = 0u;
        static auto bmeUpdateMillis = 0u;

        const auto millis = to_ms_since_boot(get_absolute_time());

        if (millis - bmeUpdateMillis >= 5000) {
            bmeUpdateMillis = millis;
            stream_sensor_data_forced_mode(&bme);
        }

        if (millis - updateMillis >= 250) {
            updateMillis = millis;

            printf("Painting diagnostics screen\r\n");
            diagScreen->updateNrf24Data();
            diagScreen->updateTouchPanelControllerData(readTouchPanelController());
            diagScreen->updateRtcData();
            diagScreen->updateEeramData();

            Painter painter;
            painter.paintWidget(diagScreen.get());
        }
#endif

#if ENABLE_DRAW_TEST
        printf("BME280 init result: %d\r\n", bmeInitResult);
        printf("Running display test\r\n");

        display->clear();

        for (auto i = 0; i < 240; i += 2) {
            display->drawLine(0, 0, i, 159);
            display->update();
        }

        for (auto i = 158; i > 0; i -= 2) {
            display->drawLine(0, 0, 239, i);
            display->update();
        }

        nrf24_dump_registers(&nrf);
        // stream_sensor_data_forced_mode(&bme);
#endif

#if 0
        if (millis() - lastUpdateMillis >= 500) {
            lastUpdateMillis = millis();

            digitalWrite(LED_BUILTIN, updateLedOn ? HIGH : LOW);
            updateLedOn = !updateLedOn;

            printf(fmt::format("{}: update\r\n", __func__).c_str());

            weatherStation->setCurrentTemperature(temperature);
            weatherStation->setCurrentSensorTemperature(temperature);
            weatherStation->setCurrentMinimumTemperature(temperature);
            weatherStation->setCurrentMaximumTemperature(temperature);
            weatherStation->setCurrentPressure(pressure);
            weatherStation->setCurrentHumidity(humidity);
            weatherStation->setCurrentWindSpeed(windSpeed);
            weatherStation->setCurrentWindGustSpeed(windSpeed);
            weatherStation->setInternalSensorHumidity(humidity);
            weatherStation->setInternalSensorTemperature(temperature);

            painter->paintWidget(weatherStation.get());

            if (++temperature > 30) {
                temperature = -30;
            }

            if (++pressure > 1030) {
                pressure = 980;
            }

            if (++humidity > 100) {
                humidity = 0;
            }

            if (++windSpeed > 120) {
                windSpeed = 0;
            }
        }
#endif
    }
}