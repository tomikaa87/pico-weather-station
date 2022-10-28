#pragma once

class Widget;

class Painter
{
public:
    explicit Painter();

    void paintWidget(Widget* widget);

private:
    static void updateWidgetRepaintFlags(Widget* w);
    static bool paintWidgetRecursive(Widget* w);
};