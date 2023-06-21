#include <raylib.h>
#include <raymath.h>
#include "panel.hpp"
#include "console.hpp"
#include "properties.hpp"
#include "tools.hpp"
#include "graph.hpp"

int ClampInt(int x, int min, int max)
{
    return ((x > max) ? (max) : ((x < min) ? (min) : (x)));
}

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    panel::windowBounds.xmax = &windowWidth;
    panel::windowBounds.ymax = &windowHeight;

    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetTargetFPS(60);

    panel::Panel* panels[] = {
        &properties::propertiesPanel,
        &tools::toolsPanel,
        &graph::graphPanel,
        &console::consolePanel,
    };

    console::CalculateDisplayableLogCount(); // Call at beginning so that panels don't need to move to "activate" the console

    int testNumber = 5;

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

    properties::Add("Int 1", "55234");
    properties::Add("Long long 1", "23542346434534");
    properties::Add("Float 1", "243.34782");
    properties::Add("Double 1", "453.93332384");
    properties::AddHeader("Collection 1");
    properties::Add("Int 2", "56435");
    properties::Add("Long long 2", "67574563653463");
    properties::Add("Float 2", "453.63534");
    properties::Add("Double 2", "343423.4578975757364563");
    properties::AddCloser();
    properties::AddHeader("Collection 2");
    properties::Add("Int 3", "48325");
    properties::Add("Long long 3", "6372795382473892");
    properties::Add("Float 3", "534.34782");
    properties::Add("Double 3", "23462.74567246573456");
    properties::AddHeader("Collection 2.1");
    properties::Add("Hello", ":3");
    properties::Addf("Formatted", "0x%06x", 543634);
    properties::Add("Multiline A", "Apple\nOrange\nBanana");
    properties::AddMultiline("Multiline B", "Apple\nOrange\nBanana");
    properties::AddLinkedInt("Linked", "%i", &testNumber);
    properties::AddCloser();
    properties::AddCloser();
#endif

    // Moves the specified panel to the front of the draw order - which is the back of the array.
    auto shiftToFront = [&panels](panel::Panel* panel) {
        constexpr size_t numPanels = sizeof(panels) / sizeof(panel::Panel*);
        size_t i = 0;
        // Locate panel
        for (; i < numPanels; ++i)
        {
            if (panels[i] == panel)
            {
                break;
            }
        }
        // Shift following panels
        for (++i; i < numPanels; ++i)
        {
            panels[i - 1] = panels[i];
        }
        panels[numPanels - 1] = panel;
    };

    panel::Panel* currentlyDragging = nullptr;
    panel::PanelHover draggingInfo = panel::PanelHover();

    // Offset from mouse when dragging - Set when drag begins
    int mouseOffsX{ }, mouseOffsY{ };

    int mousePrevX{ }, mousePrevY{ };

    while (!WindowShouldClose())
    {
        // Whether or not the mouse is currently doing something that should prevent hover effects/interactions from occurring
        // Todo: Make this more robust with prioritization and interruption
        bool hoverDisabled = false;

        int mouseCurrX{ GetMouseX() }, mouseCurrY{ GetMouseY() };
        int mouseDltaX{ mouseCurrX - mousePrevX }, mouseDltaY{ mouseCurrY - mousePrevY };

        testNumber = GetRandomValue(0,9999);

        if (currentlyDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            currentlyDragging = nullptr;
        }

        if (!currentlyDragging && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            for (panel::Panel* currentPanel : panels)
            {
                panel::PanelHover panelHover = panel::CheckPanelCollision(currentPanel->bounds, currentPanel->draggable, mouseCurrX, mouseCurrY);
                if (panelHover)
                {
                    currentlyDragging = currentPanel;
                    draggingInfo = panelHover;

                    int xBound = panel::HasLeft(panelHover.identity) ? currentPanel->bounds.xmin : currentPanel->bounds.xmax;
                    int yBound = panel::HasTop (panelHover.identity) ? currentPanel->bounds.ymin : currentPanel->bounds.ymax;
                    mouseOffsX = xBound - mouseCurrX;
                    mouseOffsY = yBound - mouseCurrY;

                    shiftToFront(currentPanel);

                    break;
                }
            }
        }

        hoverDisabled = !!currentlyDragging;

        if (currentlyDragging)
        {
            // Left
            if (panel::HasLeft(draggingInfo.identity))
            {
                int x = ClampInt(mouseCurrX + mouseOffsX, 0, currentlyDragging->bounds.xmax - panel::minWidth);
                draggingInfo.bounds.xmax = (draggingInfo.bounds.xmin = currentlyDragging->bounds.xmin = x) + panel::panelDraggableWidth;
            }
            // Right
            else if (panel::HasRight(draggingInfo.identity))
            {
                int x = ClampInt(mouseCurrX + mouseOffsX, currentlyDragging->bounds.xmin + panel::minWidth, windowWidth);
                draggingInfo.bounds.xmin = (draggingInfo.bounds.xmax = currentlyDragging->bounds.xmax = x) - panel::panelDraggableWidth;
            }
            // Top
            if (panel::HasTop(draggingInfo.identity))
            {
                int y = ClampInt(mouseCurrY + mouseOffsY, 0, currentlyDragging->bounds.ymax - panel::minHeight);
                draggingInfo.bounds.ymax = (draggingInfo.bounds.ymin = currentlyDragging->bounds.ymin = y) + panel::panelDraggableWidth;
            }
            // Bottom
            else if (panel::HasBottom(draggingInfo.identity))
            {
                int y = ClampInt(mouseCurrY + mouseOffsY, currentlyDragging->bounds.ymin + panel::minHeight, windowHeight);
                draggingInfo.bounds.ymin = (draggingInfo.bounds.ymax = currentlyDragging->bounds.ymax = y) - panel::panelDraggableWidth;
            }

            console::CalculateDisplayableLogCount(); // Call once per tick, while panels move
        }

        BeginDrawing();

        ClearBackground(BLACK);

        for (panel::Panel* currentPanel : panels)
        {
            panel::DrawPanelBackground(currentPanel);

            if (panel::BeginPanelScissor(currentPanel))
            {
                if (currentPanel == &console::consolePanel)
                {
                    console::DrawPanelContents(mouseCurrX, mouseCurrY, !hoverDisabled);
                }
                else if (currentPanel == &properties::propertiesPanel)
                {
                    properties::DrawPanelContents();
                }
                else if (currentPanel == &graph::graphPanel)
                {
                    graph::DrawPanelContents();
                }
                else if (currentPanel == &tools::toolsPanel)
                {
                    tools::DrawPanelContents();
                }

            } panel::EndPanelScissor();

            panel::DrawPanelForeground(currentPanel);
        }

        if (currentlyDragging)
        {
            panel::DrawPanelDragElement(currentlyDragging->bounds, draggingInfo);
        }

        EndDrawing();

        mousePrevX = mouseCurrX;
        mousePrevY = mouseCurrY;
    }

    CloseWindow();

    properties::Clear();
    console::Clear();

	return 0;
}