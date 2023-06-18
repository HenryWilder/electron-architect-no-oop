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
		enum class Type
		{
			Int,
			Float,
			String,
			Header,
			Closer, // Marker indicating the end of a collection
		} type;

		union
		{
			long long intValue;
			double floatValue;
			const char* stringValue;
		};

		// Name of the property (nullptr if anonymous - only for headers and closers)
		const char* name;
		// Pointer to the collection header acting as this property's parent
		Property* parent;
		size_t indent;
	};

	constexpr size_t MAX_HEADERS = 256;
	constexpr size_t MAX_PROPS = 1024;
	Property props[MAX_PROPS] = {};
	int numProps = 0;

	constexpr int lineHeight = 16;
	constexpr int indentSize = 16;

	void DrawPanelContents()
	{
		int y = propertiesPanel.bounds.ymin;
		int xBaseline = propertiesPanel.bounds.xmin;
		int indent = 0;

		for (size_t i = 0; i < numProps; ++i)
		{
			const Property& prop = props[i];
			int x = xBaseline + indent;

			if (prop.type == Property::Type::Closer)
			{
				indent -= indentSize;
				continue; // Don't iterate property line
			}
			else
			{
				DrawText(prop.name, x, y, 8, WHITE);
			}

			switch (prop.type)
			{
			case Property::Type::Int:
				break;

			case Property::Type::Float:
				break;

			case Property::Type::String:
				break;

			case Property::Type::Header:
				break;

			case Property::Type::Closer:
				break;
			}

			if (prop.type == Property::Type::Header)
			{
				indent += indentSize;
			}

			y += lineHeight;
		}
	}

	void AddProperty(const char* name, long long value)
	{
		if (numProps >= MAX_PROPS)
		{
			throw "too many properties";
		}

		props[numProps++] = {.type = Property::Type::Int, .intValue = value, .parent = nullptr, .indent = 0};
	}

	void ClearProperties()
	{
		numProps = 0;
	}
}
