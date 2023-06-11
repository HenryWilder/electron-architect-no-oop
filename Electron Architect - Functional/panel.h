#pragma once
#include <raylib.h>

// Functions shared by each of the panels.
namespace panel
{
    extern Color background; // Color of standard panel background
    extern Color accent; // Color of standard panel parts separating sections
    extern Color foreground; // Color of standard panel text
    extern Color draggableHighlight; // Color of standard panel text

    constexpr int titleInset = 8; // Padding of standard panel title in pixels
    constexpr int titleSize = 16; // Size of title font
    constexpr int panelDraggableRadius = 6; // Half-size of the draggable section of a panel edge in pixels
    constexpr int panelDraggableWidth = panelDraggableRadius * 2; // Full size of the draggable section of a panel edge in pixels

    // Draws a panel with a title bar and optional highlighting for draggability
    void DrawPanel(const char* title, int xmin, int ymin, int xmax, int ymax,
        bool draggableLeft, bool draggableTop, bool draggableRight, bool draggableBottom,
        int mousex, int mousey);
}
