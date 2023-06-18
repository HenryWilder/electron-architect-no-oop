#include "console.hpp"
#include "tools.hpp"

namespace tools
{
	panel::Panel toolsPanel = {
		"Tools",
		{
			&panel::windowBounds.xmin,
			&panel::windowBounds.ymin,
			64,
			&console::consolePanel.bounds.ymin
		},
		panel::DraggableEdges::None
	};

	void DrawPanelContents()
	{

	}
}
