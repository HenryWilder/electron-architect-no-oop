#include <raylib.h>
#include <raymath.h>
#include "utils.hpp"
extern "C"
{
#include "logtypes.h"
}
#include "console.hpp"
#pragma warning( push )
#pragma warning( disable : 26812 )

namespace console
{
	constexpr int consolePaddingX = 4;
	constexpr int consolePaddingY = 4;

	constexpr int lineHeight = 16;
	constexpr int logIndentWidth = 16;
	size_t displayableLogCount = 0; // Update when panel size changes

	extern LogElement logs[maxLogs];
	extern size_t totalLogs;
	extern size_t currentIndent;

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
				logBoxYMin <= mousey && mousey < logBoxYMin + lineHeight) // y is < instead of <= so that mouse doesn't overlap multiple elements
			{
				LogElement& logRef = logs[logIndex];
				logRef.lastHovered = GetTime();
			}

			float minBgAlpha = backgroundHoverAlpha - ((log.type == LOGTYPE_NORMAL) ? backgroundHoverAlpha : 0.0f);
			float maxBgAlpha = backgroundHoverAlpha * 2 - ((log.type == LOGTYPE_NORMAL) ? backgroundHoverAlpha : 0.0f);

			float backgroundAlpha = (float)AnimatedFade(GetTime() - log.lastHovered, logFadeTime, maxBgAlpha, minBgAlpha);

			DrawRectangle(logBoxXMin, logBoxYMin, logBoxXMax - logBoxXMin, lineHeight, ColorAlpha(backgroundColor, backgroundAlpha));

			if (logTypeWidth[log.type] == 0) [[unlikely]] // Should only happen once per unique log type
			{
				logTypeWidth[log.type] = MeasureText(logTypeStr[log.type], 8) + 7;
			}

			size_t displayedCount = log.count <= maxLogCount ? log.count : maxLogCount;
			const char* displayedCountStr = TextFormat(log.count <= maxLogCount ? "(%i)" : "(%i+)", displayedCount);
			bool shouldDisplayCount = log.count > 1;
			int displayedCountWidth = shouldDisplayCount ? MeasureText(displayedCountStr, 8) + 8 : 0;
			int indent = (int)log.indent * logIndentWidth;
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
}

#pragma warning( pop )
