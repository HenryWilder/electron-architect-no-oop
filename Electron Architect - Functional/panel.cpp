#include "panel.hpp"

namespace panel
{
    Color background = { 31,31,32, 255 };
    Color accent = { 65,65,66, 255 };
    Color foreground = { 245,245,245, 255 };
    Color draggableHighlight = { 255,255,0, 255 };

    Bounds windowBounds = { 0, 0, 0, 0 };

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

    void DrawPanelBackground(const Panel* panel)
    {
        Bounds rect = panel->bounds;
        int x { rect.xmin },
            y { rect.ymin };
        int w { rect.xmax - x },
            h { rect.ymax - y };
        DrawRectangle(x, y, w, h, accent);

        int borderInset = borderWidth * 2;

        // Main panel
        DrawRectangle(x + borderWidth, y + borderWidth, w - borderInset, h - borderInset, background);
    }

    void DrawPanelForeground(const Panel* panel)
    {
        Bounds rect = panel->bounds;
        int x{ rect.xmin },
            y{ rect.ymin };
        int w{ rect.xmax - x },
            h{ rect.ymax - y };

        BeginScissorMode(x, y, w, h);

        // Shadow
        if (BeginPanelScissor(panel))
        {
            BeginBlendMode(BLEND_MULTIPLIED);

            const Color transparent = { 0,0,0, 0 };
            const Color mainShadow = { 0,0,0, 64 };
            const Color ambiShadow = { 0,0,0, 32 };

            int mainShadowSize{ 4 },
                ambiShadowSize{ 16 };

            // Top
            DrawRectangleGradientV(x, y + panelTitlebarHeight, w, ambiShadowSize, ambiShadow, transparent);
            DrawRectangleGradientV(x, y + panelTitlebarHeight, w, mainShadowSize, mainShadow, transparent);

            // Bottom
            DrawRectangleGradientV(x, y + h - ambiShadowSize, w, ambiShadowSize, transparent, ambiShadow);
            DrawRectangleGradientV(x, y + h - mainShadowSize, w, mainShadowSize, transparent, mainShadow);

            // Left
            DrawRectangleGradientH(x, y + panelTitlebarHeight, ambiShadowSize, h, ambiShadow, transparent);
            DrawRectangleGradientH(x, y + panelTitlebarHeight, mainShadowSize, h, mainShadow, transparent);

            // Right
            DrawRectangleGradientH(x + w - ambiShadowSize, y + panelTitlebarHeight, ambiShadowSize, h, transparent, ambiShadow);
            DrawRectangleGradientH(x + w - mainShadowSize, y + panelTitlebarHeight, mainShadowSize, h, transparent, mainShadow);

            EndBlendMode();
        } EndPanelScissor();

        // Title section
        DrawRectangle(x, y, w, panelTitlebarHeight, accent);
        DrawText(panel->title, x + titlePaddingX, y + titlePaddingY, titleSize, foreground);

        EndScissorMode();
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

    void BoundsToRect(Rect& rect, const Bounds& bounds)
    {
        int x = bounds.xmin;
        int y = bounds.ymin;
        rect.x = x;
        rect.y = y;
        rect.w = bounds.xmax - x;
        rect.h = bounds.ymax - y;
    }

    void BoundsFromRect(Bounds& bounds, const Rect& rect)
    {
        int x = rect.x;
        int y = rect.y;
        bounds.xmin = x;
        bounds.ymin = y;
        bounds.xmax = x + rect.w;
        bounds.ymax = y + rect.h;
    }

    // The only real reason this requires a panel and not just bounds is to ensure that it is being used properly.
    // I will change it in the future if I find need for it to be done differently.
    Bounds PanelClientBounds(const Panel& panel)
    {
        Bounds clientBounds;
        clientBounds.xmin = panel.bounds.xmin + borderWidth + 1;
        clientBounds.ymin = panel.bounds.ymin + panelTitlebarHeight;
        clientBounds.xmax = panel.bounds.xmax - borderWidth + 1;
        clientBounds.ymax = panel.bounds.ymax - borderWidth + 1;
        return clientBounds;
    }

    bool BeginPanelScissor(const Panel* panel)
    {
        Bounds rect = panel->bounds;
        int x { rect.xmin + borderWidth },
            y { rect.ymin + titleSize };
        int w { rect.xmax - x - borderWidth }, 
            h { rect.ymax - y - borderWidth };

        BeginScissorMode(x, y, w, h);

        return w > 0 && h > 0;
    }

    void EndPanelScissor()
    {
        EndScissorMode();
    }
}
