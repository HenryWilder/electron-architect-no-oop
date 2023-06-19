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

	struct Property
	{
		const char* name     = nullptr; // nullptr for closers
		const char* valueStr = nullptr; // nullptr for headers

		bool usesHeap = false; // If true: Needs to be freed when clearing the properties console or updating the value string.
		// Reasoning: The alternatives are storing the pointers in a map (which would require find()-ing any time we want to replace the value),
		// or storing an unknown number of variables of unknown types and redundantly reformatting every frame just in case TextFormat() runs out of buffer.
	};

	constexpr size_t MAX_PROPS = 1024;
	Property props[MAX_PROPS] = {};
	int numProps = 0;

	// Todo: Implement this
	size_t showingPropertiesFor = NULL;

	// Position of the divider (offset from left side)
	// Todo: Make this draggable
	int dividerX = 100;

	constexpr int lineHeight = 16;
	constexpr int indentSize = 8;

	constexpr int panelPaddingX = 6;
	constexpr int panelPaddingY = 4;

	void DrawPanelContents()
	{
		int y = propertiesPanel.bounds.ymin + panel::panelTitlebarHeight + panelPaddingY;
		int xBaseline = propertiesPanel.bounds.xmin + panelPaddingX;
		int indent = 0;

		// Pass 1: Names
		for (size_t i = 0; i < numProps; ++i)
		{
			const Property& prop = props[i];
			int x = xBaseline + indent;

			if (!prop.name)
			{
				indent -= indentSize;
				continue; // Don't iterate property line
			}

			if (prop.name)
			{
				DrawText(prop.name, x, y, 8, WHITE);
			}

			if (prop.valueStr)
			{
				DrawText(prop.valueStr, x + dividerX, y, 8, WHITE);
			}

			if (prop.name && !prop.valueStr)
			{
				indent += indentSize;
			}

			y += lineHeight;
		}
	}

	void _AddProperty(Property newProperty)
	{
		if (numProps == MAX_PROPS)
		{
			throw "Too many properties, out of property memory";
			return; // In case someone tries to continue anyway
		}

		props[numProps++] = newProperty;
	}

	void AddProperty(const char* name, const char* valueStr)
	{
		_AddProperty({ name, valueStr, false });
	}

	void AddPropertyHeader(const char* name)
	{
		_AddProperty({ name, nullptr, false });
	}

	void AddPropertyCloser()
	{
		_AddProperty({ nullptr, nullptr, false });
	}

	void ClearProperties()
	{
		for (Property& prop : props)
		{
			if (prop.usesHeap)
			{
				delete prop.valueStr;
#if _DEBUG
				prop.valueStr = "[USING DELETED MEMORY]";
				prop.usesHeap = false;
#endif
			}
		}
		numProps = 0;
	}
}
