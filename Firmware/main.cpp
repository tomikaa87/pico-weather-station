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
#include <array>
#include <string>

#include <fmt/core.h>

#include "Drivers/nrf24.h"

#include "Hardware/EnvironmentSensor.h"
#include "Hardware/I2cDevice.h"
#include "Hardware/RadioReceiver.h"
#include "Hardware/RealTimeClock.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/spi.h>
#include <pico/stdio.h>

#include "Drivers/EERAM_47xxx.h"

#define ENABLE_DRAW_TEST 0
#define ENABLE_DIAG_SCREEN 0
#define SET_RTC_DATETIME 0
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
    } raw;
};

namespace
{
    std::unique_ptr<Display> display;
    std::unique_ptr<Screens::WeatherStation> weatherStation;
    std::unique_ptr<Painter> painter;
    std::unique_ptr<Hardware::I2cDevice> i2c;
    std::unique_ptr<Hardware::RealTimeClock> rtc;
    std::unique_ptr<Hardware::EnvironmentSensor> envSens;
    std::unique_ptr<Hardware::RadioReceiver> radioReceiver;
    EERAM_47xxx_Device eeram;
    nrf24_t nrf;
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

    void updateBme280Data()
    {
        _bme280Label.setText(
            fmt::format(
                "BME280: init={}, t={:0.2f}C, p={:0.2f}hPa, h={:0.2f}%",
                envSens->bme280InitResult(),
                envSens->temperatureCelsius(),
                envSens->pressurePa() * 0.01,
                envSens->relativeHumidity()
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
                "Touch: Z1={}, Z2={}, X={}, Y={}",
                data.raw.z1,
                data.raw.z2,
                data.raw.x,
                data.raw.y
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

TouchPanelControllerData readTouchPanelController()
{
    // const auto spiBaudrate = spi_get_baudrate(spi1);
    // spi_set_baudrate(spi1, 100'000);

    auto readAdc = [](const uint8_t ctrl) -> int16_t {

        spi_write_blocking(spi1, &ctrl, 1);

        busy_wait_ms(1);
        
        uint8_t data[2] = { 0 };
        spi_read_blocking(spi1, 0, data, sizeof(data));

        // printf("readAdc: ctrl=%02x, data=%02x %02x\r\n", ctrl, data[0], data[1]);

        return (static_cast<int16_t>(data[0]) << 8 | data[1]) >> 3;
    };

    auto bestTwoAvg = [](const int16_t x, const int16_t y, const int16_t z) {
        const int16_t da = (x > y) ? x - y : y - x;
        const int16_t db = (x > z) ? x - z : z - x;
        const int16_t dc = (z > y) ? z - y : y - z;

        int16_t reta = 0;

        if (da <= db && da <= dc) {
            reta = (x + y) >> 1;
        } else if (db <= da && db <= dc) {
            reta = (x + z) >> 1;
        } else {
            reta = (y + z) >> 1;
        }

        return reta;
    };

    auto normalize = [](const int16_t value, const int16_t min, const int16_t max) -> int16_t {
        if (value <= min) {
            return 0;
        } else if (value >= max) {
            return 0xfff;
        } else {
            return (int16_t)((int) 0xfff * (value - min) / (max - min));
        }
    };

    gpio_put(Pins::Touch::CS, false);

    const int16_t touchPressure[] = {
        readAdc(0xB1),
        readAdc(0xC2)
    };
    
    // Dummy Y measurement to avoid noise
    readAdc(0xD1);

    const int16_t data[] = {
        readAdc(0x91), // Y
        readAdc(0xD1), // X
        readAdc(0x91), // Y
        readAdc(0xD1), // X
        readAdc(0x91), // Y
        readAdc(0xD0), // X + Power-Down
    };

    gpio_put(Pins::Touch::CS, true);

    // spi_set_baudrate(spi1, spiBaudrate);

    const auto zRaw = touchPressure[0] + 0xfff - touchPressure[1];

    // FIXME zRaw doesn't seem to be reliable
    if (touchPressure[0] <= 1000) {
        const auto xRaw = bestTwoAvg(data[1], data[3], data[5]);
        const auto yRaw = bestTwoAvg(data[0], data[2], data[4]);

        static int16_t xMin = 0xfff, xMax = 0, yMin = 0xfff, yMax = 0;

        xMin = std::min(xMin, xRaw);
        xMax = std::max(xMax, xRaw);
        yMin = std::min(yMin, yRaw);
        yMax = std::max(yMax, yRaw);

        const auto x = normalize(xRaw, xMin, xMax);
        const auto y = normalize(yRaw, yMin, yMax);

        const int16_t xDisp = x * 240 / 0xfff;
        const int16_t yDisp = y * 160 / 0xfff;

        printf(
            "Touch: xRaw=%d, yRaw=%d, zRaw=%d, Z1=%d, Z2=%d, xMin=%d, xMax=%d, yMin=%d, yMax=%d, xDisp=%d, yDisp=%d\r\n",
            xRaw, yRaw, zRaw, touchPressure[0], touchPressure[1],
            xMin, xMax, yMin, yMax, xDisp, yDisp
        );

        return TouchPanelControllerData{
            .raw = TouchPanelControllerData::Raw{
                .z1 = touchPressure[0],
                .z2 = touchPressure[1],
                .x = xRaw,
                .y = yRaw
            }
        };
    }

    return {};
}

int main()
{
    stdio_init_all();

    busy_wait_ms(3000);

    printf("Initializing...\r\n");

    display = std::make_unique<Display>();
    display->clear();
    display->setBacklightLevel(10);

    weatherStation = std::make_unique<Screens::WeatherStation>(display.get());

    gpio_set_function(Pins::SPIPeri::TX, GPIO_FUNC_SPI);
    gpio_set_function(Pins::SPIPeri::SCK, GPIO_FUNC_SPI);
    gpio_set_function(Pins::SPIPeri::RX, GPIO_FUNC_SPI);

    spi_init(spi1, 2'000'000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    printf("Initializing I2C\r\n");
    i2c = std::make_unique<Hardware::I2cDevice>(20, 21);
    printf("Initializing EnvironmentSensor\r\n");
    envSens = std::make_unique<Hardware::EnvironmentSensor>(*i2c);
    printf("Initializing RTC\r\n");
    rtc = std::make_unique<Hardware::RealTimeClock>(*i2c);

    printf("Initializing EERAM\r\n");

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

    printf("Initializing touch controller pins\r\n");
    // Touch controller pins
    gpio_init(Pins::Touch::CS);
    gpio_set_dir(Pins::Touch::CS, true);
    gpio_put(Pins::Touch::CS, true);
    gpio_init(Pins::Touch::PENIRQ);
    gpio_set_dir(Pins::Touch::PENIRQ, false);
    gpio_init(Pins::Touch::BUSY);
    gpio_set_dir(Pins::Touch::BUSY, false);
    gpio_set_pulls(Pins::Touch::BUSY, false, true);

    printf("BUSY=%u\r\n", gpio_get(Pins::Touch::BUSY));

    printf("Initializing NRF24 pins\r\n");
    gpio_init(Pins::NRF24::CE);
    gpio_set_dir(Pins::NRF24::CE, true);
    gpio_put(Pins::NRF24::CE, true);
    gpio_init(Pins::NRF24::CSN);
    gpio_set_dir(Pins::NRF24::CSN, true);
    gpio_put(Pins::NRF24::CSN, true);
    gpio_init(Pins::NRF24::IRQ);
    gpio_set_dir(Pins::NRF24::IRQ, false);
    gpio_set_pulls(Pins::NRF24::IRQ, false, true);

    printf("Initializing NRF24\r\n");
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

    // printf("Initializing DiagScreen\r\n");
    // diagScreen = std::make_unique<DiagScreen>();

#if SET_RTC_DATETIME
    if (!
        RTC_MCP7940N_SetDateTime(
            rtc->device(),
            22, 10, 31, 1, 10, 51, 00, false, false
        )
    ) {
        printf("RTC SetDateTime failed\r\n");
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

    printf("Initializing RadioReceiver\r\n");

    radioReceiver = std::make_unique<Hardware::RadioReceiver>();

    printf("Initialization finished\r\n");

    const auto tasks = std::array<std::reference_wrapper<ITask>, 2>{
        *envSens,
        *radioReceiver
    };

    uint32_t weatherScreenUpdateTimestamp = 0;

    weatherStation->setCurrentTemperature(0);
    weatherStation->setCurrentSensorTemperature(0);
    weatherStation->setCurrentMinimumTemperature(0);
    weatherStation->setCurrentMaximumTemperature(0);
    weatherStation->setCurrentPressure(0);
    weatherStation->setCurrentHumidity(0);
    weatherStation->setCurrentWindSpeed(0);
    weatherStation->setCurrentWindGustSpeed(0);

    while (true) {
        for (auto& task : tasks) {
            task.get().run();
        }

        const auto millis = to_ms_since_boot(get_absolute_time());
        static auto updateMillis = 0u;
        if (millis - updateMillis >= 250) {
            updateMillis = millis;

            [[maybe_unused]] const auto touchPanelData = readTouchPanelController();
    
#if ENABLE_DIAG_SCREEN


            printf("Painting diagnostics screen\r\n");
            diagScreen->updateBme280Data();
            diagScreen->updateNrf24Data();
            diagScreen->updateTouchPanelControllerData();
            diagScreen->updateRtcData();
            diagScreen->updateEeramData();

            Painter painter;
            painter.paintWidget(diagScreen.get());
#endif
        }

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

#if 1
        if (millis - weatherScreenUpdateTimestamp >= 5000 || weatherScreenUpdateTimestamp == 0) {
            weatherScreenUpdateTimestamp = millis;

            weatherStation->setCurrentPressure(envSens->pressurePa() * 0.01);
            weatherStation->setInternalSensorHumidity(envSens->relativeHumidity());
            weatherStation->setInternalSensorTemperature(envSens->temperatureCelsius());

            uint8_t y, mo, d, wd, h, m, s;
            bool m12h, pm;

            if (!
                RTC_MCP7940N_GetDateTime(
                    rtc->device(),
                    &y, &mo, &d, &wd, &h, &m, &s, &m12h, &pm
                )
            ) {
                printf("RTC GetDateTime failed\r\n");
            }

            printf(
                "RTC: %u 20%02u-%02u-%02u %02u:%02u:%02u\r\n",
                wd, y, mo, d, h, m, s
            );

            static constexpr const char* Weekdays[7] = {
                "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
            };

            weatherStation->setClockTime(h, m);
            weatherStation->setClockDate(d, Weekdays[wd - 1]);

            painter->paintWidget(weatherStation.get());
        }
#endif
    }
}