#include "console.h"

namespace console
{
	panel::Panel consolePanel = { "Console", { &panel::windowBounds.xmin, 500, &panel::windowBounds.xmax, &panel::windowBounds.ymax }, panel::DraggableEdges::EdgeT };
}
