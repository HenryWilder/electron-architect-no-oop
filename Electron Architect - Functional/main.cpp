#include <raylib.h>
#include <raymath.h>
#include "panel.h"
#include "console.h"
#include "properties.h"
#include "tools.h"
#include "graph.h"

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

    console::Log("Test log 1");
    console::Warn("Test warning 1");
    console::Error("Test error 1");
    console::Group("Test group 1");
    console::Log("Test log 2");
    console::Warn("Test warning 2");
    console::Error("Test error 2");
    console::GroupEnd();

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
        int mouseCurrX{ GetMouseX() }, mouseCurrY{ GetMouseY() };
        int mouseDltaX{ mouseCurrX - mousePrevX }, mouseDltaY{ mouseCurrY - mousePrevY };

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

        if (currentlyDragging)
        {
            // Left
            if (panel::HasLeft(draggingInfo.identity))
            {
                int x = Clamp(mouseCurrX + mouseOffsX, 0, currentlyDragging->bounds.xmax - panel::minWidth);
                draggingInfo.bounds.xmax = (draggingInfo.bounds.xmin = currentlyDragging->bounds.xmin = x) + panel::panelDraggableWidth;
            }
            // Right
            else if (panel::HasRight(draggingInfo.identity))
            {
                int x = Clamp(mouseCurrX + mouseOffsX, currentlyDragging->bounds.xmin + panel::minWidth, windowWidth);
                draggingInfo.bounds.xmin = (draggingInfo.bounds.xmax = currentlyDragging->bounds.xmax = x) - panel::panelDraggableWidth;
            }
            // Top
            if (panel::HasTop(draggingInfo.identity))
            {
                int y = Clamp(mouseCurrY + mouseOffsY, 0, currentlyDragging->bounds.ymax - panel::minHeight);
                draggingInfo.bounds.ymax = (draggingInfo.bounds.ymin = currentlyDragging->bounds.ymin = y) + panel::panelDraggableWidth;
            }
            // Bottom
            else if (panel::HasBottom(draggingInfo.identity))
            {
                int y = Clamp(mouseCurrY + mouseOffsY, currentlyDragging->bounds.ymin + panel::minHeight, windowHeight);
                draggingInfo.bounds.ymin = (draggingInfo.bounds.ymax = currentlyDragging->bounds.ymax = y) - panel::panelDraggableWidth;
            }
        }

        BeginDrawing();

        ClearBackground(BLACK);

        for (panel::Panel* currentPanel : panels)
        {
            panel::DrawPanelBackground(currentPanel);

            panel::BeginPanelScissor(currentPanel);
            
            if (currentPanel == &console::consolePanel)
            {
                console::DrawPanelContents();
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

            panel::EndPanelScissor();

            panel::DrawPanelTitlebar(currentPanel);
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

	return 0;
}