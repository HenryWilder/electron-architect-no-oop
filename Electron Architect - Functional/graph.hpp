#pragma once
#include "panel.hpp"
#include <string>

// Functions related to the circuit graphing feature of the program.
namespace graph
{
	enum class NodeType : char
	{
		Any = '|',
		All = '&',
		Non = '!',
		One = '^',
	};

	enum class WireElbow : unsigned char
	{
		DiagonalHori = 0,
		DiagonalVert = 1,
		HoriDiagonal = 2,
		VertDiagonal = 3,
	};

	constexpr size_t MAX_NODES = 4096;
	constexpr size_t MAX_WIRES = 4096;

	struct Node
	{
		NodeType type;
		int x, y;
		std::string name;
	};

	extern size_t numNodes;
	extern Node nodes[MAX_NODES];

	struct Wire
	{
		WireElbow elbow;
		size_t startNode, endNode; // No null node pointers. A wire should not exist if it doesn't connect two existing nodes.
	};

	extern size_t numWires;
	extern Wire wires[MAX_WIRES];

	extern panel::Panel graphPanel;

	// Number of grid spaces offset horizontally
	// This is *added* to the panel's xmin
	extern int gridOffsetX;

	// Number of grid spaces offset vertically
	// This is *added* to the panel's ymin
	extern int gridOffsetY;

	// Call this after performing modifications to zoom/offset, and prior to placing elements
	void UpdateGridDisplaySize();

	void DrawPanelContents(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, bool allowHover, bool isMousePressed);

	void Zoom(int amount);

	void AddNode(NodeType type, int screenx, int screeny);
	void RemoveNode(int screenx, int screeny);
}
