#pragma once
#include "panel.hpp"

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
}
