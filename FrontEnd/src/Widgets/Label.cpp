#include "Label.h"

#include "Display.h"

Label::Label(Widget* parent)
    : Widget{ parent }
{}

Label::Label(std::string text, Widget* parent)
    : Widget{ parent }
    , _text{ std::move(text) }
{}

void Label::paint() const
{
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