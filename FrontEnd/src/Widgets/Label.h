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

private:
    std::string _text;
    Font _font;

    void updateHeightByFont();
};