#include "console.h"

namespace console
{
	panel::Panel consolePanel = {
		"Console",
		{
			&panel::windowBounds.xmin,
			500,
			&panel::windowBounds.xmax,
			&panel::windowBounds.ymax
		},
		panel::DraggableEdges::EdgeT
	};

	void DrawPanelContents()
	{

	}

	struct LogElement
	{
		int style;
		const char* content;
		LogElement* parent;
	};

	// The current group to add child logs to
	LogElement* currentGroup = nullptr;

	constexpr size_t maxLogs = 16;
	LogElement _logMem[maxLogs];
	LogElement logs[maxLogs];
	size_t totalLogs = 0; // At most maxLogs

	// todo: style
	LogElement* AppendLog(int style, const char* text)
	{
		for (size_t i = 1; i < maxLogs; ++i)
		{
			logs[i - 1] = logs[i];
		}

		logs[maxLogs - 1] = {
			style,
			text,
			currentGroup,
		};
	}

	void Log(const char* text)
	{
		AppendLog(NULL, text);
	}

	void Warn(const char* text)
	{
		AppendLog(NULL, text);
	}

	void Error(const char* text)
	{
		AppendLog(NULL, text);
	}

	void Group(const char* groupName)
	{
		currentGroup = AppendLog(NULL, groupName);
	}

	void GroupEnd()
	{
		currentGroup = currentGroup->parent;
	}

	void Clear()
	{
		totalLogs = 0;
	}
}
