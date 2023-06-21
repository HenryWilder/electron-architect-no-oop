#include <cstdarg>
#include <raylib.h>
#include <raymath.h>
extern "C"
{
#include "logtypes.h"
}
#include "textfmt.hpp"
#include "console.hpp"
#pragma warning( push )
#pragma warning( disable : 26812 )

float EaseInOut(float t)
{
	float sqt = t * t;
	return sqt / (2.0 * (sqt - t) + 1.0f);
}

namespace console
{
	constexpr int consolePaddingX = 4;
	constexpr int consolePaddingY = 4;

	panel::Panel consolePanel =
	{
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
		// Element styling
		LogType type = LogType::LOGTYPE_NORMAL;
		// Should always set this
		const char* content = nullptr;
		// Indentation of the log
		size_t indent = 0;
		// Number of duplicate logs being compressed into this
		size_t count = 1;
		// The time this element was last hovered (used for animated fade)
		double lastHovered = 0.0;
		// Whether the log content is heap memory and needs to be freed before overwriting or closing the program
		bool contentUsesHeap = false;
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

	// Time in seconds for a log to fade after being unhovered
	constexpr double logFadeTime = 0.35;
	constexpr float backgroundHoverAlpha = 0.35f;

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
	void DrawPanelContents(int mousex, int mousey, bool allowHover)
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
			const LogElement log = logs[logIndex];

			auto [color, backgroundColor] = logTypeStyles[log.type];

			// Mouse is within the log element
			if (allowHover &&
				logBoxXMin <= mousex && mousex <= logBoxXMax &&
				logBoxYMin <= mousey && mousey <  logBoxYMin + lineHeight) // y is < instead of <= so that mouse doesn't overlap multiple elements
			{
				LogElement& logRef = logs[logIndex];
				logRef.lastHovered = GetTime();
			}

			float minBgAlpha = backgroundHoverAlpha     - ((log.type == LOGTYPE_NORMAL) ? backgroundHoverAlpha : 0.0f);
			float maxBgAlpha = backgroundHoverAlpha * 2 - ((log.type == LOGTYPE_NORMAL) ? backgroundHoverAlpha : 0.0f);
			float backgroundAlpha = Lerp(maxBgAlpha, minBgAlpha, EaseInOut(Clamp((float)((GetTime() - log.lastHovered) / logFadeTime), 0.0f, 1.0f)));

			DrawRectangle(logBoxXMin, logBoxYMin, logBoxXMax - logBoxXMin, lineHeight, ColorAlpha(backgroundColor, backgroundAlpha));

			if (logTypeWidth[log.type] == 0) [[unlikely]] // Should only happen once per unique log type
			{
				logTypeWidth[log.type] = MeasureText(logTypeStr[log.type], 8) + 7;
			}

			int displayedCount = log.count <= maxLogCount ? log.count : maxLogCount;
			const char* displayedCountStr = TextFormat(log.count <= maxLogCount ? "(%i)" : "(%i+)", displayedCount);
			bool shouldDisplayCount = log.count > 1;
			int displayedCountWidth = shouldDisplayCount ? MeasureText(displayedCountStr, 8) + 8 : 0;
			int indent = log.indent * logIndentWidth;
			int paddedIndent = indent + consolePaddingX;
			int logTypeWidthHere = logTypeWidth[log.type];

			DrawRectangle(logBoxXMin + 1 + indent, logBoxYMin + 2, logTypeWidthHere + displayedCountWidth - 2, lineHeight - 3, ColorAlpha(backgroundColor, 0.5f));

			DrawText(
				logTypeStr[log.type],
				logBoxXMin + paddedIndent,
				logBoxYMin + consolePaddingY,
				8,
				color);

			if (shouldDisplayCount)
			{
				DrawText(
					displayedCountStr,
					logBoxXMin + paddedIndent + logTypeWidthHere,
					logBoxYMin + consolePaddingY,
					8,
					color);
			}

			DrawText(
				log.content,
				logBoxXMin + paddedIndent + displayedCountWidth + logTypeWidthHere,
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

	void AppendLog(LogType type, const char* text, bool usesHeap = false)
	{
		if (totalLogs != 0) [[likely]] // Happens for all log-appends following the first after the console is cleared.
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
			// `droppedLog` is only valid in this context. 
			// It is going to be overwritten immediately in the first loop iteration.
			{
				LogElement& droppedLog = logs[0];
				if (droppedLog.contentUsesHeap)
				{
					delete[] droppedLog.content;
#if _DEBUG
					droppedLog.content = "[console: READING DELETED MEMORY]";
#endif
				}
			}

			for (size_t i = 1; i < totalLogs; ++i)
			{
				logs[i - 1] = logs[i];
			}
		}

		logs[totalLogs - 1] = {
			.type = type,
			.content = text,
			.indent = currentIndent,
			.count = 1,
			.lastHovered = GetTime(), // Creation is also be treated as hover (for stylistic reasons)
			.contentUsesHeap = usesHeap,
		};
	}

	void Log(const char* text)
	{
		AppendLog(LOGTYPE_NORMAL, text);
	}

	void Log(const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* text = Formatted(fmt, args);
		va_end(args);
		AppendLog(LOGTYPE_NORMAL, text);
	}

	void Warn(const char* text)
	{
		AppendLog(LOGTYPE_WARNING, text);
	}

	void Error(const char* text)
	{
		AppendLog(LOGTYPE_ERROR, text);
	}

	void Assert(bool condition, const char * text)
	{
		if (!condition)
		{
			AppendLog(LOGTYPE_FAILED_ASSERTION, text);
		}
	}

	void Group(const char* groupName)
	{
		AppendLog(LOGTYPE_NORMAL, groupName);
		++currentIndent;
	}

	void GroupEnd()
	{
		--currentIndent;
	}

	void Clear()
	{
		for (size_t i = 0; i < totalLogs; ++i)
		{
			LogElement& log = logs[i];
			if (log.contentUsesHeap)
			{
				delete[] log.content;
				log.contentUsesHeap = false;
#if _DEBUG
				log.content = "[console: READING DELETED MEMORY]";
#endif
			}
		}
		totalLogs = 0;
		currentIndent = 0;
	}

	void Unload()
	{
		Clear();
	}
}

#pragma warning( pop )
