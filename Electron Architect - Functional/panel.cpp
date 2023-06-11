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

    PanelHover CheckPanelCollision(const Bounds& rect, DraggableEdges draggable, int mousex, int mousey)
    {
        // No hovers are allowed. Why did you even call this then?
        if (draggable == DraggableEdges::None)
        {
            return PanelHover();
        }

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

        bool mouseInRow = Between(xminDragOuter, mousex, xmaxDragOuter); // Mouse is within the horizontal bounds
        bool mouseInCol = Between(yminDragOuter, mousey, ymaxDragOuter); // Mouse is within the vertical   bounds

        bool mouseInLCol = mouseInCol && Between(xminDragOuter, mousex, xminDragInner); // Left
        bool mouseInRCol = mouseInCol && Between(xmaxDragInner, mousex, xmaxDragOuter); // Right
        bool mouseInTRow = mouseInRow && Between(yminDragOuter, mousey, yminDragInner); // Top
        bool mouseInBRow = mouseInRow && Between(ymaxDragInner, mousey, ymaxDragOuter); // Bottom

        bool inDraggableLCol = mouseInLCol && HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeL); // In left   edge and draggable
        bool inDraggableRCol = mouseInRCol && HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeR); // In right  edge and draggable
        bool inDraggableTRow = mouseInTRow && HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeT); // In top    edge and draggable
        bool inDraggableBRow = mouseInBRow && HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeB); // In bottom edge and draggable

        bool inDraggableTL = inDraggableLCol && inDraggableTRow; // In top    left  corner and both are draggable
        bool inDraggableBR = inDraggableRCol && inDraggableBRow; // In bottom right corner and both are draggable
        bool inDraggableTR = inDraggableTRow && inDraggableRCol; // In top    right corner and both are draggable
        bool inDraggableBL = inDraggableBRow && inDraggableLCol; // In bottom left  corner and both are draggable

        bool inRight  = inDraggableRCol; // In right (draggable)
        bool inBottom = inDraggableBRow; // In bottom (draggable)

        bool inRow = inDraggableTRow || inDraggableBRow; // In row (draggable)
        bool inCol = inDraggableLCol || inDraggableRCol; // In column (draggable)

        bool inCorner = inDraggableTL || inDraggableTR || inDraggableBR || inDraggableBL; // In corner (draggable)
        bool inEdge   = inRow || inCol; // In edge (draggable)

        bool inDraggable = inEdge; // In any edge or corner - corners are implied as they are the intersections of edges

        if (!inDraggable)
        {
            return PanelHover(); // Return that there are no hovers
        }

        int x = inRight  ? xmaxDragInner : xminDragOuter; // Left corner of the hover element
        int y = inBottom ? ymaxDragInner : yminDragOuter; // Top  corner of the hover element

        int w = (inCorner || !inRow) ? (panelDraggableWidth) : (xmaxDragOuter - xminDragOuter); // Width  of the hover element
        int h = (inCorner || !inCol) ? (panelDraggableWidth) : (ymaxDragOuter - yminDragOuter); // Height of the hover element

        Bounds bounds{};
        bounds.xmin = x;
        bounds.ymin = y;
        bounds.xmax = x + w;
        bounds.ymax = y + h;

        HoverSection section;
        if (inCorner)
        {
            if (inDraggableTL)
                section = HoverSection::CornerTL;
            else if (inDraggableBR)
                section = HoverSection::CornerBR;
            else if (inDraggableTR)
                section = HoverSection::CornerTR;
            else // (inDraggableBL)
                section = HoverSection::CornerBL;
        }
        else // (inEdge)
        {
            if (inDraggableLCol)
                section = HoverSection::EdgeL;
            else if (inDraggableRCol)
                section = HoverSection::EdgeR;
            else if (inDraggableTRow)
                section = HoverSection::EdgeT;
            else // (inDraggableBRow)
                section = HoverSection::EdgeB;
        }

        return PanelHover(section, bounds);
    }

    void DrawPanel(const char* title, const Bounds& rect)
    {
        // Main panel
        DrawRectangle(rect.xmin, rect.ymin, rect.xmax - rect.xmin, rect.ymax - rect.ymin, accent);
        constexpr int borderInset = borderWidth * 2;
        DrawRectangle(rect.xmin + borderWidth, rect.ymin + borderWidth, rect.xmax - rect.xmin - borderInset, rect.ymax - rect.ymin - borderInset, background);

        // Title section
        constexpr int titleBarHeight = titlePaddingY * 2 + titleSize;
        DrawRectangle(rect.xmin, rect.ymin, rect.xmax - rect.xmin, titleBarHeight, accent);
        DrawText(title, rect.xmin + titlePaddingX, rect.ymin + titlePaddingY, titleSize, foreground);
    }

    void DrawPanelDragElement(Bounds rect, const PanelHover& hover)
    {
        if (hover)
        {
            int x = hover.bounds.xmin;
            int y = hover.bounds.ymin;
            int w = hover.bounds.xmax - x;
            int h = hover.bounds.ymax - y;
            DrawRectangle(x, y, w, h, draggableHighlight);
        }
    }
}
