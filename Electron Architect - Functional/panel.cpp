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

        int xminDragOuter = xmin - panelDraggableRadius; // Outer Left (ol)
        int yminDragOuter = ymin - panelDraggableRadius; // Outer Top (ot)
        int xmaxDragOuter = xmax + panelDraggableRadius; // Outer Right (or)
        int ymaxDragOuter = ymax + panelDraggableRadius; // Outer Bottom (ob)

        int xminDragInner = xmin + panelDraggableRadius; // Inner Left (il)
        int yminDragInner = ymin + panelDraggableRadius; // Inner Top (it)
        int xmaxDragInner = xmax - panelDraggableRadius; // Inner Right (ir)
        int ymaxDragInner = ymax - panelDraggableRadius; // Inner Bottom (ib)

        bool mouseInLCol = Between(xminDragOuter, mousex, xminDragInner) && Between(yminDragOuter, mousey, ymaxDragOuter); // Left
        bool mouseInTRow = Between(xminDragOuter, mousex, xmaxDragOuter) && Between(yminDragOuter, mousey, yminDragInner); // Top
        bool mouseInRCol = Between(xmaxDragInner, mousex, xmaxDragOuter) && Between(yminDragOuter, mousey, ymaxDragOuter); // Right
        bool mouseInBRow = Between(xminDragOuter, mousex, xmaxDragOuter) && Between(ymaxDragOuter, mousey, ymaxDragInner); // Bottom

        bool inDraggableLCol = mouseInLCol && draggableLeft;
        bool inDraggableTRow = mouseInTRow && draggableTop;
        bool inDraggableRCol = mouseInRCol && draggableRight;
        bool inDraggableBRow = mouseInBRow && draggableBottom;

        bool inDraggableTL = inDraggableLCol && inDraggableTRow;
        bool inDraggableTR = inDraggableTRow && inDraggableRCol;
        bool inDraggableBR = inDraggableRCol && inDraggableBRow;
        bool inDraggableBL = inDraggableBRow && inDraggableLCol;

        bool inRight = inDraggableRCol; // In right
        bool inBottom = inDraggableBRow; // In bottom
        bool inRow = inDraggableTRow || inDraggableBRow; // In row
        bool inCol = inDraggableLCol || inDraggableRCol; // In column
        bool inCorner = inDraggableTL || inDraggableTR || inDraggableBR || inDraggableBL; // In corner
        bool inEdge = inDraggableLCol || inDraggableTRow || inDraggableRCol || inDraggableBRow; // In edge
        bool inDraggable = inEdge;

        int x = inRight ? xmaxDragInner : xminDragOuter; // Left corner of the hover element
        int y = inBottom ? ymaxDragInner : yminDragOuter; // Top corner of the hover element
        int w = inCorner ? panelDraggableWidth : (inRow ? (xmaxDragOuter - xminDragOuter) : panelDraggableWidth); // Width of the hover element
        int h = inCorner ? panelDraggableWidth : (inCol ? (ymaxDragOuter - yminDragOuter) : panelDraggableWidth); // Height of the hover element

        if (inDraggable)
        {
            DrawRectangle(x, y, w, h, draggableHighlight);
        }
    }
}
