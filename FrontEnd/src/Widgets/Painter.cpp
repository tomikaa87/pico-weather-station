#include "Painter.h"

#include "Display.h"
#include "Widget.h"

#include <iostream>
#include <functional>

Painter::Painter()
{}

void Painter::paintWidget(Widget* const widget)
{
#if DEBUG_PAINTER
    std::cout << __FUNCTION__ << ": widget=" << widget->_name << '\n';
#endif

    updateWidgetRepaintFlags(widget);

    const auto needsDisplayUpdate = paintWidgetRecursive(widget);

    if (needsDisplayUpdate) {
#if DEBUG_PAINTER
        std::cout << __FUNCTION__ << ": updating display, widget=" << widget->_name << '\n';
#endif
        widget->_display->update();
    }

    widget->_display->resetClipRect();
}

void Painter::updateWidgetRepaintFlags(Widget* const w)
{
#if DEBUG_PAINTER
    std::cout << __FUNCTION__ << ": widget=" << w->_name << '\n';
#endif

    // Repaint parent if its child requests it (e.g geometry change)
    for (auto* child : w->_children) {
        std::cout << __FUNCTION__ << ": widget=" << w->_name << '\n';
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
#if DEBUG_PAINTER
    std::cout << __FUNCTION__ << ": widget=" << w->_name << '\n';
#endif

    auto needsDisplayUpdate = false;

    if (w->_needsRepaint) {
        const auto clipRect = w->calculateClipRect();
        w->_display->setClipRect(clipRect);

        // Clear the background
        if (w->_backgroundEnabled) {
            w->_display->setDrawColor(Display::Color::White);
            w->_display->fillRect(clipRect);
        }

#if DEBUG_PAINTER
        std::cout << __FUNCTION__ <<
            ": painting, widget=" << w->_name
            << ", rect=" << w->_rect
            << ", clipRect=" << clipRect
            << ", backgroundEnabled=" << w->_backgroundEnabled
            << '\n';
#endif

        w->paint();

        w->_needsRepaint = false;

        needsDisplayUpdate = true;
    }

    for (auto* child : w->_children) {
        needsDisplayUpdate |= paintWidgetRecursive(child);
    }

    return needsDisplayUpdate;
}