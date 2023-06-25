#include "console.hpp"
#include "properties.hpp"
#include "tools.hpp"
#include "graph.hpp"
#include <raymath.h>

using panel::Panel;
using panel::PanelID;
using panel::Bounds;
using panel::Rect;
using tools::toolsPanel;
using properties::propertiesPanel;
using console::consolePanel;

namespace graph
{
	Panel graphPanel = {
		.title = "Graph",
		.id = PanelID::Graph,
		.bounds = {
			&toolsPanel.bounds.xmax,
			0,
			&propertiesPanel.bounds.xmin,
			&consolePanel.bounds.ymin,
		},
		.draggable = (panel::DraggableEdges)((int)panel::DraggableEdges::EdgeB | (int)panel::DraggableEdges::EdgeR)
	};

	// At gridMagnitude 0, this is the sidelength in pixels of a single gridspace.
	// Excludes the width of the gridline.
	constexpr int gridSize = 16;

	// Independent of zoom
	constexpr int gridlineWidth = 1;

	// Size of display gridspaces before lines are replaced with a rectangle fill
	// Todo: Change to use a shader instead
	constexpr int gridlineFillThreshold = 4;

	constexpr Color   gridlineColor = { 127,127,127,  63 };
	constexpr Color backgroundColor = {  20, 20, 20, 255 };

	int gridMagnitude = 0;

	void Zoom(int amount)
	{
		gridMagnitude -= amount;
		if (gridMagnitude > gridSize)
		{
			gridMagnitude = gridSize;
		}
		if (gridMagnitude < -gridSize)
		{
			gridMagnitude = -gridSize;
		}

#if _DEBUG && false
		if (gridMagnitude > 0)
		{
			console::Logf("Zoomed to x/%i", (gridMagnitude + 1));
		}
		else if (gridMagnitude < 0)
		{
			console::Logf("Zoomed to %ix", (-gridMagnitude + 1));
		}
		else // gridMagnitude == 0
		{
			console::Log("Zoomed to 1x");
		}
#endif
	}

	int gridOffsetX = 0;
	int gridOffsetY = 0;

	constexpr Color hoveredSpaceColor = { 255,255,0, 200 };

	// Todo: Change to use a shader instead
	void DrawGrid(const Bounds& clientBounds, const Rect& clientRect, int gridDisplaySize, int gridDisplaySize_WithLine)
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
	void DrawMouseTrail(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, float gridDisplaySize)
	{
		float gridDisplaySizeHalf = (float)gridDisplaySize * 0.5f;

		Vector2 mouseOld = { mousexOld, mouseyOld };
		Vector2 mouseMid = { mousexMid, mouseyMid };
		Vector2 mouseNow = { mousexNow, mouseyNow };

		float smearLength = sqrtf(Vector2DistanceSqr(mouseOld, mouseMid) + Vector2DistanceSqr(mouseMid, mouseNow));

		smearLength /= 32.0f; // Smudge factor

		float baseAlpha = (float)hoveredSpaceColor.a / 255.0f;
		float smearAlpha = (smearLength == 0) ? baseAlpha : (baseAlpha / smearLength);

		DrawLineBezierQuad(mouseOld, mouseNow, mouseMid, gridDisplaySize, ColorAlpha(hoveredSpaceColor, smearAlpha));
	}

	void DrawPanelContents(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, bool allowHover, bool isMousePressed)
	{
		Bounds clientBounds = panel::PanelClientBounds(graphPanel);

		// Fill background
		Rect clientRect;
		panel::BoundsToRect(clientRect, clientBounds);
		DrawRectangle(clientRect.x, clientRect.y, clientRect.w, clientRect.h, backgroundColor);

		int gridDisplaySize = gridSize;

		if (gridMagnitude > 0)
		{
			gridDisplaySize /= (gridMagnitude + 1);
		}
		else if (gridMagnitude < 0)
		{
			gridDisplaySize *= (-gridMagnitude + 1);
		}

		int gridDisplaySize_WithLine = gridDisplaySize + gridlineWidth;

		DrawGrid(clientBounds, clientRect, gridDisplaySize, gridDisplaySize_WithLine);

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
				DrawMouseTrail(mousexNow, mouseyNow, mousexMid, mouseyMid, mousexOld, mouseyOld, gridDisplaySize);
			}
		}
	}
}
