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
		if (-gridMagnitude > gridSize)
		{
			gridMagnitude = -gridSize;
		}
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

	void DrawPanelContents(int mousex, int mousey, bool allowHover, bool isMousePressed)
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
	}
}
