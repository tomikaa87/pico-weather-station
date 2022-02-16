#include "Display.h"

#include "../Fonts.h"

#include <u8g2.h>

#include <iostream>

#ifdef RASPBERRY_PI
#include <wiringPi.h>
#include <wiringPiSPI.h>

static uint8_t u8x8_wiringpi_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
static uint8_t u8x8_byte_wiringpi_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif

extern "C" {
extern uint8_t u8x8_d_st7586s_erc240160_chunked(
    u8x8_t *u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void *arg_ptr
);
}

struct Display::Private
{
    u8g2_t u8g2;
};

Display::Display()
    : _p{ std::make_unique<Private>() }
{
    setup();
    setFont(Font{ Font::Family::NormalText });
}

Display::~Display() = default;

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

void Display::setDrawColor(const Color color)
{
    u8g2_SetDrawColor(&_p->u8g2, static_cast<int>(color));
}

void Display::setFont(const Font& font)
{
    if (const auto* const data = font.data(); data != nullptr) {
        u8g2_SetFont(&_p->u8g2, data);
    }
}

void Display::setClipRect(const Rect& rect)
{
    u8g2_SetClipWindow(
        &_p->u8g2,
        rect.x(),
        rect.y(),
        rect.x() + rect.w(),
        rect.y() + rect.h()
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
    u8g2_DrawFrame(&_p->u8g2, rect.x(), rect.y(), rect.w(), rect.h());
}

void Display::setup()
{
#ifdef RASPBERRY_PI
    if (const auto result = wiringPiSetupGpio(); result != 0) {
        std::cout << "wiringPiSetupFailed: result=" << result << '\n';
        return;
    }

#if 0
    g_i2c_fd = wiringPiI2CSetup(display_dev_id);
    std::cout << "i2c_fd=" << g_i2c_fd << '\n';
#endif
#endif

    uint8_t tileBufHeight;
    uint8_t* buf;

    u8g2_SetupDisplay(
        &_p->u8g2,
        u8x8_d_st7586s_erc240160_chunked,
        u8x8_cad_011,
#ifdef RASPBERRY_PI
        u8x8_byte_wiringpi_hw_spi,
        u8x8_wiringpi_gpio_and_delay
#endif
    );

    buf = u8g2_m_30_20_f(&tileBufHeight);

    u8g2_SetupBuffer(&_p->u8g2, buf, tileBufHeight, u8g2_ll_hvline_horizontal_right_lsb, U8G2_R0);
    u8x8_SetPin(u8g2_GetU8x8(&_p->u8g2), U8X8_PIN_DC, 24);
    u8x8_SetPin(u8g2_GetU8x8(&_p->u8g2), U8X8_PIN_RESET, 25);

    u8g2_InitDisplay(&_p->u8g2);
    u8g2_SetPowerSave(&_p->u8g2, 0);
    u8g2_SetContrast(&_p->u8g2, 60);
}

#ifdef RASPBERRY_PI

#if 0
static const uint8_t display_dev_id = 0x3c;
uint8_t u8g2_rpi_hw_i2c_byte(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr)
{
#ifdef DEBUG
    std::cout << __FUNCTION__ << ": arg_int=" << (int)arg_int << ", msg=";
#endif

    static int s_i2c_mode = 0;
    static bool s_first_byte_after_start = false;

    switch (msg)
    {
        case U8X8_MSG_BYTE_INIT:
#ifdef DEBUG
            std::cout << "U8X8_MSG_BYTE_INIT";
#endif
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
#ifdef DEBUG
            std::cout << "U8X8_MSG_BYTE_START_TRANSFER";
#endif
            s_first_byte_after_start = true;
            s_i2c_mode = 0;
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
#ifdef DEBUG
            std::cout << "U8X8_MSG_BYTE_END_TRANSFER";
#endif
            break;

        case U8X8_MSG_BYTE_SEND:
        {
#ifdef DEBUG
            std::cout << "U8X8_MSG_BYTE_SEND:";
            std::cout << std::hex;
#endif

            if (s_first_byte_after_start)
            {
                s_first_byte_after_start = false;
                s_i2c_mode = *reinterpret_cast<uint8_t*>(arg_ptr);
#ifdef DEBUG
                std::cout << " setting mode to " << s_i2c_mode;
#endif
                break;
            }

            auto p = reinterpret_cast<uint8_t*>(arg_ptr);
            for (int i = 0; i < arg_int; ++i)
            {
                auto data = *p++;
#ifdef DEBUG
                std::cout << " " << (int)data;
#endif
                wiringPiI2CWriteReg8(g_i2c_fd, s_i2c_mode, data);
            }
#ifdef DEBUG
            std::cout << std::dec;
#endif

            break;
        }

        default:
#ifdef DEBUG
            std::cout << "UNKNOWN(" << (int)msg << ")";
#endif
            break;
    }

#ifdef DEBUG
    std::cout << std::endl;
#endif

    return 0;
}

uint8_t u8g2_rpi_gpio_delay(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr)
{
#ifdef DEBUG
    std::cout << __FUNCTION__ << ": arg_int=" << (int)arg_int << ", msg=";
#endif

    switch (msg)
    {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
#ifdef DEBUG
            std::cout << "U8X8_MSG_GPIO_AND_DELAY_INIT";
#endif
            break;

        case U8X8_MSG_DELAY_MILLI:
#ifdef DEBUG
            std::cout << "U8X8_MSG_DELAY_MILLI";
#endif
            delay(arg_int);
            break;

        case U8X8_MSG_GPIO_RESET:
#ifdef DEBUG
            std::cout << "U8X8_MSG_GPIO_RESET";
#endif
            break;

        default:
#ifdef DEBUG
            std::cout << "UNKNOWN(" << (int)msg << ")";
#endif
            break;
    }

#ifdef DEBUG
    std::cout << std::endl;
#endif

    return 0;
}
#endif

void sleep_ms(unsigned long milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void sleep_us(unsigned long microseconds)
{
    struct timespec ts;
    ts.tv_sec = microseconds / 1000 / 1000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

void sleep_ns(unsigned long nanoseconds)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = nanoseconds;
    nanosleep(&ts, NULL);
}

uint8_t u8x8_wiringpi_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static const auto setPin = [](u8x8_t* u8x8, const uint8_t pin, const uint8_t value) {
        if (const auto p = u8x8->pins[pin]; p != U8X8_PIN_NONE) {
            digitalWrite(p, value);
        }
    };

    (void) arg_ptr; /* suppress unused parameter warning */
    switch(msg)
    {
        case U8X8_MSG_DELAY_NANO:            // delay arg_int * 1 nano second
            sleep_ns(arg_int);
            break;

        case U8X8_MSG_DELAY_100NANO:        // delay arg_int * 100 nano seconds
            sleep_ns(arg_int * 100);
            break;

        case U8X8_MSG_DELAY_10MICRO:        // delay arg_int * 10 micro seconds
            sleep_us(arg_int * 10);
            break;

        case U8X8_MSG_DELAY_MILLI:            // delay arg_int * 1 milli second
            sleep_ms(arg_int);
            break;

        case U8X8_MSG_DELAY_I2C:
            // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
            // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
            if(arg_int == 1)
            {
                sleep_us(5);
            }
            else if (arg_int == 4)
            {
                sleep_ns(1250);
            }
            break;

        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            // Function which implements a delay, arg_int contains the amount of ms

            // printf("CLK:%d, DATA:%d, CS:%d, RST:%d, DC:%d\n", u8x8->pins[U8X8_PIN_SPI_CLOCK], u8x8->pins[U8X8_PIN_SPI_DATA], u8x8->pins[U8X8_PIN_CS], u8x8->pins[U8X8_PIN_RESET], u8x8->pins[U8X8_PIN_DC]);
            // printf("SDA:%d, SCL:%d\n", u8x8->pins[U8X8_PIN_I2C_DATA], u8x8->pins[U8X8_PIN_I2C_CLOCK]);

            // Output pins
            for (const auto pin : {
                // SPI
                U8X8_PIN_SPI_CLOCK,
                U8X8_PIN_SPI_DATA,
                U8X8_PIN_CS,
                // 8080 mode
                // D0 --> spi clock
                // D1 --> spi data
                U8X8_PIN_D2,
                U8X8_PIN_D3,
                U8X8_PIN_D4,
                U8X8_PIN_D5,
                U8X8_PIN_D5,
                U8X8_PIN_D6,
                U8X8_PIN_D7,
                U8X8_PIN_E,
                U8X8_PIN_RESET,
                U8X8_PIN_DC,
                // I2C
                U8X8_PIN_I2C_DATA,
                U8X8_PIN_I2C_CLOCK
            }) {
                if (const auto p = u8x8->pins[pin]; p != U8X8_PIN_NONE) {
                    pinMode(p, OUTPUT);
                    digitalWrite(p, HIGH);
                }
            }

            break;

        //case U8X8_MSG_GPIO_D0:                // D0 or SPI clock pin: Output level in arg_int
        //case U8X8_MSG_GPIO_SPI_CLOCK:

        //case U8X8_MSG_GPIO_D1:                // D1 or SPI data pin: Output level in arg_int
        //case U8X8_MSG_GPIO_SPI_DATA:

        case U8X8_MSG_GPIO_D2:                  // D2 pin: Output level in arg_int
            setPin(u8x8, U8X8_PIN_D2, arg_int);
            break;

        case U8X8_MSG_GPIO_D3:                  // D3 pin: Output level in arg_int
	        setPin(u8x8, U8X8_PIN_D3, arg_int);
            break;

        case U8X8_MSG_GPIO_D4:                  // D4 pin: Output level in arg_int
            setPin(u8x8, U8X8_PIN_D4, arg_int);
            break;

        case U8X8_MSG_GPIO_D5:                  // D5 pin: Output level in arg_int
            setPin(u8x8, U8X8_PIN_D5, arg_int);
            break;

        case U8X8_MSG_GPIO_D6:                  // D6 pin: Output level in arg_int
	        setPin(u8x8, U8X8_PIN_D6, arg_int);
            break;

        case U8X8_MSG_GPIO_D7:                  // D7 pin: Output level in arg_int
	        setPin(u8x8, U8X8_PIN_D7, arg_int);
            break;

        case U8X8_MSG_GPIO_E:                   // E/WR pin: Output level in arg_int
	        setPin(u8x8, U8X8_PIN_E, arg_int);
            break;

        case U8X8_MSG_GPIO_I2C_CLOCK:
            // arg_int=0: Output low at I2C clock pin
            // arg_int=1: Input dir with pullup high for I2C clock pin
            setPin(u8x8, U8X8_PIN_I2C_CLOCK, arg_int);
            break;

        case U8X8_MSG_GPIO_I2C_DATA:
            // arg_int=0: Output low at I2C data pin
            // arg_int=1: Input dir with pullup high for I2C data pin
            setPin(u8x8, U8X8_PIN_I2C_DATA, arg_int);
            break;

        case U8X8_MSG_GPIO_SPI_CLOCK:
            //Function to define the logic level of the clockline
            setPin(u8x8, U8X8_PIN_SPI_CLOCK, arg_int);
            break;

        case U8X8_MSG_GPIO_SPI_DATA:
            //Function to define the logic level of the data line to the display
            setPin(u8x8, U8X8_PIN_SPI_DATA, arg_int);
            break;

	    case U8X8_MSG_GPIO_CS:
            // Function to define the logic level of the CS line
            setPin(u8x8, U8X8_PIN_CS, arg_int);
            break;

        case U8X8_MSG_GPIO_DC:
            //Function to define the logic level of the Data/ Command line
            setPin(u8x8, U8X8_PIN_DC, arg_int);
            break;

        case U8X8_MSG_GPIO_RESET:
            //Function to define the logic level of the RESET line
            setPin(u8x8, U8X8_PIN_RESET, arg_int);
            break;

        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_byte_wiringpi_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
        case U8X8_MSG_BYTE_SEND:
            wiringPiSPIDataRW(0, reinterpret_cast<uint8_t*>(arg_ptr), arg_int);
            break;

        case U8X8_MSG_BYTE_INIT: {
            const auto result = wiringPiSPISetup(0, 64000000);
            if (result < 0) {
                std::cout << "Failed to initialize SPI\n";
            }
            break;
        }

        case U8X8_MSG_BYTE_SET_DC:
            u8x8_gpio_SetDC(u8x8, arg_int);
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            break;

        default:
            return 0;
    }

    return 1;
}

#endif