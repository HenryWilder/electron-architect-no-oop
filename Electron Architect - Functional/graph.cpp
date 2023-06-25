#include "console.hpp"
#include "properties.hpp"
#include "tools.hpp"
#include "graph.hpp"

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

	void DrawMouseTrail(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, float gridDisplaySize)
	{
		float gridDisplaySizeHalf = (float)gridDisplaySize * 0.5f;

		Vector2 mouseOldCenter = { mousexOld, mouseyOld };
		Vector2 mouseMidCenter = { mousexMid, mouseyMid };
		Vector2 mouseNowCenter = { mousexNow, mouseyNow };

		DrawLineBezierQuad(mouseOldCenter, mouseNowCenter, mouseMidCenter, gridDisplaySize, { 255,255,0, 63 });
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
			int hoveredSpaceXNow = (int)(mousexNow / gridDisplaySize_WithLine) * gridDisplaySize_WithLine - 1; // No idea why the x is off by one like that
			int hoveredSpaceYNow = (int)(mouseyNow / gridDisplaySize_WithLine) * gridDisplaySize_WithLine;
			int hoveredSpaceXMid = (int)(mousexMid / gridDisplaySize_WithLine) * gridDisplaySize_WithLine - 1; // No idea why the x is off by one like that
			int hoveredSpaceYMid = (int)(mouseyMid / gridDisplaySize_WithLine) * gridDisplaySize_WithLine;

			// Only draw cursor if there is no movement
			if (hoveredSpaceXNow == hoveredSpaceXMid && hoveredSpaceYNow == hoveredSpaceYMid)
			{
				DrawRectangle(hoveredSpaceXNow, hoveredSpaceYNow, gridDisplaySize, gridDisplaySize, { 255,255,0, 127 });
			}
			else
			{
				DrawMouseTrail(mousexNow, mouseyNow, mousexMid, mouseyMid, mousexOld, mouseyOld, gridDisplaySize);
			}
		}
	}
}
