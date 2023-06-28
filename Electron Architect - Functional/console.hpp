#pragma once
extern "C"
{
	#include "logtypes.h"
}
#include "panel.hpp"

// Functions related to the Console panel.
namespace console
{
	struct LogElement
	{
		// Element styling
		LogType type = LOGTYPE_NORMAL;
		// Should always set this
		const char* content = nullptr;
		// Indentation of the log
		size_t indent = 0;
		// Number of duplicate logs being compressed into this
		size_t count = 1;
		// The time this element was last hovered (used for animated fade)
		double lastHovered = 0.0;
		// Whether the log content is heap memory and needs to be freed before overwriting or closing the program
		// Technically, it just means whether it is the console's duty to free it.
		// Heap memory may still be passed in; but if it wasn't allocated by the console, it won't be freed by the console.
		bool contentUsesHeap = false;
	};

	extern panel::Panel consolePanel;
	extern size_t displayableLogCount;

	constexpr size_t maxLogs = 32;
	constexpr size_t maxLogCount = 99; // Duplicate log count before log won't increment

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
