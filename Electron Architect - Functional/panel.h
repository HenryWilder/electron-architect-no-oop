#pragma once
#include <raylib.h>

// Functions shared by each of the panels.
namespace panel
{
    extern Color background; // Color of standard panel background
    extern Color accent; // Color of standard panel parts separating sections
    extern Color foreground; // Color of standard panel text
    extern Color draggableHighlight; // Color of standard panel text

    constexpr int titlePaddingX = 6; // Horizontal padding of standard panel title in pixels
    constexpr int titlePaddingY = 4; // Vertical padding of standard panel title in pixels
    constexpr int borderWidth = 4; // Width of standard panel border in pixels
    constexpr int titleSize = 16; // Size of title font
    constexpr int panelDraggableWidth = borderWidth; // Size of the draggable section of a panel edge in pixels (goes inward from edge)

    // Panel bounds
    struct Bounds
    {
        int xmin, ymin, xmax, ymax;
    };

    // Uses bitflags
    // Each bit is an independent boolean
    enum class DraggableEdges
    {
        None = 0, // No edges are draggable

        EdgeL = 1, // Left
        EdgeR = 2, // Right
        EdgeT = 4, // Top
        EdgeB = 8, // Bottom

        Horizontal = EdgeL | EdgeR, // Shorthand for the set of left/right edges - Can also be used as a mask
        Vertical   = EdgeT | EdgeB, // Shorthand for the set of top/bottom edges - Can also be used as a mask

        All = Horizontal | Vertical, // Shorthand for the set of all edges
    };

    inline DraggableEdges operator&(DraggableEdges lvalue, DraggableEdges rvalue)
    {
        return (DraggableEdges)((int)lvalue & (int)rvalue);
    }

    inline bool HasDraggableEdgeFlag(DraggableEdges value, DraggableEdges mask)
    {
        return (value & mask) == mask;
    }

    // Uses bitflags
    // Flags are contextual to other flags
    enum class HoverSection
    {
        // 0000 - There is not a hover
        // Behavior is undefined if any other flags are set while the "none" bit is set.
        None = 0,

        // 1___ - There is a hover
        // This distinguishes between what would otherwise be `000` (none) vs `000` (left edge)
        Any  = 8,

        // _111 - Helper
        // Use with & to get information without the "Any"-flag
        _Info = 7,

        Edge   = Any | 0, // 10__ - Hover is exactly one edge
        Corner = Any | 4, // 11__ - Hover is a combination of exactly two edges

        // __11 - Helper
        // Use with & to get edge index:
        // 0: Left
        // 1: Right
        // 2: Top
        // 3: Bottom
        _Edge = 3,

        EdgeCol = Edge | 0, // 100_ - For edges - Hover contains vertical (left or right) edge
        EdgeRow = Edge | 2, // 101_ - For edges - Hover contains horizontal (top or bottom) edge

        EdgeL = EdgeCol | 0, // 1000 - Hover is left edge
        EdgeR = EdgeCol | 1, // 1001 - Hover is right edge
        EdgeT = EdgeRow | 0, // 1010 - Hover is top edge
        EdgeB = EdgeRow | 1, // 1011 - Hover is bottom edge

        // __11 - Helper
        // Use with & to get corner index in Z-order:
        // 0: Top Left
        // 1: Top Right
        // 2: Bottom Left
        // 3: Bottom Right
        _Corner = 3,

        CornerL = Corner | 0, // 11_0 - For corners - Hover contains left edge
        CornerR = Corner | 1, // 11_1 - For corners - Hover contains right edge

        CornerT = Corner | 0, // 110_ - For corners - Hover contains top edge
        CornerB = Corner | 2, // 111_ - For corners - Hover contains bottom edge

        CornerTL = Corner | CornerT | CornerL, // 1100 - Top left corner
        CornerTR = Corner | CornerT | CornerR, // 1101 - Top right corner
        CornerBL = Corner | CornerB | CornerL, // 1110 - Bottom left corner
        CornerBR = Corner | CornerB | CornerR, // 1111 - Bottom right corner
    };

    inline HoverSection operator&(HoverSection lvalue, HoverSection rvalue)
    {
        return (HoverSection)((int)lvalue & (int)rvalue);
    }

    inline HoverSection operator|(HoverSection lvalue, HoverSection rvalue)
    {
        return (HoverSection)((int)lvalue | (int)rvalue);
    }

    inline HoverSection& operator|=(HoverSection& lvalue, HoverSection rvalue)
    {
        return (lvalue = (HoverSection)((int)lvalue | (int)rvalue));
    }

    inline bool HasHoverSectionFlag(HoverSection value, HoverSection mask)
    {
        return (value & mask) == mask;
    }

    struct PanelHover
    {
        PanelHover() : identity(HoverSection::None), bounds() {}
        PanelHover(HoverSection identity, Bounds bounds) : identity(identity), bounds(bounds) {}

        HoverSection identity;
        Bounds bounds; // Will be garbage data if identity says there is no hover

        inline operator HoverSection()
        {
            return identity;
        }

        inline operator bool()
        {
            return (bool)(identity & HoverSection::Any);
        }
    };

    // Draws a panel with a title bar and optional highlighting for draggability
    PanelHover CheckPanelCollision(Bounds rect, DraggableEdges draggable, int mousex, int mousey);

    // Draws a panel with a title bar and optional highlighting for draggability
    void DrawPanel(const char* title, Bounds rect, PanelHover hover = PanelHover());
}
