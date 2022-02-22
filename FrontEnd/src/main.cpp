#include "Fonts.h"
#include "Graphics.h"
#include "Icons.h"

#include "Screens/MainScreen.h"

#include "Widgets/Display.h"
#include "Widgets/Image.h"
#include "Widgets/Label.h"
#include "Widgets/Painter.h"
#include "Widgets/ProgressBar.h"
#include "Widgets/Widget.h"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <unistd.h>
#include <termios.h>

#include <fmt/core.h>

using namespace std::chrono_literals;

static Display display;

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

void signalHandler(const int)
{
    display.clear();
    exit(0);
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

int main()
{
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    display.clear();

#if 1
    MainScreen mainScreen{ &display };
    mainScreen.setCurrentTemperature(-88);
    mainScreen.setCurrentSensorTemperature(-88);
    mainScreen.setCurrentPressure(8888);
    mainScreen.setCurrentHumidity(888);
    mainScreen.setCurrentWindSpeed(888);
    mainScreen.setCurrentWindGustSpeed(818);
    mainScreen.setInternalSensorHumidity(88.8);
    mainScreen.setInternalSensorTemperature(88.8);
#else
    Widget mainScreen{ &display };
    mainScreen.setRect({ 0, 0, 240, 160 });
    Image mainScreenImage{ &mainScreen };
    mainScreenImage.setRect({ 0, 0, 240, 160 });
    mainScreenImage.setImage(Graphics::MainScreenTest, 240, 160);
#endif

    Painter painter;
    painter.paintWidget(&mainScreen);

    return 0;

    int temperature = -30;
    int pressure = 1000;
    int humidity = 50;
    int windSpeed = 50;

    while (true) {
        mainScreen.setCurrentTemperature(temperature);
        mainScreen.setCurrentSensorTemperature(temperature);
        mainScreen.setCurrentPressure(pressure);
        mainScreen.setCurrentHumidity(humidity);
        mainScreen.setCurrentWindSpeed(windSpeed);
        mainScreen.setCurrentWindGustSpeed(windSpeed);

        painter.paintWidget(&mainScreen);

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

        std::this_thread::sleep_for(500ms);
    }

    return 0;

    // Widget screen{ &display };
    // screen.setRect({ 0, 0, 240, 160 });
    // screen.setName("screen");

    // // Widget w1{ &screen };
    // // w1.setName("w1");
    // // w1.setRect({ 10, 10, 100, 100 });

    // // Widget w1_1{ &w1 };
    // // w1_1.setName("w1_1");
    // // w1_1.setRect({ 1, 1, 10, 10 });

    // Label l1{ "This is a label", &screen };
    // l1.setPos({ 2, 2 });
    // l1.setWidth(100);
    // Font font{ Font::Family::PfTempesta7, Font::Style::Compressed };
    // font.setBold(true);
    // l1.setFont(font);
    // l1.setName("label1");
    // // l1.setBackgroundEnabled(false);

    // Image i1{
    //     Graphics::Icons::Weather::Cloud,
    //     Graphics::Icons::Weather::Width,
    //     Graphics::Icons::Weather::Height,
    //     &screen
    // };
    // i1.setRect({ 2, 30, 70, 70 });
    // i1.setName("image1");
    // // i1.setInverted(true);

    // Painter painter;

    // for (auto i = 0; i < 20; ++i) {
    //     // w1.setPos(w1.pos() + Point{ 1, 1 });
    //     if (i % 2 == 0)
    //         l1.setText(fmt::format("ABCD {{[ijklypg Counter]}} = {}", i));
    //     painter.paintWidget(&screen);
    // }

    // for (auto image : {
    //     Graphics::Icons::Weather::Cloud,
    //     Graphics::Icons::Weather::CloudsWithIceCubes,
    //     Graphics::Icons::Weather::CloudsWithRaindrops,
    //     Graphics::Icons::Weather::CloudsWithRaindropsAndIceCubes,
    //     Graphics::Icons::Weather::CloudsWithRaindropsAndSnowflakes,
    //     Graphics::Icons::Weather::CloudsWithSleet,
    //     Graphics::Icons::Weather::CloudsWithSnowflakes,
    //     Graphics::Icons::Weather::CloudWithRaindrops,
    //     Graphics::Icons::Weather::CloudWithSnowflakes,
    //     Graphics::Icons::Weather::CloudWithThunderbolt,
    //     Graphics::Icons::Weather::Fire,
    //     Graphics::Icons::Weather::Fog,
    //     Graphics::Icons::Weather::Moon,
    //     Graphics::Icons::Weather::MoonWithCloud,
    //     Graphics::Icons::Weather::MoonWithCloudAndRaindrops,
    //     Graphics::Icons::Weather::MoonWithCloudAndSnowflakes,
    //     Graphics::Icons::Weather::MoonWithCloudAndThunderbolt,
    //     Graphics::Icons::Weather::MoonWithClouds,
    //     Graphics::Icons::Weather::MoonWithCloudsAndRaindrops,
    //     Graphics::Icons::Weather::MoonWithCloudsAndSnowflakes,
    //     Graphics::Icons::Weather::MoonWithCloudsAndThunderbolt,
    //     Graphics::Icons::Weather::MoonWithMoreClouds,
    //     Graphics::Icons::Weather::QuestionMark,
    //     Graphics::Icons::Weather::Snowflake,
    //     Graphics::Icons::Weather::Sun,
    //     Graphics::Icons::Weather::SunHot,
    //     Graphics::Icons::Weather::SunWithCloud,
    //     Graphics::Icons::Weather::SunWithCloudAndRaindrops,
    //     Graphics::Icons::Weather::SunWithCloudAndSnowflakes,
    //     Graphics::Icons::Weather::SunWithCloudAndThunderbolt,
    //     Graphics::Icons::Weather::SunWithClouds,
    //     Graphics::Icons::Weather::SunWithCloudsAndRaindrops,
    //     Graphics::Icons::Weather::SunWithCloudsAndSnowflakes,
    //     Graphics::Icons::Weather::SunWithCloudsAndThunderbolt,
    //     Graphics::Icons::Weather::SunWithMoreClouds,
    //     Graphics::Icons::Weather::Whirlpools
    // }) {
    //     i1.setImage(image, 70, 70);
    //     painter.paintWidget(&screen);
    // }

    // return 0;

    // display.drawBitmap(
    //     { 0, 0 },
    //     Graphics::MainScreen_width,
    //     Graphics::MainScreen_height,
    //     Graphics::MainScreen_bits
    // );
    // display.drawText({ 20, 140 }, "It works!");

    // auto u8g2 = setup_display();
    // g_u8g2 = &u8g2;

    std::cout << "drawing test picture" << std::endl;

    // u8g2_ClearBuffer(&u8g2);
    // u8g2_DrawXBM(&u8g2, 0, 0, Graphics::MainScreen_width, Graphics::MainScreen_height, Graphics::MainScreen_bits);
    // u8g2_SetDrawColor(&u8g2, 1);
    // u8g2_DrawCircle(&u8g2, 20, 20, 10, U8G2_DRAW_ALL);
    // u8g2_SetFont(&u8g2, Fonts::PFT7Condensed);
    // u8g2_DrawStr(&u8g2, 0, 140, "Hello");
    // u8g2_SendBuffer(&u8g2);

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
                // u8g2_SetContrast(&u8g2, contrast);
                std::cout << "contrast=" << static_cast<int>(contrast) << '\n';
                updateLabelOnly = true;
                break;
            case 'x':
                ++contrast;
                // u8g2_SetContrast(&u8g2, contrast);
                std::cout << "contrast=" << static_cast<int>(contrast) << '\n';
                updateLabelOnly = true;
                break;
            case ' ':
                return 0;
            default:
                break;
        }

#if 0
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
#endif
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

    // u8g2_SendBuffer(&u8g2);

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