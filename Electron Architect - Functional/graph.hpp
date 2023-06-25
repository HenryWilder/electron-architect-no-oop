#pragma once
#include "panel.hpp"

// Functions related to the circuit graphing feature of the program.
namespace graph
{
	extern panel::Panel graphPanel;

	// Negatives are treated as division
	// Zero is treated as 1x scale
	extern int gridMagnitude;

	void Zoom(int amount);

	// Number of grid spaces offset horizontally
	// This is *added* to the panel's xmin
	extern int gridOffsetX;

	// Number of grid spaces offset vertically
	// This is *added* to the panel's ymin
	extern int gridOffsetY;

	void DrawPanelContents(int mousexNow, int mouseyNow, int mousexMid, int mouseyMid, int mousexOld, int mouseyOld, bool allowHover, bool isMousePressed);
}
