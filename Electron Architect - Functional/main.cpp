#include <raylib.h>
#include <raymath.h>
#include "panel.h"
#include "console.h"

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "My Raylib Program");
    SetTargetFPS(60);

    /******************************************
    *   Load textures, shaders, and meshes    *
    ******************************************/

    panel::Panel testPanel1 = { "Panel 1", { 50, 50, 600, 200 }};
    panel::Panel testPanel2 = { "Panel 2", { 100, 250, 300, 500 }};
    panel::Panel* panels[] = { &testPanel1, &testPanel2 };

    panel::Panel* currentlyDragging = nullptr;
    panel::PanelHover draggingInfo = panel::PanelHover();

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables   *
        ******************************************/

        if (currentlyDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            currentlyDragging = nullptr;
        }

        if (currentlyDragging)
        {
            // Left
            if (panel::HasLeft(draggingInfo.identity))
            {
                int x = Clamp(GetMouseX(), 0, currentlyDragging->bounds.xmax - panel::minWidth);
                draggingInfo.bounds.xmax = (draggingInfo.bounds.xmin = currentlyDragging->bounds.xmin = x) + panel::panelDraggableWidth;
            }
            // Right
            else if (panel::HasRight(draggingInfo.identity))
            {
                int x = Clamp(GetMouseX(), currentlyDragging->bounds.xmin + panel::minWidth, windowWidth);
                draggingInfo.bounds.xmin = (draggingInfo.bounds.xmax = currentlyDragging->bounds.xmax = x) - panel::panelDraggableWidth;
            }
            // Top
            if (panel::HasTop(draggingInfo.identity))
            {
                int y = Clamp(GetMouseY(), 0, currentlyDragging->bounds.ymax - panel::minHeight);
                draggingInfo.bounds.ymax = (draggingInfo.bounds.ymin = currentlyDragging->bounds.ymin = y) + panel::panelDraggableWidth;
            }
            // Bottom
            else if (panel::HasBottom(draggingInfo.identity))
            {
                int y = Clamp(GetMouseY(), currentlyDragging->bounds.ymin + panel::minHeight, windowHeight);
                draggingInfo.bounds.ymin = (draggingInfo.bounds.ymax = currentlyDragging->bounds.ymax = y) - panel::panelDraggableWidth;
            }
        }
        else
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                for (panel::Panel* currentPanel : panels)
                {
                    panel::PanelHover panelHover = panel::CheckPanelCollision(currentPanel->bounds, panel::DraggableEdges::All, GetMouseX(), GetMouseY());
                    if (panelHover)
                    {
                        currentlyDragging = currentPanel;
                        draggingInfo = panelHover;
                        break;
                    }
                }
            }
        }

        /******************************************
        *   Draw the frame                        *
        ******************************************/

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
    }

    /******************************************
    *   Unload and free memory                *
    ******************************************/

    // @TODO: Unload variables

    CloseWindow();

	return 0;
}