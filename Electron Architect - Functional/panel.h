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
    constexpr int panelDraggableWidth = 6; // Size of the draggable section of a panel edge in pixels (goes inward from edge)

    constexpr int panelTitlebarHeight = titlePaddingY * 2 + titleSize; // The height of the titlebar section of a panel

    constexpr int minWidth= 60; // Minimum width of a standard panel in pixels
    constexpr int minHeight = 30; // Minimum height of a standard panel in pixels

    // int | int*
    struct IntOrIntPtr
    {
        IntOrIntPtr()
            : tag{ Tag::Val }, ref{ } { }

        IntOrIntPtr(int  x)
            : tag{ Tag::Val }, ref{ } { val = x; }

        IntOrIntPtr(int *x)
            : tag{ Tag::Ref }, ref{ x } { }

        enum class Tag { Val, Ref } tag;

        union { int val; int *ref; };

        inline bool IsVal() const { return tag == Tag::Val; }
        inline bool IsRef() const { return tag == Tag::Ref; }

        // Returns value whether reference or non-reference
        inline operator int() const { return IsRef() ? *ref : val; }

        // Replaces the reference
        inline void SetRef(IntOrIntPtr* newRef)
        {
            tag = Tag::Ref;
            ref = newRef->IsRef() ? newRef->ref : &newRef->val;
        }

        // Replaces the reference
        inline void SetRef(int* newRef)
        {
            tag = Tag::Ref;
            ref = newRef;
        }

        // Sets the value
        inline IntOrIntPtr& operator=(int newVal)
        {
            if (IsRef())
                *ref = newVal;
            else
                val = newVal;
            return *this;
        }

        // Sets the value to match - Use SetRef to replace the reference
        inline IntOrIntPtr& operator=(IntOrIntPtr newVal)
        {
            int _newVal = (int)newVal;
            return *this = _newVal;
        }
    };

    // Panel bounds - Useable for non-referencing bounds
    struct Bounds
    {
        IntOrIntPtr xmin, ymin, xmax, ymax;
    };

    // Can be referenced to keep RefBounds synced with window
    Bounds windowBounds =  { };
    
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

    struct Panel
    {
        const char* title;
        Bounds bounds;
        DraggableEdges draggable;
    };

    // Uses bitflags
    // Flags are contextual to other flags
    enum class HoverSection
    {
        // 0000 - There is not a hover
        // Behavior is undefined if any other flags are set while the "none" bit (0___) is set.
        None = 0,

        // 1___ - There is a hover
        // This distinguishes between what would otherwise be `000` (none) vs `000` (left edge)
        Any  = 8,

        // _111 - Helper
        // Use with & to get information without the "Any"-flag
        _Info = 7,

        Edge   = Any | 0, // 10__ - Hover is exactly one edge
        Corner = Any | 4, // 11__ - Hover is a combination of exactly two edges

        // _1__ - Helper
        // Use with & and Edge or Corner to test which one the hover is
        _Type = 4,

        // __11 - Helper
        // Use with & to get edge index:
        // 0: Left
        // 1: Right
        // 2: Top
        // 3: Bottom
        _Edge = 3,

        EdgeCol = Edge | 0, // 100_ - For edges - Hover contains vertical (left or right) edge
        EdgeRow = Edge | 2, // 101_ - For edges - Hover contains horizontal (top or bottom) edge

        // __1_ - Helper
        // Use with & and EdgeCol or EdgeRow to test whether hover is on a row or column
        _EdgeRowOrCol = 2,

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

        // 11_1 - Helper
        // Use with & and CornerL or CornerR to test which one the hover might be
        _CornerHorizontal = Corner | 1,

        CornerL = Corner | 0, // 11_0 - For corners - Hover contains left edge
        CornerR = Corner | 1, // 11_1 - For corners - Hover contains right edge

        // 111_ - Helper
        // Use with & and CornerT or CornerB to test which one the hover might be
        _CornerVertical = Corner | 2,

        CornerT = Corner | 0, // 110_ - For corners - Hover contains top edge
        CornerB = Corner | 2, // 111_ - For corners - Hover contains bottom edge

        CornerTL = Corner | CornerT | CornerL, // 1100 - Top left corner
        CornerTR = Corner | CornerT | CornerR, // 1101 - Top right corner
        CornerBL = Corner | CornerB | CornerL, // 1110 - Bottom left corner
        CornerBR = Corner | CornerB | CornerR, // 1111 - Bottom right corner
    };

    // Might be an edge or a corner, but contains a left edge
    inline bool HasLeft(const HoverSection section)
    {
        return section == HoverSection::EdgeL || ((int)section & (int)HoverSection::_CornerHorizontal) == (int)HoverSection::CornerL;
    }

    // Might be an edge or a corner, but contains a left edge
    inline bool HasRight(const HoverSection section)
    {
        return section == HoverSection::EdgeR || ((int)section & (int)HoverSection::_CornerHorizontal) == (int)HoverSection::CornerR;
    }

    // Might be an edge or a corner, but contains a left edge
    inline bool HasTop(const HoverSection section)
    {
        return section == HoverSection::EdgeT || ((int)section & (int)HoverSection::_CornerVertical) == (int)HoverSection::CornerT;
    }

    // Might be an edge or a corner, but contains a left edge
    inline bool HasBottom(const HoverSection section)
    {
        return section == HoverSection::EdgeB || ((int)section & (int)HoverSection::_CornerVertical) == (int)HoverSection::CornerB;
    }

    struct PanelHover
    {
        PanelHover() : identity(HoverSection::None), bounds() {}
        PanelHover(HoverSection identity, Bounds bounds) : identity(identity), bounds(bounds) {}

        HoverSection identity;
        Bounds bounds; // Bounds of specifically the hover element - Will be garbage data if identity says there is no hover

        inline operator HoverSection() const
        {
            return identity;
        }

        inline operator bool() const
        {
            return (bool)((int)identity & (int)HoverSection::Any);
        }
    };

    // Checks whether the mouse is hovering a draggable part of the panel
    PanelHover CheckPanelCollision(const Bounds& rect, DraggableEdges draggable, int mousex, int mousey);

    // Draws a panel with a title bar
    void DrawPanel(const Panel* panel);

    // Draws the dragging element for a panel
    void DrawPanelDragElement(Bounds rect, const PanelHover& hover);
}
