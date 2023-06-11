#include "panel.h"

// Inclusive
bool Between(int min, int x, int max)
{
    return min <= x && x <= max;
}

namespace panel
{
    Color background = DARKGRAY;
    Color accent = GRAY;
    Color foreground = WHITE;
    Color draggableHighlight = YELLOW;

    PanelHoverSection CheckPanelCollision(Bounds rect, DraggableEdges draggable, int mousex, int mousey)
    {
        // No more is needed if there's no hover highlighting
        if (!draggable.left && !draggable.top && !draggable.right && !draggable.bottom)
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

        int xminDragOuter = rect.xmin; // Outer Left   (ol)
        int xmaxDragOuter = rect.xmax; // Outer Right  (or)
        int yminDragOuter = rect.ymin; // Outer Top    (ot)
        int ymaxDragOuter = rect.ymax; // Outer Bottom (ob)

        int xminDragInner = rect.xmin + panelDraggableWidth; // Inner Left   (il)
        int xmaxDragInner = rect.xmax - panelDraggableWidth; // Inner Right  (ir)
        int yminDragInner = rect.ymin + panelDraggableWidth; // Inner Top    (it)
        int ymaxDragInner = rect.ymax - panelDraggableWidth; // Inner Bottom (ib)

        bool mouseInRow = (xminDragOuter <= mousex && mousex <= xmaxDragOuter); // Mouse is within the horizontal bounds
        bool mouseInCol = (yminDragOuter <= mousey && mousey <= ymaxDragOuter); // Mouse is within the vertical   bounds

        bool mouseInLCol = mouseInCol && (xminDragOuter <= mousex && mousex <= xminDragInner); // Left
        bool mouseInRCol = mouseInCol && (xmaxDragInner <= mousex && mousex <= xmaxDragOuter); // Right
        bool mouseInTRow = mouseInRow && (yminDragOuter <= mousey && mousey <= yminDragInner); // Top
        bool mouseInBRow = mouseInRow && (ymaxDragInner <= mousey && mousey <= ymaxDragOuter); // Bottom

        bool inDraggableLCol = mouseInLCol && draggable.left;   // In left   edge and draggable
        bool inDraggableRCol = mouseInRCol && draggable.right;  // In right  edge and draggable
        bool inDraggableTRow = mouseInTRow && draggable.top;    // In top    edge and draggable
        bool inDraggableBRow = mouseInBRow && draggable.bottom; // In bottom edge and draggable

        bool inDraggableTL = inDraggableLCol && inDraggableTRow; // In top    left  corner and both are draggable
        bool inDraggableBR = inDraggableRCol && inDraggableBRow; // In bottom right corner and both are draggable
        bool inDraggableTR = inDraggableTRow && inDraggableRCol; // In top    right corner and both are draggable
        bool inDraggableBL = inDraggableBRow && inDraggableLCol; // In bottom left  corner and both are draggable

        bool inRight  = inDraggableRCol; // In right
        bool inBottom = inDraggableBRow; // In bottom

        bool inRow = inDraggableTRow || inDraggableBRow; // In row
        bool inCol = inDraggableLCol || inDraggableRCol; // In column

        bool inCorner = inDraggableTL || inDraggableTR || inDraggableBR || inDraggableBL; // In corner
        bool inEdge   = inRow || inCol; // In edge

        bool inDraggable = inEdge;

        int x = inRight  ? xmaxDragInner : xminDragOuter; // Left corner of the hover element
        int y = inBottom ? ymaxDragInner : yminDragOuter; // Top  corner of the hover element

        int w = (inCorner || !inRow) ? (panelDraggableWidth) : (xmaxDragOuter - xminDragOuter); // Width  of the hover element
        int h = (inCorner || !inCol) ? (panelDraggableWidth) : (ymaxDragOuter - yminDragOuter); // Height of the hover element
    }

    void DrawPanel(const char* title, Bounds rect, PanelHoverSection hover)
    {
        // Main panel
        DrawRectangle(rect.xmin, rect.ymin, rect.xmax - rect.xmin, rect.ymax - rect.ymin, accent);
        constexpr int borderInset = borderWidth * 2;
        DrawRectangle(rect.xmin + borderWidth, rect.ymin + borderWidth, rect.xmax - rect.xmin - borderInset, rect.ymax - rect.ymin - borderInset, background);

        // Title section
        constexpr int titleBarHeight = titlePaddingY * 2 + titleSize;
        DrawRectangle(rect.xmin, rect.ymin, rect.xmax - rect.xmin, titleBarHeight, accent);
        DrawText(title, rect.xmin + titlePaddingX, rect.ymin + titlePaddingY, titleSize, foreground);

        if (!!(int)hover)
        {
            DrawRectangle(x, y, w, h, draggableHighlight);
            int cursor = MOUSE_CURSOR_RESIZE_ALL;
            if (inCorner)
            {
                cursor = (hover == PanelHoverSection::CornerTL || hover == PanelHoverSection::CornerBR)
                    ? MOUSE_CURSOR_RESIZE_NWSE
                    : MOUSE_CURSOR_RESIZE_NESW;
            }
            else
            {
                cursor = inRow ? MOUSE_CURSOR_RESIZE_NS : MOUSE_CURSOR_RESIZE_EW;
            }
            SetMouseCursor(cursor);
        }
    }
}
