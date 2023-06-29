#pragma once
#include <string>
#include "panel.hpp"

// Functions related to the circuit graphing feature of the program.
namespace graph
{
	// At gridMagnitude 0, this is the sidelength in pixels of a single gridspace.
	// Excludes the width of the gridline.
	constexpr int gridSize = 16;

	// Independent of zoom
	constexpr int gridlineWidth = 1;

	// Negatives are treated as division
	// Zero is treated as 1x scale
	extern int gridMagnitude;

	extern int gridOffsetX;
	extern int gridOffsetY;

	extern int gridDisplaySize;
	extern int gridDisplaySize_WithLine;

	enum class NodeType : char
	{
		Any = '|',
		All = '&',
		Non = '!',
		One = '^',
	};

	struct Node
	{
		NodeType type;
		int x, y;
		std::string name;
	};

	constexpr size_t MAX_NODES = 4096;
	extern size_t numNodes;
	extern Node* nodes[MAX_NODES];

	enum class WireElbow : unsigned char
	{
		DiagonalHori = 0,
		DiagonalVert = 1,
		HoriDiagonal = 2,
		VertDiagonal = 3,
	};

	// @No null node pointers. A wire should not exist if it doesn't connect two existing nodes.
	struct Wire
	{
		WireElbow elbow;
		Node* startNode;
		Node* endNode;
	};

	constexpr size_t MAX_WIRES = 4096;
	extern size_t numWires;
	extern Wire* wires[MAX_WIRES];

	extern panel::Panel graphPanel;

	// Number of grid spaces offset horizontally
	// This is *added* to the panel's xmin
	extern int gridOffsetX;

	// Number of grid spaces offset vertically
	// This is *added* to the panel's ymin
	extern int gridOffsetY;

	// Call this after performing modifications to zoom/offset, and prior to placing elements
	void UpdateGridDisplaySize();

	void DrawPanelContents(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, bool allowHover);

	void Zoom(int amount);

	void AddNode(NodeType type, int screenx, int screeny);
	void RemoveNode(int screenx, int screeny);
}
