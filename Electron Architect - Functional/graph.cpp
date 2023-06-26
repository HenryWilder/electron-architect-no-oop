#include <fstream>
#include <string>
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

	size_t numNodes = 0;
	Node nodes[MAX_NODES] = {};

	size_t numWires = 0;
	Wire wires[MAX_WIRES] = {};

	void Save(const char* filename)
	{
		std::ofstream file(filename);

		int majorVersion = 2;
		int minorVersion = 0;
		int patchVersion = 0;

		file << "v " << majorVersion << ' ' << minorVersion << ' ' << patchVersion << std::endl;

		file << "n " << numNodes;
		for (size_t i = 0; i < numNodes; ++i)
		{
			const Node& node = nodes[i];
			file << '\n' << (char)node.type << ' ' << node.x << ' ' << node.y << " ```" << node.name << "```";
		}
		file << std::endl;

		file << "w " << numWires;
		for (size_t i = 0; i < numWires; ++i)
		{
			const Wire& wire = wires[i];
			file << '\n' << (int)wire.elbow << ' ' << wire.startNode << ' ' << wire.endNode;
		}
		file << std::endl;

		file.close();
	}

	void Load(const char* filename)
	{
		std::ifstream file(filename);

		char regionCode = '\0';

		file >> regionCode;
		if (regionCode != 'v')
		{
			console::Error("graph: File is malformed or incompatible: Expected VERSION (v) region. Cancelling.");
			return;
		}
		int majorVersion, minorVersion, patchVersion;
		file >> majorVersion >> minorVersion >> patchVersion;

		file >> regionCode;
		if (regionCode != 'n')
		{
			console::Error("graph: File is malformed or incompatible: Expected NODE (n) region. Cancelling.");
			return;
		}
		file >> numNodes;

		for (size_t i = 0; i < numNodes; ++i)
		{
			Node& node = nodes[i];
			char _type;
			file >> _type;
			node.type = (NodeType)_type;
			file >> node.x >> node.y;

			// I have concerns about this.
			if (file.peek() != '\n' && !file.eof())
			{
				int ch;
				constexpr size_t BUFF_MAX_SIZE = 512;
				char buff[BUFF_MAX_SIZE] = {};
				size_t strBuffSize = 0;
				while ((ch = file.get()) != '\n')
				{
					buff[strBuffSize++] = ch;
					if (strBuffSize == BUFF_MAX_SIZE - 1)
					{
						console::Warn("Node name exceeds maximum length of 511 characters. Some information will be lost.");
					}
				}
				buff[strBuffSize] = '\0';
				node.name.reserve(strBuffSize);
				node.name = buff;
			}
		}

		file >> regionCode;
		if (regionCode != 'w')
		{
			console::Error("graph: File is malformed or incompatible: Expected WIRE (w) region. Cancelling.");
			return;
		}
		file >> numWires;

		for (size_t i = 0; i < numWires; ++i)
		{
			Wire& wire = wires[i];
			int _elbow;
			file >> _elbow;
			wire.elbow = (WireElbow)_elbow;
			file >> wire.startNode >> wire.endNode;
		}

		file.close();
	}

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

	// Negatives are treated as division
	// Zero is treated as 1x scale
	int gridMagnitude = 0;

	int gridOffsetX = 0;
	int gridOffsetY = 0;

	int gridDisplaySize;
	int gridDisplaySize_WithLine;

	void UpdateGridDisplaySize()
	{
		if (gridMagnitude > 0)
		{
			gridDisplaySize = gridSize / (gridMagnitude + 1);
		}
		else if (gridMagnitude < 0)
		{
			gridDisplaySize = gridSize * (-gridMagnitude + 1);
		}
		else // gridMagnitude == 0
		{
			gridDisplaySize = gridSize;
		}

		gridDisplaySize_WithLine = gridDisplaySize + gridlineWidth;
	}

	constexpr Color hoveredSpaceColor = { 255,255,0, 200 };

	int ScreenXToGridX(int x)
	{
		return 0;
	}

	int ScreenYToGridY(int y)
	{
		return 0;
	}

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
	}
}
