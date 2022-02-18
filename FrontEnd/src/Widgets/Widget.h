#pragma once

#include "Point.h"
#include "Rect.h"
#include "Size.h"

#include <iostream>
#include <string>
#include <vector>

class Display;

class Widget
{
    friend class Painter;

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
    void setWidth(int width);
    void setHeight(int height);
    void setRect(Rect rect);

    void setBackgroundEnabled(bool enabled);

    Point mapToGlobal(const Point& p) const;
    Rect mapToGlobal(const Rect& r) const;
    Point mapToParent(const Point& p) const;
    Rect mapToParent(const Rect& r) const;

protected:
    Display* const _display;
    Widget* const _parent;
    std::string _name;
    std::vector<Widget*> _children;
    Rect _rect;
    bool _needsRepaint = true;
    bool _parentNeedsRepaint = true;
    bool _backgroundEnabled = true;

    virtual void paint();

    Rect calculateClipRect() const;
};