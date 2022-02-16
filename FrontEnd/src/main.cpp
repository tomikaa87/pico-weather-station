#include "Fonts.h"
#include "Graphics.h"

#include "Widgets/Display.h"
#include "Widgets/ProgressBar.h"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <string>

#include <unistd.h>
#include <termios.h>

#include <fmt/core.h>

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