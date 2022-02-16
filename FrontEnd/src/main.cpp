#include "Fonts.h"
#include "Graphics.h"
#include "ProgressBar.h"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <string>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringPiSPI.h>

#include <u8g2.h>
#include <u8x8.h>
#include <U8g2lib.h>

#include <unistd.h>
#include <termios.h>

#include <fmt/core.h>

char getch()
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
            perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
            perror ("tcsetattr ~ICANON");
    return (buf);
}

static const uint8_t display_dev_id = 0x3c;

int g_i2c_fd = 0;

// #define DEBUG

extern "C" {
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

} // extern "C"

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

extern "C" {
extern uint8_t u8x8_d_st7586s_erc240160_chunked(
    u8x8_t *u8x8,
    uint8_t msg,
    uint8_t arg_int,
    void *arg_ptr
);
}

u8g2_t setup_display()
{
    std::cout << "setting up the display" << std::endl;

    u8g2_t u8g2;
    // u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_rpi_hw_i2c_byte, u8g2_rpi_gpio_delay);
    // u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_rpi_hw_i2c_byte, u8g2_rpi_gpio_delay);
    // u8g2_Setup_st7586s_erc240160_1(&u8g2, U8G2_R0, u8g2_rpi_gpio_delay, u8g2_rpi_gpio_delay);
    // u8x8_SetPin_4Wire_HW_SPI(u8g2_GetU8x8(&u8g2), cs, dc, reset);
    // u8g2_Setup_st7586s_erc240160_f(&u8g2, U8G2_R0, u8x8_byte_wiringpi_hw_spi, u8x8_wiringpi_gpio_and_delay);
    uint8_t tile_buf_height;
    uint8_t *buf;
    u8g2_SetupDisplay(
        &u8g2,
        u8x8_d_st7586s_erc240160_chunked,
        u8x8_cad_011,
        u8x8_byte_wiringpi_hw_spi,
        u8x8_wiringpi_gpio_and_delay
    );
    buf = u8g2_m_30_20_f(&tile_buf_height);
    u8g2_SetupBuffer(&u8g2, buf, tile_buf_height, u8g2_ll_hvline_horizontal_right_lsb, U8G2_R0);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_DC, 24);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_RESET, 25);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_SetContrast(&u8g2, 60);

    return u8g2;
}

u8g2_t* g_u8g2 = nullptr;

void signalHandler(const int)
{
    u8g2_ClearDisplay(g_u8g2);
    exit(0);
}

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

int main()
{
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    if (const auto result = wiringPiSetupGpio(); result != 0) {
        std::cout << "wiringPiSetupFailed: result=" << result << '\n';
        return 1;
    }

    g_i2c_fd = wiringPiI2CSetup(display_dev_id);
    std::cout << "i2c_fd=" << g_i2c_fd << '\n';

    auto u8g2 = setup_display();
    g_u8g2 = &u8g2;

    std::cout << "drawing test picture" << std::endl;

    // u8g2_ClearBuffer(&u8g2);
    u8g2_DrawXBM(&u8g2, 0, 0, Graphics::MainScreen_width, Graphics::MainScreen_height, Graphics::MainScreen_bits);
    // u8g2_SetDrawColor(&u8g2, 1);
    // u8g2_DrawCircle(&u8g2, 20, 20, 10, U8G2_DRAW_ALL);
    u8g2_SetFont(&u8g2, Fonts::PFT7Condensed);
    // u8g2_DrawStr(&u8g2, 0, 140, "Hello");
    u8g2_SendBuffer(&u8g2);

    // u8g2_SetDrawColor(&u8g2, 1);
    // u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
    // uint8_t counter = 0;

    // std::cout << "starting main loop\n";

    // while (true) {
    //     u8g2_ClearBuffer(&u8g2);

    //     u8g2_SendBuffer(&u8g2);
    //     return 0;

    //     u8g2_DrawStr(&u8g2, 0, 7, std::to_string(counter++).c_str());
    //     u8g2_SendBuffer(&u8g2);
    //     // delay(100);
    // }

    int w = 128;
    int h = 10;
    int progress = 50;

    uint8_t contrast = 60;

    while (true) {
        const auto ch = ::getch();
        auto update = true;
        auto updateLabelOnly = false;

        switch (ch) {
            case 'w':
                if (h > 0) {
                    --h;
                }
                break;
            case 's':
                if (h < 63) {
                    ++h;
                }
                break;
            case 'a':
                if (w > 0) {
                    --w;
                }
                break;
            case 'd':
                if (w < 127) {
                    ++w;
                }
                break;
            case 'q':
                if (progress > 0) {
                    --progress;
                }
                break;
            case 'e':
                if (progress < 100) {
                    ++progress;
                }
                break;
            case 'z':
                --contrast;
                u8g2_SetContrast(&u8g2, contrast);
                std::cout << "contrast=" << static_cast<int>(contrast) << '\n';
                updateLabelOnly = true;
                break;
            case 'x':
                ++contrast;
                u8g2_SetContrast(&u8g2, contrast);
                std::cout << "contrast=" << static_cast<int>(contrast) << '\n';
                updateLabelOnly = true;
                break;
            default:
                break;
        }

        if (update) {
            u8g2_ClearBuffer(&u8g2);
            if (updateLabelOnly) {
                u8g2_DrawXBM(&u8g2, 0, 0, Graphics::MainScreen_width, Graphics::MainScreen_height, Graphics::MainScreen_bits);
                u8g2_SetFont(&u8g2, Fonts::PFT7Condensed);
                u8g2_DrawStr(&u8g2, 0, 140, fmt::format("Contrast = {}", contrast).c_str());
            } else {
                drawFancyProgressBar(&u8g2, 0, 0, w, h, progress);
            }
            u8g2_SendBuffer(&u8g2);
        }
    }

    // // Background
    // u8g2_SetDrawColor(&u8g2, 1);
    // u8g2_DrawRBox(&u8g2, 0, 20, 127, 10, 1);
    // u8g2_DrawRBox(&u8g2, 1, 21, 127, 10, 1);
    // // Empty space for the bar
    // u8g2_SetDrawColor(&u8g2, 0);
    // u8g2_DrawRBox(&u8g2, 1, 21, 125, 8, 1);
    // // The bar itself
    // u8g2_SetDrawColor(&u8g2, 1);
    // for (auto i = 0u; i < 64; ++i) {
    //     if (i % 2 == 0) {
    //         for (auto j = 0u; j < 3; ++j) {
    //             u8g2_DrawPixel(&u8g2, i + 3, 22 + j * 2);
    //         }
    //     } else {
    //         for (auto j = 0u; j < 3; ++j) {
    //             u8g2_DrawPixel(&u8g2, i + 3, 22 + j * 2 + 1);
    //         }
    //     }
    // }

    // u8g2_SetFont(&u8g2, PFT7Condensed);
    // u8g2_SetDrawColor(&u8g2, 1);
    // u8g2_DrawStr(&u8g2, 0, 40, "PF Tempesta 7 Condensed");

    u8g2_SendBuffer(&u8g2);

#if 0
    ProgressBar progressBar{ u8g2 };
    progressBar.setRect(0, 0, 128, 10);

    // progressBar.setPosition(100);
    // u8g2_SendBuffer(&u8g2);

    int pos = 100;
    while (true) {
        progressBar.setPosition(pos);
        progressBar.setText("Pos: " + std::to_string(pos) + "/100");
        progressBar.update();
        ++pos;
        if (pos >= 100) {
            pos = 0;
        }
        u8g2_SendBuffer(&u8g2);
    }
#endif

    std::cout << "Works\n";
    return 0;
}