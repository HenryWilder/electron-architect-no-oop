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
		FailedAssertion,
	};
	const char* logTypeStr[] = {
		"Info",
		"Error",
		"Warning",
		"Failed Assertion"
	};
	// Width of the "log type" segment of the log element
	int logTypeWidth[4] = {};

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
		size_t count;
	};

	constexpr int lineHeight = 16;
	constexpr int logIndentWidth = 16;
	constexpr size_t maxLogs = 32;
	// Log count before log won't increment
	constexpr size_t maxLogCount = 99;
	LogElement logs[maxLogs];
	size_t currentIndent = 0;
	size_t totalLogs = 0; // At most maxLogs
	size_t displayableLogCount = 0; // Update when panel size changes

	void CalculateDisplayableLogCount()
	{
		int panelHeight = consolePanel.bounds.ymax - consolePanel.bounds.ymin;
		if (panelHeight <= panel::panelTitlebarHeight)
		{
			displayableLogCount = 0;
			return;
		}
		int bodyHeight = panelHeight - panel::panelTitlebarHeight;
		int contentBoxHeight = bodyHeight - consolePaddingY * 2;
		int numFittableLogs = contentBoxHeight / lineHeight + 2;

		displayableLogCount = (size_t)numFittableLogs;
	}

	// Todo: Make this work for multiline logs
	void DrawPanelContents() 
	{
		size_t displayableLogs = displayableLogCount;
		size_t startLog = totalLogs < displayableLogs ? 0 : totalLogs - displayableLogs;
		size_t numDisplayableLogs = totalLogs - startLog;
		int logBoxXMin = consolePanel.bounds.xmin + panel::borderWidth;
		int logBoxXMax = consolePanel.bounds.xmax - panel::borderWidth;
		int logBoxYMax = consolePanel.bounds.ymax - panel::borderWidth;
		int logBoxYMin = logBoxYMax - lineHeight;
		for (size_t _i = numDisplayableLogs; _i > 0; --_i)
		{
			size_t i = _i - 1;
			size_t logIndex = startLog + i;
			LogElement log = logs[logIndex];
			Color color, backgroundColor;
			switch (log.style)
			{
			case LogStyle::Warning:
				backgroundColor = Color{ 127,127,0, 64 };
				color = Color{ 255,255,0, 255 };
				break;

			case LogStyle::FailedAssertion:
			case LogStyle::Error:
				backgroundColor = Color{ 127,0,0, 64 };
				color = Color{ 255,0,0, 255 };
				break;

			default:
				backgroundColor = Color{ 0,0,0, 0 };
				color = Color{ 255,255,255, 255 };
				break;
			}
			DrawRectangle(logBoxXMin, logBoxYMin, logBoxXMax - logBoxXMin, lineHeight, backgroundColor);
			if (logTypeWidth[(int)log.style] == 0)
			{
				logTypeWidth[(int)log.style] = MeasureText(logTypeStr[(int)log.style], 8) + 7;
			}
			int displayedCount = log.count <= maxLogCount ? log.count : maxLogCount;
			const char* displayedCountStr = TextFormat(log.count <= maxLogCount ? "%2i" : "%2i+", displayedCount);
			bool shouldDisplayCount = log.count > 1;
			int displayedCountWidth = shouldDisplayCount ? 24 : 0;
			int indent = log.indent * logIndentWidth;
			int paddedIndent = indent + consolePaddingX;
			int logTypeWidthHere = logTypeWidth[(int)log.style];
			DrawRectangle(logBoxXMin + 1 + indent + displayedCountWidth, logBoxYMin + 2, logTypeWidthHere - 2, lineHeight - 3, backgroundColor);
			if (shouldDisplayCount)
			{
				DrawText(
					displayedCountStr,
					logBoxXMin + paddedIndent,
					logBoxYMin + consolePaddingY,
					8,
					color);
			}
			DrawText(
				logTypeStr[(int)log.style],
				logBoxXMin + paddedIndent + displayedCountWidth,
				logBoxYMin + consolePaddingY,
				8,
				color);
			DrawText(
				log.content,
				logBoxXMin + paddedIndent + logTypeWidthHere + displayedCountWidth,
				logBoxYMin + consolePaddingY,
				8,
				color);
			logBoxYMin -= lineHeight;
		}
	}

	bool TextMatches(const char* str1, const char* str2)
	{
		if (str1 == nullptr || str2 == nullptr)
		{
			return false;
		}

		for (; *str1 != '\0' || *str2 != '\0'; ++str1, ++str2)
		{
			if (*str1 != *str2)
			{
				return false;
			}
		}

		return true;
	}

	// todo: style
	void AppendLog(LogStyle style, const char* text)
	{
		if (totalLogs != 0)
		{
			LogElement& lastLog = logs[totalLogs - 1];
			if (TextMatches(text, lastLog.content))
			{
				if (lastLog.count <= maxLogCount)
				{
					++lastLog.count;
				}
				return;
			}
		}

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
			1,
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

	void Assert(bool condition, const char * text)
	{
		if (!condition)
		{
			AppendLog(LogStyle::FailedAssertion, text);
		}
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
