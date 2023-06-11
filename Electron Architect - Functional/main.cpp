#include <raylib.h>
#include <raymath.h>
#include "panel.h"
#include "console.h"

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Electron Architect");
    SetTargetFPS(60);

    panel::Panel testPanel1 = { "Panel 1", { 50, 50, 600, 200 }};
    panel::Panel testPanel2 = { "Panel 2", { 100, 250, 300, 500 }};
    panel::Panel* panels[] = { &testPanel1, &testPanel2 };

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
                panel::PanelHover panelHover = panel::CheckPanelCollision(currentPanel->bounds, panel::DraggableEdges::All, mouseCurrX, mouseCurrY);
                if (panelHover)
                {
                    currentlyDragging = currentPanel;
                    draggingInfo = panelHover;

                    int xBound = panel::HasLeft(panelHover.identity) ? currentPanel->bounds.xmin : currentPanel->bounds.xmax;
                    int yBound = panel::HasTop (panelHover.identity) ? currentPanel->bounds.ymin : currentPanel->bounds.ymax;
                    mouseOffsX = xBound - mouseCurrX;
                    mouseOffsY = yBound - mouseCurrY;

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

        BeginDrawing(); {

            ClearBackground(BLACK);

            SetMouseCursor(MouseCursor::MOUSE_CURSOR_DEFAULT);

            for (panel::Panel* currentPanel : panels)
            {
                panel::DrawPanel(currentPanel);
            }

            if (currentlyDragging)
            {
                panel::DrawPanelDragElement(currentlyDragging->bounds, draggingInfo);
            }

        } EndDrawing();

        mousePrevX = mouseCurrX;
        mousePrevY = mouseCurrY;
    }

    CloseWindow();

	return 0;
}