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
        PanelHover result = PanelHover();

        // Return if no hovers are allowed. Why did you even call this, then?
        if (draggable == DraggableEdges::None) return result;

        int outerL{ rect.xmin }, outerR{ rect.xmax },
            outerT{ rect.ymin }, outerB{ rect.ymax };

        bool outOfBounds =
            mousex < outerL || outerR < mousex ||
            mousey < outerT || outerB < mousey;

        // Return if mouse is outside the panel
        if (outOfBounds) return result;

        int innerL{ outerL + panelDraggableWidth }, innerR{ outerR - panelDraggableWidth },
            innerT{ outerT + panelDraggableWidth }, innerB{ outerB - panelDraggableWidth };

        bool canDragL{ HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeL) }, canDragR{ HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeR) },
             canDragT{ HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeT) }, canDragB{ HasDraggableEdgeFlag(draggable, DraggableEdges::EdgeB) };

        bool inDraggableL{ canDragL && outerL <= mousex && mousex <= innerL }, inDraggableR{ canDragR && innerR <= mousex && mousex <= outerR },
             inDraggableT{ canDragT && outerT <= mousey && mousey <= innerT }, inDraggableB{ canDragB && innerB <= mousey && mousey <= outerB };

        bool inDraggableRow{ inDraggableT || inDraggableB };

        bool touchesDraggableL{ inDraggableL || inDraggableRow },
             touchesDraggableR{ inDraggableR || inDraggableRow };

        // Left
        if (inDraggableL)
        {
                 if (inDraggableT) result.identity = HoverSection::CornerTL;
            else if (inDraggableB) result.identity = HoverSection::CornerBL;
            else                   result.identity = HoverSection::EdgeL;
        }
        // Right
        else if (inDraggableR)
        {
                 if (inDraggableT) result.identity = HoverSection::CornerTR;
            else if (inDraggableB) result.identity = HoverSection::CornerBR;
            else                   result.identity = HoverSection::EdgeR;
        }
        // Top or bottom - but NOT left nor right
        else if (inDraggableRow)
        {
                 if (inDraggableT) result.identity = HoverSection::EdgeT;
            else if (inDraggableB) result.identity = HoverSection::EdgeB;
            // inDraggableB is implied by inDraggableRow, but I still want it to be explicit.
            // The compiler *should* optimize the redundant "if" away. Probably.
        }

        // Return if none were satisfied
        if (result.identity == HoverSection::None) return PanelHover();

        result.bounds.xmin = touchesDraggableL ? outerL : innerR;
        result.bounds.xmax = touchesDraggableR ? outerR : innerL;
        result.bounds.ymin = inDraggableB ? innerB : outerT;
        result.bounds.ymax = inDraggableT ? innerT : outerB;

        return result;
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
