#include "console.h"
#include "tools.h"

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
