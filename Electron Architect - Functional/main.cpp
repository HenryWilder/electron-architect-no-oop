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
            // Left or right
            if (panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerL) ||
                panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerR) ||
                panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::EdgeCol))
            {
                draggingInfo.SetPos(GetMouseX());
            }

            // Top or bottom
            if (panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerT) ||
                panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerB) ||
                panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::EdgeRow))
            {
                draggingInfo.SetPos(GetMouseY());
            }

            // Left
            if (panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerL) ||
                draggingInfo == panel::HoverSection::EdgeL)
            {
                currentlyDragging->xmin = GetMouseX();
            }
            // Right
            else if (panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerR) ||
                draggingInfo == panel::HoverSection::EdgeR)
            {
                currentlyDragging->xmax = GetMouseX();
            }
            
            // Top
            if (panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerT) ||
                draggingInfo == panel::HoverSection::EdgeT)
            {
                currentlyDragging->ymin = GetMouseY();
            }
            // Bottom
            else if (panel::HasHoverSectionFlag(draggingInfo, panel::HoverSection::CornerB) ||
                draggingInfo == panel::HoverSection::EdgeB)
            {
                currentlyDragging->ymax = GetMouseY();
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