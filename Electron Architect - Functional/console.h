#pragma once
#include "panel.h"

// Functions related to the Console panel.
namespace console
{
	extern panel::Panel consolePanel;
	void DrawPanelContents();

	void Log  (const char* text);
	void Warn (const char* text);
	void Error(const char* text);
	void Group(const char* groupName);
	void GroupEnd();
	void Clear();
}
