#include "console.hpp"
#include "tools.hpp"

namespace tools
{
	panel::Panel toolsPanel = {
		.title = "Tools",
		.id = panel::PanelID::Tools,
		.bounds = {
			&panel::windowBounds.xmin,
			&panel::windowBounds.ymin,
			64,
			&console::consolePanel.bounds.ymin
		},
		.draggable = panel::DraggableEdges::None
	};

	void DrawPanelContents()
	{

	}
}
