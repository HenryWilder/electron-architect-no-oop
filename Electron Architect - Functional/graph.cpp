#include "console.hpp"
#include "properties.hpp"
#include "tools.hpp"
#include "graph.hpp"

namespace graph
{
	panel::Panel graphPanel = {
		.title = "Graph",
		.id = panel::PanelID::Graph,
		.bounds = {
			&tools::toolsPanel.bounds.xmax,
			0,
			&properties::propertiesPanel.bounds.xmin,
			&console::consolePanel.bounds.ymin,
		},
		.draggable = (panel::DraggableEdges)((int)panel::DraggableEdges::EdgeB | (int)panel::DraggableEdges::EdgeR)
	};

	void DrawPanelContents()
	{

	}
}
