#include "Painter.h"

#include "Display.h"
#include "Widget.h"

#include <functional>

Painter::Painter()
{}

void Painter::paintWidget(Widget* widget)
{
    updateWidgetRepaintFlags(widget);

    const auto needsDisplayUpdate = paintWidgetRecursive(widget);

    if (needsDisplayUpdate) {
        widget->_display->update();
    }
}

void Painter::updateWidgetRepaintFlags(Widget* const w)
{
    // Repaint parent if its child requests it (e.g geometry change)
    for (auto* child : w->_children) {
        w->_needsRepaint |= child->_parentNeedsRepaint;
        child->_parentNeedsRepaint = false;
    }

    // Children must be repainted if parent is repainted
    for (auto* child : w->_children) {
        child->_needsRepaint |= w->_needsRepaint;
        updateWidgetRepaintFlags(child);
    }
}

bool Painter::paintWidgetRecursive(Widget* const w)
{
    auto needsDisplayUpdate = false;

    if (w->_needsRepaint) {
        // Clear the background
        if (w->_backgroundEnabled) {
            w->_display->setDrawColor(Display::Color::White);
            w->_display->fillRect(w->calculateClipRect());
        }

        w->paint();

        w->_needsRepaint = false;

        needsDisplayUpdate = true;
    }

    for (auto* child : w->_children) {
        needsDisplayUpdate |= paintWidgetRecursive(child);
    }

    return needsDisplayUpdate;
}