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

    while (!WindowShouldClose())
    {
        /******************************************
        *   Simulate frame and update variables   *
        ******************************************/

        panel::PanelHover testPanelHover = panel::CheckPanelCollision(testPanel, panel::DraggableEdges::All, GetMouseX(), GetMouseY());

        /******************************************
        *   Draw the frame                        *
        ******************************************/

        BeginDrawing(); {

            ClearBackground(BLACK);

            SetMouseCursor(MouseCursor::MOUSE_CURSOR_DEFAULT);

            panel::DrawPanel("Panel", testPanel, testPanelHover);

        } EndDrawing();
    }

    /******************************************
    *   Unload and free memory                *
    ******************************************/

    // @TODO: Unload variables

    CloseWindow();

	return 0;
}