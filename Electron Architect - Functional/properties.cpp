#include "console.h"
#include "properties.h"

namespace properties
{
	panel::Panel propertiesPanel = { "Properties", { 1000, &panel::windowBounds.ymin, &panel::windowBounds.xmax, &console::consolePanel.bounds.ymin }, panel::DraggableEdges::EdgeL };
}
