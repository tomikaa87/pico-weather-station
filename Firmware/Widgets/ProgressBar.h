#pragma once

#include "Rect.h"

#include <U8g2lib.h>
#include <U8x8lib.h>

#include <string>

class ProgressBar
{
public:
    explicit ProgressBar(u8g2_t& display);

    void setRect(int x, int y, int w, int h);
    void setPosition(int pos);
    void setText(std::string text);

    void update();

private:
    u8g2_t& _display;
    Rect _rect;
    int _position = 0;
    std::string _text;
    bool _useDefaultText = true;
};