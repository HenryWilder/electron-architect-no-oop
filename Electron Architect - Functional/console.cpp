#include <cstdarg>
#include "textfmt.hpp"
#include "console.hpp"
#include "properties.hpp"

namespace console
{
	panel::Panel consolePanel =
	{
		.title = "Console",
		.id = panel::PanelID::Console,
		.bounds = {
			&panel::windowBounds.xmin,
			500,
			&properties::propertiesPanel.bounds.xmin,
			&panel::windowBounds.ymax
		},
		.draggable = (panel::DraggableEdges)((int)panel::DraggableEdges::EdgeT | (int)panel::DraggableEdges::EdgeR)
	};

	LogElement logs[maxLogs];
	size_t totalLogs = 0; // At most maxLogs
	size_t currentIndent = 0;

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

	void AppendLog(LogType type, const char* text, bool usesHeap)
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

	// Do NOT use this if `text` uses heap memory and you aren't freeing it yourself!
	void Log(const char* text)
	{
		AppendLog(LOGTYPE_NORMAL, text, false);
	}

	void Logf(const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* text = Formatted(fmt, args);
		va_end(args);
		AppendLog(LOGTYPE_NORMAL, text, true);
	}

	// Do NOT use this if `text` uses heap memory and you aren't freeing it yourself!
	void Warn(const char* text)
	{
		AppendLog(LOGTYPE_WARNING, text, false);
	}

	void Warnf(const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* text = Formatted(fmt, args);
		va_end(args);
		AppendLog(LOGTYPE_WARNING, text, true);
	}

	// Do NOT use this if `text` uses heap memory and you aren't freeing it yourself!
	void Error(const char* text)
	{
		AppendLog(LOGTYPE_ERROR, text, false);
	}

	void Errorf(const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* text = Formatted(fmt, args);
		va_end(args);
		AppendLog(LOGTYPE_ERROR, text, true);
	}

	// Do NOT use this if `text` uses heap memory and you aren't freeing it yourself!
	void Assert(bool condition, const char * text)
	{
		if (!condition)
		{
			AppendLog(LOGTYPE_FAILED_ASSERTION, text, false);
		}
	}

	void Assertf(bool condition, const char* fmt...)
	{
		if (!condition)
		{
			va_list args;
			va_start(args, fmt);
			char* text = Formatted(fmt, args);
			va_end(args);
			AppendLog(LOGTYPE_FAILED_ASSERTION, text, true);
		}
	}

	void Group(const char* groupName)
	{
		AppendLog(LOGTYPE_GROUP, groupName, false);
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
