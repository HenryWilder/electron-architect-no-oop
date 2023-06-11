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

        int outerL = rect.xmin;
        int outerR = rect.xmax;
        int outerT = rect.ymin;
        int outerB = rect.ymax;

        if (mousex < outerL || outerR < mousex ||
            mousey < outerT || outerB < mousey)
        {
            return PanelHover();
        }

        int innerL = outerL + panelDraggableWidth;
        int innerR = outerR - panelDraggableWidth;
        int innerT = outerT + panelDraggableWidth;
        int innerB = outerB - panelDraggableWidth;

        bool canDragL = HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeL);
        bool canDragR = HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeR);
        bool canDragT = HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeT);
        bool canDragB = HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeB);

        bool inDraggableL = canDragL && outerL <= mousex && mousex <= innerL;
        bool inDraggableR = canDragR && innerR <= mousex && mousex <= outerR;
        bool inDraggableT = canDragT && outerT <= mousey && mousey <= innerT;
        bool inDraggableB = canDragB && innerB <= mousey && mousey <= outerB;

        bool touchesDraggableL = inDraggableL || inDraggableT || inDraggableB;
        bool touchesDraggableR = inDraggableR || inDraggableT || inDraggableB;

        HoverSection identity = HoverSection::None;
        Bounds bounds{};
        bounds.xmin = touchesDraggableL ? outerL : innerR;
        bounds.xmax = touchesDraggableR ? outerR : innerL;
        bounds.ymin = inDraggableB ? innerB : outerT;
        bounds.ymax = inDraggableT ? innerT : outerB;

        // Left
        if (inDraggableL)
        {
            if (inDraggableT)
                identity = HoverSection::CornerTL;
            else if (inDraggableB)
                identity = HoverSection::CornerBL;
            else
                identity = HoverSection::EdgeL;
        }
        // Right
        else if (inDraggableR)
        {
            if (inDraggableT)
                identity = HoverSection::CornerTR;
            else if (inDraggableB)
                identity = HoverSection::CornerBR;
            else
                identity = HoverSection::EdgeR;
        }
        // Top or bottom
        else if (inDraggableT || inDraggableB)
        {
            if (inDraggableT)
                identity = HoverSection::EdgeT;
            else // (inDraggableB)
                identity = HoverSection::EdgeB;
        }

        return PanelHover(identity, bounds);
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
