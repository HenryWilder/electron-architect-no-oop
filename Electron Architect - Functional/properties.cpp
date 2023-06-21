#include <cstdarg>
#include "textfmt.hpp"
#include "console.hpp"
#include "properties.hpp"

namespace properties
{
	panel::Panel propertiesPanel = {
		"Properties",
		{
			1000,
			&panel::windowBounds.ymin,
			&panel::windowBounds.xmax,
			&console::consolePanel.bounds.ymin
		},
		(panel::DraggableEdges)((int)panel::DraggableEdges::EdgeL | (int)panel::DraggableEdges::EdgeB)
	};

	// If neither name nor valueStr is nullptr, this is a regular property.
	// If valueStr is nullptr but not name, this is a header.
	// If name is nullptr but not valueStr, this is a multiline property.
	// If both name and valueStr are nullptr, this is a closer.
	struct Property
	{
		const char* name  = nullptr;
		const char* fmt   = nullptr;
		const void* value = nullptr; // Assumed to be a c-string when fmt is null. Otherwise, assumed to be a pointer to a value specified by the format

		// Whether the valueStr content is heap memory and needs to be freed before overwriting or closing the program
		// Technically, it just means whether it is the properties panel's duty to free it.
		// Heap memory may still be passed in; but if it wasn't allocated by the properties panel, it won't be freed by the properties panel.
		// ! Should not be used if fmt is non-null
		bool usesHeap = false;
	};

	constexpr size_t MAX_PROPS = 1024;
	Property props[MAX_PROPS]  = {};
	int numProps               = 0;

	// Position of the divider (offset from left side)
	// Todo: Make this draggable
	int dividerX = 100;

	constexpr int lineHeight = 16;
	constexpr int indentSize = 8;

	constexpr int panelPaddingX = 6;
	constexpr int panelPaddingY = 4;
	constexpr int fontSize      = 8;

	void DrawPanelContents()
	{
		int y          = propertiesPanel.bounds.ymin + panel::panelTitlebarHeight + panelPaddingY;
		int xBaseline  = propertiesPanel.bounds.xmin + panelPaddingX;
		int indent     = 0;
		int halfHeight = fontSize / 2;

		// Pass 1: Names
		for (size_t i = 0; i < numProps; ++i)
		{
			const Property& prop = props[i];
			int x = xBaseline + indent;

			// Closer
			if (!prop.name && !prop.value)
			{
				indent -= indentSize;
				continue; // Don't iterate property line
			}

			// Regular
			else if (prop.name && prop.value)
			{
				DrawText(prop.name, x, y, fontSize, WHITE);
				DrawText((const char*)prop.value, x + dividerX, y, fontSize, WHITE);
			}

			// Multiline
			else if (!prop.name && prop.value)
			{
				DrawText((const char*)prop.value, x, y, fontSize, WHITE);
			}

			// Header
			else if (prop.name && !prop.value)
			{
				int yMid = y + halfHeight;
				DrawText(prop.name, x, y, fontSize, WHITE);
				int xStart = x + MeasureText(prop.name, fontSize) + panelPaddingX;
				int xEnd = propertiesPanel.bounds.xmax - panelPaddingX;
				if (xEnd >= xStart)
				{
					DrawLine(xStart, yMid, xEnd, yMid, WHITE);
				}
				indent += indentSize;
			}

			else
			{
				console::Error(TextFormat("Undefined situation: name is \"%s\", value is \"%s\"", prop.name, prop.value));
			}

			y += lineHeight;
		}
	}

	void _Add(Property newProperty)
	{
		if (numProps == MAX_PROPS)
		{
			throw "Too many properties, out of property memory";
			return; // In case someone tries to continue anyway
		}

		props[numProps++] = newProperty;
	}

	void Add(const char* name, const char* valueStr)
	{
		_Add({
			.name     = name,
			.fmt      = nullptr,
			.value    = valueStr,
			.usesHeap = false,
		});
	}

	void Addf(const char* name, const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* value= Formatted(fmt, args);
		va_end(args);
		_Add({
			.name     = name,
			.fmt      = nullptr,
			.value    = value,
			.usesHeap = true,
		});
	}

	void AddHeader(const char* name)
	{
		_Add({
			.name     = name,
			.fmt      = nullptr,
			.value    = nullptr,
			.usesHeap = false
		});
	}

	void AddCloser()
	{
		_Add({
			.name     = nullptr,
			.fmt      = nullptr,
			.value    = nullptr,
			.usesHeap = false
		});
	}

	void AddMultiline(const char* name, const char* valueStr)
	{
		AddHeader(name);
		_Add({
			.name     = nullptr,
			.fmt      = nullptr,
			.value    = valueStr,
			.usesHeap = false
		});
		AddCloser();
	}

	void Clear()
	{
		for (Property& prop : props)
		{
			if (prop.usesHeap)
			{
				delete[] prop.value;
#if _DEBUG
				prop.value = "[properties: READING DELETED MEMORY]";
				prop.usesHeap = false;
#endif
			}
		}
		numProps = 0;
	}

	void Unload()
	{
		Clear();
	}
}
