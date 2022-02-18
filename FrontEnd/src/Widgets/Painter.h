#pragma once

class Widget;

class Painter
{
public:
    explicit Painter();

    void paintWidget(Widget* widget);
};