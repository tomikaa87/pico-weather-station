#include "Widget.h"

#include "Display.h"

#include <algorithm>

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
}

void Widget::setSize(Size s)
{
    _rect.setSize(std::move(s));
}

void Widget::setRect(Rect r)
{
    _rect = std::move(r);
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

void Widget::paint() const
{
    for (auto child : _children) {
        child->paint();
    }

    printf("Painter::paint: widget=%s\r\n", _name.c_str());

    const auto mappedRect = Rect{
        mapToGlobal(pos()),
        size()
    };

    _display->setClipRect(mappedRect);
    _display->drawRect(mappedRect);
    _display->resetClipRect();
}