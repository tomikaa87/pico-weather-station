#include "Widget.h"

#include "Display.h"

#include <algorithm>

#ifndef WIDGET_DEBUG
#define WIDGET_DEBUG 0
#endif

Widget::Widget(Display* display)
    : _display{ display }
    , _parent{ nullptr }
{}

Widget::Widget(Widget* parent)
    : _display{ parent->_display }
    , _parent{ parent }
{
    parent->_children.push_back(this);
}

Widget::~Widget()
{
    if (_parent) {
        std::erase(
            _children,
            this
        );
    }
}

void Widget::setName(std::string name)
{
    _name = std::move(name);
}

const Point& Widget::pos() const
{
    return _rect.pos();
}

const Size& Widget::size() const
{
    return _rect.size();
}

const Rect& Widget::rect() const
{
    return _rect;
}

void Widget::setPos(Point p)
{
    _rect.setPos(std::move(p));
    _needsRepaint = true;
    _parentNeedsRepaint = true;
}

void Widget::setSize(Size s)
{
    _rect.setSize(std::move(s));
    _needsRepaint = true;
    _parentNeedsRepaint = true;
}

void Widget::setWidth(const int width)
{
    _rect.setWidth(width);
    _needsRepaint = true;
    _parentNeedsRepaint = true;
}

void Widget::setHeight(const int height)
{
    _rect.setHeight(height);
    _needsRepaint = true;
    _parentNeedsRepaint = true;
}

void Widget::setRect(Rect r)
{
    _rect = std::move(r);
    _needsRepaint = true;
    _parentNeedsRepaint = true;
}

void Widget::setBackgroundEnabled(const bool enabled)
{
    _backgroundEnabled = enabled;
    _needsRepaint = true;
}

Point Widget::mapToGlobal(const Point& p) const
{
    Point mappedPoint = p;
    const Widget* w = this;

    while (w) {
        mappedPoint = w->mapToParent(mappedPoint);
        w = w->_parent;
    }

    return mappedPoint;
}

Rect Widget::mapToGlobal(const Rect& r) const
{
    return Rect{
        mapToGlobal(r.pos()),
        r.size()
    };
}

Point Widget::mapToParent(const Point& p) const
{
    if (!_parent) {
        return p;
    }

    return Point{
        p.x() + _parent->pos().x(),
        p.y() + _parent->pos().y()
    };
}

Rect Widget::mapToParent(const Rect& r) const
{
    return Rect{
        mapToParent(r.pos()),
        r.size()
    };
}

void Widget::paint()
{
    std::cout << "Widget::paint()\n";

#if WIDGET_DEBUG
    _display->setClipRect(calculateClipRect());
    _display->setDrawColor(Display::Color::Black);
    _display->drawRect(mapToGlobal(_rect));
    _display->resetClipRect();
#endif
}

Rect Widget::calculateClipRect() const
{
    if (!_parent) {
        return _rect;
    }

    const auto selfGlobalRect = mapToGlobal(_rect);
    const auto parentGlobalRect = _parent->mapToGlobal(_parent->_rect);

    return parentGlobalRect & selfGlobalRect;
}