#pragma once

#include "Font.h"
#include "Point.h"
#include "Rect.h"
#include "Size.h"

#include <memory>
#include <string>

class Display
{
public:
    explicit Display();
    ~Display();

    enum class Color
    {
        Black,
        White,
        Xor
    };

    Size size() const;

    void clear();
    void clearBuffer();

    void update();

    void setContrast(uint8_t value);
    void setDrawColor(Color color);
    void setFont(const Font& font);
    void setClipRect(const Rect& rect);
    void resetClipRect();

    [[nodiscard]] int calculateFontAscent() const;
    [[nodiscard]] int calculateFontDescent() const;
    [[nodiscard]] int calculateMaxCharHeight() const;
    [[nodiscard]] int calculateTextWidth(const std::string& text) const;

    void drawText(const Point& pos, const std::string& s);
    void drawBitmap(const Point& pos, int width, int height, const uint8_t* data);
    void drawRect(const Rect& rect);
    void drawLine(const Point& from, const Point& to);

    void fillRect(const Rect& rect);

private:
    struct Private;
    std::unique_ptr<Private> _p;

    void setup();
};