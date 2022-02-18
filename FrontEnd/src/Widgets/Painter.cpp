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

    static std::function<bool (Widget*)> paintAllWidgets = [](Widget* w) {
        auto needsDisplayUpdate = false;

        if (w->_needsRepaint) {
            // Clear the background
            w->_display->setDrawColor(Display::Color::White);
            w->_display->fillRect(w->calculateClipRect());

            w->paint();

            w->_needsRepaint = false;

            needsDisplayUpdate = true;
        }

        for (auto* child : w->_children) {
            needsDisplayUpdate |= paintAllWidgets(child);
        }

        return needsDisplayUpdate;
    };

    updateRepaintFlags(widget);

    const auto needsDisplayUpdate = paintAllWidgets(widget);

    if (needsDisplayUpdate) {
        widget->_display->update();
    }
}