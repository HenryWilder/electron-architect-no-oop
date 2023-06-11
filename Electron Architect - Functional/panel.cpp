#include "panel.h"

namespace panel
{
    Color background = DARKGRAY;
    Color accent = GRAY;
    Color foreground = WHITE;
    Color draggableHighlight = YELLOW;

    Bounds windowBounds = { 0, 0, 1280, 720 };

    PanelHover CheckPanelCollision(const Bounds& rect, DraggableEdges draggable, int mousex, int mousey)
    {
        PanelHover result = PanelHover();

        // Return if no hovers are allowed. Why did you even call this, then?
        if (draggable == DraggableEdges::None) return result;

        int outerL{ rect.xmin }, outerR{ rect.xmax },
            outerT{ rect.ymin }, outerB{ rect.ymax };

        bool isOutOfBounds =
            mousex < outerL || outerR < mousex ||
            mousey < outerT || outerB < mousey;

        // Return if mouse is outside the panel
        if (isOutOfBounds) return result;

        int innerL{ outerL + panelDraggableWidth }, innerR{ outerR - panelDraggableWidth },
            innerT{ outerT + panelDraggableWidth }, innerB{ outerB - panelDraggableWidth };

        bool isLDraggable{ ((int)draggable & (int)DraggableEdges::EdgeL) == (int)DraggableEdges::EdgeL },
             isRDraggable{ ((int)draggable & (int)DraggableEdges::EdgeR) == (int)DraggableEdges::EdgeR },
             isTDraggable{ ((int)draggable & (int)DraggableEdges::EdgeT) == (int)DraggableEdges::EdgeT },
             isBDraggable{ ((int)draggable & (int)DraggableEdges::EdgeB) == (int)DraggableEdges::EdgeB };

        // We already know mouse is within outer bounds, so we only need to test inner bounds here.

        bool isInDraggableL{ isLDraggable && mousex <= innerL }, isInDraggableR{ isRDraggable && innerR <= mousex },
             isInDraggableT{ isTDraggable && mousey <= innerT }, isInDraggableB{ isBDraggable && innerB <= mousey };

        bool isInDraggableRow{ isInDraggableT || isInDraggableB };

        // Left
        if (isInDraggableL)
        {
                 if (isInDraggableT) result.identity = HoverSection::CornerTL;
            else if (isInDraggableB) result.identity = HoverSection::CornerBL;
            else                     result.identity = HoverSection::EdgeL;
        }
        // Right
        else if (isInDraggableR)
        {
                 if (isInDraggableT) result.identity = HoverSection::CornerTR;
            else if (isInDraggableB) result.identity = HoverSection::CornerBR;
            else                     result.identity = HoverSection::EdgeR;
        }
        // Top or bottom - but NOT left nor right
        else if (isInDraggableRow)
        {
                 if (isInDraggableT) result.identity = HoverSection::EdgeT;
            else if (isInDraggableB) result.identity = HoverSection::EdgeB;
            // isInDraggableB is implied by isInDraggableRow, but I still want it to be explicit.
            // The compiler *should* optimize the redundant "if" away. Probably.
        }

        // Return if none were satisfied
        if (result.identity == HoverSection::None) return PanelHover();

        bool isTouchingDraggableL{ isInDraggableL || isInDraggableRow },
             isTouchingDraggableR{ isInDraggableR || isInDraggableRow };

        result.bounds.xmin = isTouchingDraggableL ? outerL : innerR;
        result.bounds.xmax = isTouchingDraggableR ? outerR : innerL;
        result.bounds.ymin =       isInDraggableB ? innerB : outerT;
        result.bounds.ymax =       isInDraggableT ? innerT : outerB;

        return result;
    }

    void DrawPanel(const Panel* panel)
    {
        // Main panel
        DrawRectangle(panel->bounds.xmin, panel->bounds.ymin, panel->bounds.xmax - panel->bounds.xmin, panel->bounds.ymax - panel->bounds.ymin, accent);
        constexpr int borderInset = borderWidth * 2;
        DrawRectangle(panel->bounds.xmin + borderWidth, panel->bounds.ymin + borderWidth, panel->bounds.xmax - panel->bounds.xmin - borderInset, panel->bounds.ymax - panel->bounds.ymin - borderInset, background);

        // Title section
        constexpr int titleBarHeight = titlePaddingY * 2 + titleSize;
        DrawRectangle(panel->bounds.xmin, panel->bounds.ymin, panel->bounds.xmax - panel->bounds.xmin, titleBarHeight, accent);
        DrawText(panel->title, panel->bounds.xmin + titlePaddingX, panel->bounds.ymin + titlePaddingY, titleSize, foreground);
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
