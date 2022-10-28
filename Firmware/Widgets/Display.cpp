#include "Display.h"

#include "../Fonts.h"

#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/spi.h>

#include <U8g2lib.h>

#include <memory>

extern "C" {
extern uint8_t u8x8_d_st7586s_erc240160_chunked(
    u8x8_t *u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void *arg_ptr
);
}

extern "C" uint8_t u8x8_byte_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    uint8_t *data;
    uint8_t internal_spi_mode;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:

        // 1.6.5 offers a block transfer, but the problem is, that the
        // buffer is overwritten with the incoming data
        // so it can not be used...
        // SPI.transfer((uint8_t *)arg_ptr, arg_int);

        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            spi0Remapped->transfer((uint8_t)*data);
            data++;
            arg_int--;
        }

        break;
    case U8X8_MSG_BYTE_INIT:
        if (u8x8->bus_clock == 0) /* issue 769 */
            u8x8->bus_clock = u8x8->display_info->sck_clock_hz;
        /* disable chipselect */
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);

        /* no wait required here */

        /* for SPI: setup correct level of the clock signal */
        // removed, use SPI.begin() instead: pinMode(11, OUTPUT);
        // removed, use SPI.begin() instead: pinMode(13, OUTPUT);
        // removed, use SPI.begin() instead: digitalWrite(13, u8x8_GetSPIClockPhase(u8x8));

        /* setup hardware with SPI.begin() instead of previous digitalWrite() and pinMode() calls */
        spi0Remapped->begin();
        break;

    case U8X8_MSG_BYTE_SET_DC:
        u8x8_gpio_SetDC(u8x8, arg_int);
        break;

    case U8X8_MSG_BYTE_START_TRANSFER: {
        const auto cpol = [&] {
            switch (u8x8->display_info->spi_mode)
            {
                case 0:
                case 1:
                default:
                    return SPI_CPOL_0;
                case 2:
                case 3:
                    return SPI_CPOL_1;
            }
        }();

        const auto cpha = [&] {
            switch (u8x8->display_info->spi_mode)
            {
                case 0:
                case 2:
                default:
                    return SPI_CPHA_0;
                case 1:
                case 3:
                    return SPI_CPHA_1;
            }
        }();
    
        spi_set_format(spi0, 8, cpol, cpha, SPI_MSB_FIRST);
        spi_init(spi0, 60 * 1000 * 1000);

        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
        u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
        break;
    }

    case U8X8_MSG_BYTE_END_TRANSFER:
        u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
        u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);

        spi0Remapped->endTransaction();

        break;
    default:
        return 0;
    }

    return 1;
}

namespace Pins
{
    constexpr auto CS = 5;
    constexpr auto DC = 8;
    constexpr auto RST = 9;

    constexpr auto BacklightPwm = 0;

    namespace SPI0
    {
        constexpr auto RX = 4;
        constexpr auto CS = 5;
        constexpr auto SCK = 6;
        constexpr auto TX = 7;
    }
}

struct Display::Private
{
    u8g2_t u8g2;
};

Display::Display()
    : _p{ std::make_unique<Private>() }
{
    setup();
    setFont(Font{});
}

Display::~Display() = default;

Size Display::size() const
{
    return {
        u8g2_GetDisplayWidth(&_p->u8g2),
        u8g2_GetDisplayHeight(&_p->u8g2)
    };
}

void Display::clear()
{
    u8g2_ClearDisplay(&_p->u8g2);
}

void Display::clearBuffer()
{
    u8g2_ClearBuffer(&_p->u8g2);
}

void Display::update()
{
    u8g2_SendBuffer(&_p->u8g2);
}

void Display::setContrast(const uint8_t value)
{
    u8g2_SetContrast(&_p->u8g2, value);
}

void Display::setBacklightLevel(const uint8_t value)
{
    pwm_set_gpio_level(Pins::BacklightPwm, value);
}

void Display::setDrawColor(const Color color)
{
    u8g2_SetDrawColor(&_p->u8g2, static_cast<int>(color));
}

void Display::setFont(const Font& font)
{
    const auto* const data = font.data();
    if (data != nullptr) {
        u8g2_SetFont(&_p->u8g2, data);
    }
}

void Display::setClipRect(const Rect& rect)
{
    u8g2_SetClipWindow(
        &_p->u8g2,
        rect.x(),
        rect.y(),
        rect.x() + rect.width(),
        rect.y() + rect.height()
    );
}

void Display::resetClipRect()
{
    u8g2_SetMaxClipWindow(&_p->u8g2);
}

int Display::calculateFontAscent() const
{
    return u8g2_GetAscent(&_p->u8g2);
}

int Display::calculateFontDescent() const
{
    return u8g2_GetDescent(&_p->u8g2);
}

int Display::calculateMaxCharHeight() const
{
    return u8g2_GetMaxCharHeight(&_p->u8g2);
}

int Display::calculateTextWidth(const std::string& text) const
{
    return u8g2_GetStrWidth(&_p->u8g2, text.c_str());
}

void Display::drawText(const Point& pos, const std::string& s)
{
    u8g2_DrawStr(&_p->u8g2, pos.x(), pos.y(), s.c_str());
}

void Display::drawBitmap(
    const Point& pos,
    const int width,
    const int height,
    const uint8_t* const data)
{
    u8g2_DrawXBM(&_p->u8g2, pos.x(), pos.y(), width, height, data);
}

void Display::drawRect(const Rect& rect)
{
    u8g2_DrawFrame(&_p->u8g2, rect.x(), rect.y(), rect.width(), rect.height());
}

void Display::drawLine(const Point& from, const Point& to)
{
    u8g2_DrawLine(&_p->u8g2, from.x(), from.y(), to.x(), to.y());
}

void Display::drawLine(
    const uint8_t x1,
    const uint8_t y1,
    const uint8_t x2,
    const uint8_t y2
) {
    u8g2_DrawLine(&_p->u8g2, x1, y1, x2, y2);
}

void Display::fillRect(const Rect& rect)
{
    u8g2_DrawBox(&_p->u8g2, rect.x(), rect.y(), rect.width(), rect.height());
}

void Display::setup()
{
    printf("%s\r\n", __FUNCTION__);

    uint8_t tileBufHeight;
    uint8_t* buf;

    u8g2_SetupDisplay(
        &_p->u8g2,
        u8x8_d_st7586s_erc240160_chunked,
        u8x8_cad_011,
        u8x8_byte_hw_spi,
        u8x8_gpio_and_delay_arduino
    );

    printf("u8g2_SetupDisplay OK\r\n");

    buf = u8g2_m_30_20_f(&tileBufHeight);

    u8g2_SetupBuffer(&_p->u8g2, buf, tileBufHeight, u8g2_ll_hvline_horizontal_right_lsb, U8G2_R0);

    printf("u8g2_SetupBuffer OK\r\n");

    // Setup the SPI bus
    spi_init(spi_default, 60 * 1000 * 1000);
    gpio_set_function(Pins::SPI0::RX, GPIO_FUNC_SPI);
    gpio_set_function(Pins::SPI0::SCK, GPIO_FUNC_SPI);
    gpio_set_function(Pins::SPI0::TX, GPIO_FUNC_SPI);
    gpio_set_function(Pins::CS, GPIO_FUNC_SIO);
    gpio_set_function(Pins::DC, GPIO_FUNC_SIO);
    gpio_set_function(Pins::RST, GPIO_FUNC_SIO);
    gpio_set_dir(Pins::CS, GPIO_OUT);
    gpio_set_dir(Pins::DC, GPIO_OUT);
    gpio_set_dir(Pins::RST, GPIO_OUT);
    u8x8_SetPin(u8g2_GetU8x8(&_p->u8g2), U8X8_PIN_CS, Pins::CS);
    u8x8_SetPin(u8g2_GetU8x8(&_p->u8g2), U8X8_PIN_DC, Pins::DC);
    u8x8_SetPin(u8g2_GetU8x8(&_p->u8g2), U8X8_PIN_RESET, Pins::RST);

    // Setup the backlight control PWM pin
    gpio_set_function(Pins::BacklightPwm, GPIO_FUNC_PWM);
    const auto blPwmPinSlice = pwm_gpio_to_slice_num(Pins::BacklightPwm);
    auto blPwmConfig = pwm_get_default_config();
    pwm_config_set_clkdiv(&blPwmConfig, 4.f);
    pwm_config_set_wrap(&blPwmConfig, 255);
    pwm_init(blPwmPinSlice, &blPwmConfig, true);
    setBacklightLevel(60);

    u8g2_InitDisplay(&_p->u8g2);
    u8g2_SetPowerSave(&_p->u8g2, 0);
    u8g2_SetContrast(&_p->u8g2, 60);

    printf("%s OK\r\n", __FUNCTION__);
}
