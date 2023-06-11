#include "panel.h"

// Inclusive
bool Between(int min, int x, int max)
{
    return min <= x && x <= max;
}

namespace panel
{
    Color background = BLACK;
    Color accent = DARKGRAY;
    Color foreground = WHITE;
    Color draggableHighlight = YELLOW;

    void DrawPanel(const char* title, int xmin, int ymin, int xmax, int ymax,
        bool draggableLeft, bool draggableTop, bool draggableRight, bool draggableBottom,
        int mousex, int mousey)
    {
        // Main panel
        DrawRectangle(xmin, ymin, xmax - xmin, ymax - ymin, background);

        // Title section
        DrawRectangle(xmin, ymin, xmax - xmin, ymin + titleInset * 2, accent);
        DrawText(title, xmin + titleInset, ymin + titleInset, titleSize, foreground);

        // No more is needed if there's no hover highlighting
        if (!draggableLeft && !draggableTop && !draggableRight && !draggableBottom)
            return;

        /*
        *         ot
        *     .========.
        *    ||   it   ||
        * ol ||il    ir|| or
        *    ||   ib   ||
        *     '========'
        *         ob
        */

        int xminDragOuter = xmin - panelDraggableRadius; // Outer Left   (ol)
        int xmaxDragOuter = xmax + panelDraggableRadius; // Outer Right  (or)
        int yminDragOuter = ymin - panelDraggableRadius; // Outer Top    (ot)
        int ymaxDragOuter = ymax + panelDraggableRadius; // Outer Bottom (ob)

        int xminDragInner = xmin + panelDraggableRadius; // Inner Left   (il)
        int xmaxDragInner = xmax - panelDraggableRadius; // Inner Right  (ir)
        int yminDragInner = ymin + panelDraggableRadius; // Inner Top    (it)
        int ymaxDragInner = ymax - panelDraggableRadius; // Inner Bottom (ib)

        bool mouseInRow = (xminDragOuter <= mousex && mousex <= xmaxDragOuter); // Mouse is within the horizontal bounds
        bool mouseInCol = (yminDragOuter <= mousey && mousey <= ymaxDragOuter); // Mouse is within the vertical   bounds

        bool mouseInLCol = mouseInCol && (xminDragOuter <= mousex && mousex <= xminDragInner); // Left
        bool mouseInRCol = mouseInCol && (xmaxDragInner <= mousex && mousex <= xmaxDragOuter); // Right
        bool mouseInTRow = mouseInRow && (yminDragOuter <= mousey && mousey <= yminDragInner); // Top
        bool mouseInBRow = mouseInRow && (ymaxDragOuter <= mousey && mousey <= ymaxDragInner); // Bottom

        bool inDraggableLCol = mouseInLCol && draggableLeft;   // In left   edge and draggable
        bool inDraggableRCol = mouseInRCol && draggableRight;  // In right  edge and draggable
        bool inDraggableTRow = mouseInTRow && draggableTop;    // In top    edge and draggable
        bool inDraggableBRow = mouseInBRow && draggableBottom; // In bottom edge and draggable

        bool inDraggableTL = inDraggableLCol && inDraggableTRow; // In top    left  corner and both are draggable
        bool inDraggableBR = inDraggableRCol && inDraggableBRow; // In bottom right corner and both are draggable
        bool inDraggableTR = inDraggableTRow && inDraggableRCol; // In top    right corner and both are draggable
        bool inDraggableBL = inDraggableBRow && inDraggableLCol; // In bottom left  corner and both are draggable

        bool inRight  = inDraggableRCol; // In right
        bool inBottom = inDraggableBRow; // In bottom

        bool inRow = inDraggableTRow || inDraggableBRow; // In row
        bool inCol = inDraggableLCol || inDraggableRCol; // In column

        bool inCorner = inDraggableTL   || inDraggableTR   || inDraggableBR   || inDraggableBL;   // In corner
        bool inEdge   = inDraggableLCol || inDraggableTRow || inDraggableRCol || inDraggableBRow; // In edge

        bool inDraggable = inEdge;

        int x = inRight  ? xmaxDragInner : xminDragOuter; // Left corner of the hover element
        int y = inBottom ? ymaxDragInner : yminDragOuter; // Top  corner of the hover element

        int w = (inCorner || !inRow) ? (panelDraggableWidth) : (xmaxDragOuter - xminDragOuter); // Width  of the hover element
        int h = (inCorner || !inCol) ? (panelDraggableWidth) : (ymaxDragOuter - yminDragOuter); // Height of the hover element

        if (inDraggable)
        {
            DrawRectangle(x, y, w, h, draggableHighlight);
        }
    }
}
