#pragma once
#include "panel.hpp"

// Functions related to the Console panel.
namespace console
{
	extern panel::Panel consolePanel;
	extern size_t displayableLogCount;

	// Updates the value of displayableLogCount. Should be called anytime the console panel is resized.
	// (Which could be caused by another panel resizing, so call it when *any* panel resizes)
	void CalculateDisplayableLogCount();

	void DrawPanelContents(int mousex, int mousey, bool allowHover);

	void Log    (const char* text);
	void Logf   (const char* fmt...);

	void Warn   (const char* text);
	void Warnf  (const char* fmt...);

	void Error  (const char* text);
	void Errorf (const char* fmt...);

	void Assert (bool condition, const char* text);
	void Assertf(bool condition, const char* fmt...);

	void Group(const char* groupName);
	void GroupEnd();

	void Clear();

	// Performs any cleanup needed to unload resources associated with this namespace when the program closes
	void Unload();
}
