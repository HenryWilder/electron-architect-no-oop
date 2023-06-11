#include <raylib.h>
#include <raymath.h>
#include "panel.h"
#include "console.h"

int Min(int a, int b)
{
    return b < a ? b : a;
}

int Max(int a, int b)
{
    return b > a ? b : a;
}

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "My Raylib Program");
    SetTargetFPS(60);

    /******************************************
    *   Load textures, shaders, and meshes    *
    ******************************************/

    panel::Bounds testPanel = { 50, 50, 600, 200 };

    panel::Bounds* currentlyDragging = nullptr;
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
            bool draggingLeft =
                draggingInfo.identity == panel::HoverSection::EdgeL ||
                draggingInfo.identity == panel::HoverSection::CornerTL ||
                draggingInfo.identity == panel::HoverSection::CornerBL;

            bool draggingRight =
                draggingInfo.identity == panel::HoverSection::EdgeR ||
                draggingInfo.identity == panel::HoverSection::CornerTR ||
                draggingInfo.identity == panel::HoverSection::CornerBR;

            bool draggingTop =
                draggingInfo.identity == panel::HoverSection::EdgeT ||
                draggingInfo.identity == panel::HoverSection::CornerTL ||
                draggingInfo.identity == panel::HoverSection::CornerTR;

            bool draggingBottom =
                draggingInfo.identity == panel::HoverSection::EdgeB ||
                draggingInfo.identity == panel::HoverSection::CornerBL ||
                draggingInfo.identity == panel::HoverSection::CornerBR;

            // Left
            if (draggingLeft)
            {
                int x = Clamp(GetMouseX(), 0, currentlyDragging->xmax - panel::minWidth);
                draggingInfo.bounds.xmax = (draggingInfo.bounds.xmin = currentlyDragging->xmin = x) + panel::panelDraggableWidth;
            }
            // Right
            else if (draggingRight)
            {
                int x = Clamp(GetMouseX(), currentlyDragging->xmin + panel::minWidth, windowWidth);
                draggingInfo.bounds.xmin = (draggingInfo.bounds.xmax = currentlyDragging->xmax = x) - panel::panelDraggableWidth;
            }
            // Top
            if (draggingTop)
            {
                int y = Clamp(GetMouseY(), 0, currentlyDragging->ymax - panel::minHeight);
                draggingInfo.bounds.ymax = (draggingInfo.bounds.ymin = currentlyDragging->ymin = y) + panel::panelDraggableWidth;
            }
            // Bottom
            else if (draggingBottom)
            {
                int y = Clamp(GetMouseY(), currentlyDragging->ymin + panel::minHeight, windowHeight);
                draggingInfo.bounds.ymin = (draggingInfo.bounds.ymax = currentlyDragging->ymax = y) - panel::panelDraggableWidth;
            }
        }
        else
        {
            panel::PanelHover testPanelHover = panel::CheckPanelCollision(testPanel, panel::DraggableEdges::All, GetMouseX(), GetMouseY());

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (testPanelHover)
                {
                    currentlyDragging = &testPanel;
                    draggingInfo = testPanelHover;
                }
            }
        }

        /******************************************
        *   Draw the frame                        *
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            SetMouseCursor(MouseCursor::MOUSE_CURSOR_DEFAULT);

            panel::DrawPanel("Panel", testPanel);

            if (currentlyDragging)
            {
                panel::DrawPanelDragElement(*currentlyDragging, draggingInfo);
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