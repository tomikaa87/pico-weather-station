#pragma once

#include "Point.h"
#include "Rect.h"
#include "Size.h"

#include <string>
#include <vector>

class Display;

class Widget
{
public:
    explicit Widget(Widget* parent = nullptr);
    Widget(std::string name, Widget* parent = nullptr);

    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;

    const Point& pos() const;
    const Size& size() const;
    const Rect& rect() const;

    void setPos(Point p);
    void setSize(Size s);
    void setRect(Rect rect);

    Point mapToGlobal(const Point& p) const;
    Point mapToParent(const Point& p) const;

    void paint(Display& display) const;

protected:
    std::string _name;
    Widget* const _parent;
    std::vector<Widget*> _children;
    Rect _rect;
};