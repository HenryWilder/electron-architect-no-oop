#include <raylib.h>
#include "console.h"

namespace console
{
	constexpr int consolePaddingX = 4;
	constexpr int consolePaddingY = 4;

	enum class LogStyle
	{
		Normal,
		Warning,
		Error,
	};

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

	struct LogElement
	{
		LogStyle style;
		size_t indent;
		const char* content;
	};

	constexpr int lineHeight = 16;
	constexpr int logIndentWidth = 10;
	constexpr size_t maxLogs = 32;
	LogElement logs[maxLogs];
	size_t currentIndent = 0;
	size_t totalLogs = 0; // At most maxLogs

	size_t CalculateDisplayableLogCount()
	{
		int panelHeight = consolePanel.bounds.ymax - consolePanel.bounds.ymin;
		int bodyHeight = panelHeight - panel::panelTitlebarHeight;
		int contentBoxHeight = bodyHeight - consolePaddingY * 2;
		int numFittableLogs = contentBoxHeight / lineHeight;

		return (size_t)numFittableLogs;
	}

	// Todo: Make this work for multiline logs
	void DrawPanelContents()
	{
		size_t displayableLogs = CalculateDisplayableLogCount();
		size_t startLog = totalLogs < displayableLogs ? 0 : totalLogs - displayableLogs;
		size_t numDisplayableLogs = totalLogs - startLog;
		int logBoxXMin = consolePanel.bounds.xmin + panel::borderWidth;
		int logBoxXMax = consolePanel.bounds.xmax - panel::borderWidth;
		int logBoxYMin = consolePanel.bounds.ymin + panel::panelTitlebarHeight;
		for (size_t i = 0; i < numDisplayableLogs; ++i)
		{
			size_t logIndex = startLog + i;
			LogElement log = logs[logIndex];
			DrawRectangle(logBoxXMin, logBoxYMin, logBoxXMax - logBoxXMin, lineHeight, SKYBLUE);
			DrawText(
				log.content,
				logBoxXMin + log.indent * logIndentWidth + consolePaddingX,
				logBoxYMin + consolePaddingY,
				8,
				WHITE);
			logBoxYMin += lineHeight;
		}
	}

	// todo: style
	void AppendLog(LogStyle style, const char* text)
	{
		if (totalLogs < maxLogs)
		{
			++totalLogs;
		}
		else
		{
			for (size_t i = 1; i < totalLogs; ++i)
			{
				logs[i - 1] = logs[i];
			}
		}

		logs[totalLogs - 1] = {
			style,
			currentIndent,
			text,
		};
	}

	void Log(const char* text)
	{
		AppendLog(LogStyle::Normal, text);
	}

	void Warn(const char* text)
	{
		AppendLog(LogStyle::Warning, text);
	}

	void Error(const char* text)
	{
		AppendLog(LogStyle::Error, text);
	}

	void Group(const char* groupName)
	{
		AppendLog(LogStyle::Normal, groupName);
		++currentIndent;
	}

	void GroupEnd()
	{
		--currentIndent;
	}

	void Clear()
	{
		totalLogs = 0;
		currentIndent = 0;
	}
}
