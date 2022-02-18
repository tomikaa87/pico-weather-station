#include "Label.h"

#include "Display.h"

Label::Label(Widget* parent)
    : Widget{ parent }
{}

Label::Label(std::string text, Widget* parent)
    : Widget{ parent }
    , _text{ std::move(text) }
{}

void Label::setText(std::string text)
{
    _text = std::move(text);
    _needsRepaint = true;
}

void Label::paint()
{
    std::cout << "Label::paint()\n";

    _display->setDrawColor(Display::Color::Black);
    _display->setFont(_font);
    _display->setClipRect(calculateClipRect());

    // TODO implement text rect calculation

    _display->drawText(
        mapToGlobal(_rect.pos()) + Point{ 0, _display->calculateFontAscent() },
        _text
    );

    _display->resetClipRect();

    Widget::paint();
}