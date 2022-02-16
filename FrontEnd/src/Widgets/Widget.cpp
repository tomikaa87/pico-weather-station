#include "Widget.h"

#include "Display.h"

#include <algorithm>

Widget::Widget(Widget* parent)
    : Widget{ {}, parent }
{}

Widget::Widget(std::string name, Widget* parent)
    : _parent{ parent }
    , _name{ std::move(name) }
{
    if (_parent) {
        _parent->_children.push_back(this);
    }
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

void Widget::paint(Display& display) const
{
    for (auto child : _children) {
        child->paint(display);
    }

    printf("Painter::paint: widget=%s\r\n", _name.c_str());

    const auto mappedRect = Rect{
        mapToGlobal(pos()),
        size()
    };

    display.drawRect(mappedRect);
}