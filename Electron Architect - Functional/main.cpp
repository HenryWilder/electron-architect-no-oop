#include <thread>
#include <raylib.h>
#include <raymath.h>
#include "panel.hpp"
#include "console.hpp"
#include "properties.hpp"
#include "tools.hpp"
#include "graph.hpp"

int ClampInt(int x, int min, int max);
template<size_t NUM_PANELS> void ShiftToFront(panel::Panel* panels[NUM_PANELS], panel::Panel* panel);

#pragma region // "using" longwinded names with namespace already in name

using panel::Panel;
using panel::PanelID;
using panel::Bounds;
using panel::PanelHover;
using panel::windowBounds;

using properties::propertiesPanel;
using properties::PropValueType;

using tools::toolsPanel;
using graph::graphPanel;
using console::consolePanel;

#pragma endregion

int main()
{
#pragma region // Pre-loop

    int windowWidth = 1280;
    int windowHeight = 720;
    panel::windowBounds.xmax = &windowWidth;
    panel::windowBounds.ymax = &windowHeight;

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetTargetFPS(60);

    Panel* panels[] = {
        &propertiesPanel,
        &toolsPanel,
        &graphPanel,
        &consolePanel,
    };
    constexpr size_t NUM_PANELS = sizeof(panels) / sizeof(panel::Panel*);

    console::CalculateDisplayableLogCount(); // Call at beginning so that panels don't need to move to "activate" the console

    int testNumber = 5;
    const char* testString = "apple";

#if _DEBUG // Testing
    console::Log("Test log 1");
    console::Warn("Test warning 1");
    console::Error("Test error 1");
    console::Group("Test group 1");
    console::Log("Test log 2");
    console::Warn("Test warning 2");
    console::Error("Test error 2");
    console::GroupEnd();
    console::Assert(false, "Hello");
    console::Assert(false, "Hello");

    properties::AddInt("Int 1", 55234);
    properties::AddInt("Long long 1", 23542346434534ll);
    properties::AddFloat("Float 1", 243.34782f);
    properties::AddFloat("Double 1", 453.93332384);
    properties::AddBool("Boolean 1", true);
    properties::AddBool("Boolean 2", false);
    properties::AddHeader("Collection 1"); {
        properties::AddInt("Int 2", 56435);
        properties::AddInt("Long long 2", 67574563653463ll);
        properties::AddFloat("Float 2", 453.63534f);
        properties::AddFloat("Double 2", 343423.4578975757364563);
    } properties::AddCloser();
    properties::AddHeader("Collection 2"); {
        properties::AddInt("Int 3", 48325);
        properties::AddInt("Long long 3", 6372795382473892ll);
        properties::AddFloat("Float 3", 534.34782f);
        properties::AddFloat("Double 3", 23462.74567246573456);
        properties::AddHeader("Collection 2.1"); {
            properties::AddString("Hello", ":3");
            properties::Addf_hint("Formatted", 9, PropValueType::Int, "0x%06x", 543634);
            properties::Addf_hint("Overflow hint", 6, PropValueType::Int, "0x%09x", 1);
            properties::AddString("Multiline", "Apple\nOrange\nBanana");
            properties::AddLinkedInt("Linked", "%5i", &testNumber);
            properties::AddLinkedString("Side of screen", "%s", &testString);
        } properties::AddCloser();
    } properties::AddCloser();
#endif

    Panel* currentlyWithin = nullptr;
    Panel* currentlyResizing = nullptr;
    PanelHover draggingInfo = PanelHover();

    // Offset from mouse when dragging - Set when drag begins
    int mouseOffsX{ }, mouseOffsY{ };

    int mousePrevX{ }, mousePrevY{ };

#pragma endregion

#pragma region // Loop

    while (!WindowShouldClose())
    {
        if (IsWindowResized())
        {
            int propertiesWidth = propertiesPanel.bounds.xmax - propertiesPanel.bounds.xmin;
            int consoleHeight   =    consolePanel.bounds.ymax -    consolePanel.bounds.ymin;

            windowBounds.xmax = windowWidth = GetRenderWidth();
            propertiesPanel.bounds.xmin = windowWidth - propertiesWidth;

            windowBounds.ymax = windowHeight = GetRenderHeight();
            consolePanel.bounds.ymin = windowHeight - consoleHeight;
        }

        // Whether or not the mouse is currently doing something that should prevent hover effects/interactions from occurring
        // Todo: Make this more robust with prioritization and interruption
        bool hoverDisabled = false;

        int mouseCurrX{ GetMouseX() }, mouseCurrY{ GetMouseY() };
        int mouseDltaX{ mouseCurrX - mousePrevX }, mouseDltaY{ mouseCurrY - mousePrevY };

#if _DEBUG
        testString = (mouseCurrX < (windowWidth / 2)) ? "Left" : "Right";
        testNumber = GetRandomValue(0,9999);
#endif

#pragma region // Resize panel

        if (!currentlyResizing)
        {
            currentlyWithin = nullptr;
            for (Panel* panel : panels)
            {
                Bounds bounds = panel->bounds;
                if (bounds.xmin <= mouseCurrX && mouseCurrX <= bounds.xmax &&
                    bounds.ymin <= mouseCurrY && mouseCurrY <= bounds.ymax)
                {
                    currentlyWithin = panel;
                }
            }
        }

        if (currentlyResizing && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            currentlyResizing = nullptr;
        }

        if (!currentlyResizing && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && currentlyWithin)
        {
            PanelHover panelHover = panel::CheckPanelCollision(currentlyWithin->bounds, currentlyWithin->draggable, mouseCurrX, mouseCurrY);
            if (panelHover)
            {
                currentlyResizing = currentlyWithin;
                draggingInfo = panelHover;

                int xBound = panel::HasLeft(panelHover.identity) ? currentlyWithin->bounds.xmin : currentlyWithin->bounds.xmax;
                int yBound = panel::HasTop (panelHover.identity) ? currentlyWithin->bounds.ymin : currentlyWithin->bounds.ymax;
                mouseOffsX = xBound - mouseCurrX;
                mouseOffsY = yBound - mouseCurrY;

                ShiftToFront<NUM_PANELS>(panels, currentlyWithin);
            }
        }

        hoverDisabled = !!currentlyResizing;

        // Resize panel
        if (currentlyResizing)
        {
            // For no other reason than to shorten the name
            constexpr int dragWidth = panel::panelDraggableWidth;

            Bounds& resizingBounds = currentlyResizing->bounds;

            bool isDraggingL = panel::HasLeft  (draggingInfo.identity);
            bool isDraggingR = panel::HasRight (draggingInfo.identity);
            bool isDraggingT = panel::HasTop   (draggingInfo.identity);
            bool isDraggingB = panel::HasBottom(draggingInfo.identity);

            bool isDraggingH = isDraggingL || isDraggingR;
            bool isDraggingV = isDraggingT || isDraggingB;

            // Horizontal
            if (isDraggingH)
            {
                int mouseOffsetX = mouseCurrX + mouseOffsX;

                int xmin = isDraggingR ? (int)resizingBounds.xmin : 0;
                int xmax = isDraggingL ? (int)resizingBounds.xmax : windowWidth;
                int x = ClampInt(mouseOffsetX, xmin, xmax);

                int xminNew = isDraggingL ? x : x - dragWidth;
                int xmaxNew = isDraggingR ? x : x + dragWidth;

                draggingInfo.bounds.xmin = xminNew;
                draggingInfo.bounds.xmax = xmaxNew;

                if (isDraggingL) { resizingBounds.xmin = xminNew; } // Left
                else             { resizingBounds.xmax = xmaxNew; } // Right
            }

            // Vertical
            if (isDraggingV)
            {
                int mouseOffsetY = mouseCurrY + mouseOffsY;

                int ymin = isDraggingB ? (int)resizingBounds.ymin : 0;
                int ymax = isDraggingT ? (int)resizingBounds.ymax : windowHeight;
                int y = ClampInt(mouseOffsetY, ymin, ymax);

                int yminNew = isDraggingT ? y : y - dragWidth;
                int ymaxNew = isDraggingB ? y : y + dragWidth;

                draggingInfo.bounds.ymin = yminNew;
                draggingInfo.bounds.ymax = ymaxNew;

                if (isDraggingT) { resizingBounds.ymin = yminNew; } // Top
                else             { resizingBounds.ymax = ymaxNew; } // Bottom
            }

            console::CalculateDisplayableLogCount(); // Call once per tick, while panels move
        }

        // Scroll within properties panel
        if (!hoverDisabled && currentlyWithin == &propertiesPanel)
        {
            float amount = GetMouseWheelMove();
            properties::scrollY -= (int)(amount) * properties::lineHeight;
            if (properties::scrollY < 0)
            {
                properties::scrollY = 0;
            }
        }

#pragma endregion

        BeginDrawing();

        ClearBackground(BLACK);

        for (Panel* currentPanel : panels)
        {
            bool isWithinThisPanel = currentPanel == currentlyWithin;
            bool isHoverNeeded = isWithinThisPanel && !hoverDisabled;

            panel::DrawPanelBackground(currentPanel);

            if (panel::BeginPanelScissor(currentPanel))
            {
                switch (currentPanel->id)
                {
                case PanelID::Console:
                    console::DrawPanelContents(mouseCurrX, mouseCurrY, isHoverNeeded);
                    break;

                case PanelID::Properties:
                    properties::DrawPanelContents(mouseCurrX, mouseCurrY, isHoverNeeded, IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
                    break;

                case PanelID::Graph:
                    graph::DrawPanelContents();
                    break;

                case PanelID::Tools:
                    tools::DrawPanelContents();
                    break;
                }
            } panel::EndPanelScissor();

            panel::DrawPanelForeground(currentPanel);

#if _DEBUG // Debug panel interactability with mouse
            if (!isWithinThisPanel) [[likely]] // Only one panel at a time ever has the mouse within it
            {
                panel::Rect rect;
                panel::BoundsToRect(rect, currentPanel->bounds);
                DrawRectangle(rect.x, rect.y, rect.w, rect.h, { 127,127,127, 127 });
            }
#endif
        }

        if (currentlyResizing)
        {
            panel::DrawPanelDragElement(currentlyResizing->bounds, draggingInfo);
        }

        EndDrawing();

        mousePrevX = mouseCurrX;
        mousePrevY = mouseCurrY;
    }

#pragma endregion

#pragma region // Post-loop

    CloseWindow();

    properties::Clear();
    console::Clear();

	return 0;

#pragma endregion
}

int ClampInt(int x, int min, int max)
{
    return ((x > max) ? (max) : ((x < min) ? (min) : (x)));
}

template<size_t NUM_PANELS> void ShiftToFront(panel::Panel* panels[NUM_PANELS], panel::Panel* panel)
{
    size_t i = 0;

    // Locate panel
    for (; i < NUM_PANELS; ++i)
    {
        if (panels[i] == panel)
        {
            break;
        }
    }

    // Shift following panels
    for (++i; i < NUM_PANELS; ++i)
    {
        panels[i - 1] = panels[i];
    }

    panels[NUM_PANELS - 1] = panel;
}
