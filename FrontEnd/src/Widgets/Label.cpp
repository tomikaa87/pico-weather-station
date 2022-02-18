#include "Label.h"

#include "Display.h"

Label::Label(Widget* parent)
    : Widget{ parent }
{
    updateHeightByFont();
}

Label::Label(std::string text, Widget* parent)
    : Widget{ parent }
    , _text{ std::move(text) }
{
    updateHeightByFont();
}

void Label::setText(std::string text)
{
    _text = std::move(text);
    _needsRepaint = true;
}

void Label::setFont(const Font& font)
{
    _font = font;
    updateHeightByFont();
}

void Label::paint()
{
    std::cout << "Label::paint()\n";

    _display->setDrawColor(Display::Color::Black);
    _display->setFont(_font);
    _display->setClipRect(calculateClipRect());

    _display->drawText(
        mapToGlobal(_rect.pos()) + Point{ 0, _display->calculateFontAscent() + 1 },
        _text
    );

    _display->resetClipRect();

    Widget::paint();
}

void Label::updateHeightByFont()
{
    _display->setFont(_font);
    setHeight(_display->calculateMaxCharHeight() + 1);
}