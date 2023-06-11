#include "console.h"
#include "properties.h"
#include "tools.h"
#include "graph.h"

namespace graph
{
	panel::Panel graphPanel = {
		"Graph",
		{
			&tools::toolsPanel.bounds.xmax,
			0,
			&properties::propertiesPanel.bounds.xmin,
			&console::consolePanel.bounds.ymin,
		},
		(panel::DraggableEdges)((int)panel::DraggableEdges::EdgeB | (int)panel::DraggableEdges::EdgeR)
	};
}
