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
    explicit Widget(Display* display);
    explicit Widget(Widget* parent);

    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;

    void setName(std::string name);

    const Point& pos() const;
    const Size& size() const;
    const Rect& rect() const;

    void setPos(Point p);
    void setSize(Size s);
    void setRect(Rect rect);

    Point mapToGlobal(const Point& p) const;
    Rect mapToGlobal(const Rect& r) const;
    Point mapToParent(const Point& p) const;
    Rect mapToParent(const Rect& r) const;

    virtual void paint() const;

protected:
    Display* const _display;
    Widget* const _parent;
    std::string _name;
    std::vector<Widget*> _children;
    Rect _rect;

    Rect calculateClipRect() const;
};