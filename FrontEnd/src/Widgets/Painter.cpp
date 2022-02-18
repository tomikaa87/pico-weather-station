#include "Painter.h"

#include "Display.h"
#include "Widget.h"

#include <functional>

Painter::Painter()
{}

void Painter::paintWidget(Widget* widget)
{
    static std::function<void (Widget*)> updateRepaintFlags = [](Widget* w) {
        for (auto* child : w->_children) {
            w->_needsRepaint |= child->_parentNeedsRepaint;
            child->_parentNeedsRepaint = false;

            updateRepaintFlags(child);
        }
    };

    static std::function<void (Widget*)> paintAllWidgets = [](Widget* w) {
        if (w->_needsRepaint) {
            // Clear the background
            w->_display->setDrawColor(Display::Color::White);
            w->_display->fillRect(w->calculateClipRect());

            w->paint();

            w->_needsRepaint = false;
        }

        for (auto* child : w->_children) {
            paintAllWidgets(child);
        }
    };

    updateRepaintFlags(widget);
    paintAllWidgets(widget);
}