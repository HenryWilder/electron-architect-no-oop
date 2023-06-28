#include <raylib.h>
#include <raymath.h>
#include "panel.hpp"
#include "graph.hpp"

using panel::Panel;
using panel::PanelID;
using panel::Bounds;
using panel::Rect;

namespace graph
{
	// Size of display gridspaces before lines are replaced with a rectangle fill
	// Todo: Change to use a shader instead
	constexpr int gridlineFillThreshold = 4;

	constexpr Color     gridlineColor = { 127,127,127,  63 };
	constexpr Color   backgroundColor = {  20, 20, 20, 255 };
	constexpr Color hoveredSpaceColor = { 255,255,  0, 200 };

	// Todo: Change to use a shader instead
	void DrawGrid(const Bounds& clientBounds, const Rect& clientRect)
	{
		const int edgeL = clientBounds.xmin;
		const int edgeT = clientBounds.ymin;
		const int edgeR = clientBounds.xmax;
		const int edgeB = clientBounds.ymax;

		if (gridDisplaySize > 4)
		{
			for (int x = edgeL; x < edgeR; x += gridDisplaySize_WithLine)
			{
				DrawLine(x, edgeT, x, edgeB, gridlineColor);
			}

			for (int y = edgeT; y < edgeB; y += gridDisplaySize_WithLine)
			{
				DrawLine(edgeL, y, edgeR, y, gridlineColor);
			}
		}
		else // Finer than four pixels per space means we fill the background instead
		{
			float baseGridlineAlpha = gridlineColor.a / 255.0f;
			float scaledGridlineAlpha = (float)baseGridlineAlpha * ((float)(gridlineWidth * 2.0f) / (float)gridDisplaySize_WithLine);
			Color color = ColorAlpha(gridlineColor, scaledGridlineAlpha);
			DrawRectangle(clientRect.x, clientRect.y, clientRect.w, clientRect.h, color);
		}
	}

	// Draws a "smear" of the cursor
	void DrawMouseTrail(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld)
	{
		float gridDisplaySizeHalf = (float)gridDisplaySize * 0.5f;

		Vector2 mouseOld = { (float)mousexOld, (float)mouseyOld };
		Vector2 mouseMid = { (float)mousexMid, (float)mouseyMid };
		Vector2 mouseNow = { (float)mousexNow, (float)mouseyNow };

		float smearLength = sqrtf(Vector2DistanceSqr(mouseOld, mouseMid) + Vector2DistanceSqr(mouseMid, mouseNow));

		smearLength /= 32.0f; // Smudge factor

		float baseAlpha = (float)hoveredSpaceColor.a / 255.0f;
		float smearAlpha = (smearLength == 0) ? baseAlpha : (baseAlpha / smearLength);

		DrawLineBezierQuad(mouseOld, mouseNow, mouseMid, (float)gridDisplaySize, ColorAlpha(hoveredSpaceColor, smearAlpha));
	}

	void DrawPanelContents(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, bool allowHover, bool isMousePressed)
	{
		Bounds clientBounds = panel::PanelClientBounds(graphPanel);

		// Fill background
		Rect clientRect;
		panel::BoundsToRect(clientRect, clientBounds);
		DrawRectangle(clientRect.x, clientRect.y, clientRect.w, clientRect.h, backgroundColor);

		DrawGrid(clientBounds, clientRect);

		if (allowHover)
		{
			// Todo: Space calculation isn't accounting for zoom!

			int hoveredSpaceX = mousexNow / gridDisplaySize_WithLine;
			int hoveredSpaceY = mouseyNow / gridDisplaySize_WithLine;
			int hoveredSpaceXPrev = mousexOld / gridDisplaySize_WithLine;
			int hoveredSpaceYPrev = mouseyOld / gridDisplaySize_WithLine;

			int hoveredXSnapped = hoveredSpaceX * gridDisplaySize_WithLine - 1; // No idea why the x is off by one like that
			int hoveredYSnapped = hoveredSpaceY * gridDisplaySize_WithLine;

			int hoveredXSnappedPrev = hoveredSpaceXPrev * gridDisplaySize_WithLine - 1; // No idea why the x is off by one like that
			int hoveredYSnappedPrev = hoveredSpaceYPrev * gridDisplaySize_WithLine;

			int moveDistance = abs(hoveredSpaceX - hoveredSpaceXPrev) + abs(hoveredSpaceY - hoveredSpaceYPrev);

			// Only draw cursor if there is no movement
			if (moveDistance < 2)
			{
				DrawRectangle(hoveredXSnapped, hoveredYSnapped, gridDisplaySize, gridDisplaySize, hoveredSpaceColor);
			}
			else
			{
				DrawMouseTrail(mousexNow, mouseyNow, mousexMid, mouseyMid, mousexOld, mouseyOld);
			}
		}

		// Draw nodes
		{
			float nodeRadius = (float)gridDisplaySize / 2.0f;
			for (size_t i = 0; i < numNodes; ++i)
			{
				Vector2 position =
				{
					.x = (float)(nodes[i].x * gridDisplaySize_WithLine) + nodeRadius - 1.0f,
					.y = (float)(nodes[i].y * gridDisplaySize_WithLine) + nodeRadius,
				};
				DrawCircleV(position, nodeRadius, BLUE);
			}
		}
	}
}
