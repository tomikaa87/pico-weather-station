#pragma once

#include "Font.h"
#include "Widget.h"

#include <string>

class Label : public Widget
{
public:
    explicit Label(Widget* parent);
    Label(std::string text, Widget* parent);

    virtual void paint() override;

    void setText(std::string text);
    void setFont(const Font& font);

    enum class Align
    {
        Left,
        Center,
        Right
    };

    void setAlignment(Align alignment);

    enum class HeightCalculation
    {
        WithDescent,
        NoDescent
    };

    void setHeightCalculation(HeightCalculation heightCalculation);

private:
    std::string _text;
    Font _font;
    Align _alignment = Align::Left;
    Point _textPos;
    HeightCalculation _heightCalculation = HeightCalculation::WithDescent;

    void updateHeightByFont();
    void updateTextPosition();
};