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

	// Todo: Implement this
	size_t showingPropertiesFor = NULL;

	// Position of the divider (offset from left side)
	// Todo: Make this draggable
	int dividerX = 100;

	constexpr int lineHeight = 16;
	constexpr int indentSize = 16;

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

			if (prop.type == Property::Type::Closer)
			{
				indent -= indentSize;
				continue; // Don't iterate property line
			}

			if (prop.name)
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

		// Pass 2: Values
		for (size_t i = 0; i < numProps; ++i)
		{
			const Property& prop = props[i];
			int x = xBaseline + indent;

			if (prop.type == Property::Type::Closer)
			{
				indent -= indentSize;
				continue; // Don't iterate property line
			}

			if (prop.name)
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

		DrawRectangle();

	}

#if _DEBUG
#define PROP_COUNT_GUARD if (numProps >= MAX_PROPS) { [[unlikely]] throw "too many properties"; }
#else
#define PROP_COUNT_GUARD
#endif

	void AddIntProperty(const char* name, long long value)
	{
		PROP_COUNT_GUARD
		props[numProps++] = {
			.type = Property::Type::Int,
			.intValue = value,
			.name = name,
			.parent = nullptr,
			.indent = 0
		};
	}

	void AddFloatProperty(const char* name, double value)
	{
		PROP_COUNT_GUARD
		props[numProps++] = {
			.type = Property::Type::Float,
			.floatValue = value,
			.name = name,
			.parent = nullptr,
			.indent = 0
		};
	}

	void AddStringProperty(const char* name, const char* value)
	{
		PROP_COUNT_GUARD
		props[numProps++] = {
			.type = Property::Type::String,
			.stringValue = value,
			.name = name,
			.parent = nullptr,
			.indent = 0
		};
	}

	void AddPropertyCollectionHeader(const char* name, const char* collectionName)
	{
		PROP_COUNT_GUARD
		props[numProps++] = {
			.type = Property::Type::Header,
			.stringValue = collectionName,
			.name = name,
			.parent = nullptr,
			.indent = 0
		};
	}

	void AddTopLevelPropertyCollectionHeader(const char* collectionName)
	{
		PROP_COUNT_GUARD
			props[numProps++] = {
			.type = Property::Type::Header,
			.stringValue = collectionName,
			.name = nullptr,
			.parent = nullptr,
			.indent = 0
		};
	}

	void AddPropertyCollectionCloser()
	{
		PROP_COUNT_GUARD
		props[numProps++] = {
			.type = Property::Type::Closer,
			.stringValue = nullptr,
			.name = nullptr,
			.parent = nullptr,
			.indent = 0
		};
	}

	void ClearProperties()
	{
		numProps = 0;
	}
}
