#include "console.h"

namespace console
{
	panel::Panel consolePanel = { "Console", { panel::windowBounds.xmin, 500, 1280, 720 }, panel::DraggableEdges::EdgeT };
}
