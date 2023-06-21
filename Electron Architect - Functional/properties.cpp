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
		// The name of the property
		const char* name = nullptr;

		// Format - Only use when value is a pointer to a value and not a value
		const char* fmt = nullptr;

		// Assumed to be a c-string when fmt is null. Otherwise, assumed to be a pointer to a value specified by the format.
		// ! Should NEVER be nullptr if fmt is set.
		const void* value = nullptr;

		// Only use when value is a pointer to a value
		PropType type = PropType::Any;

		// Set anytime name changes (hopefully only once)
		int nameWidth = -1;

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

	int CountNewlines(const char* str)
	{
		int numLines = 1;
		char ch = '\0';
		for (; (ch = *str) != '\0'; ++str)
		{
			numLines += (int)(ch == '\n');
		}
		return numLines;
	}

	void DrawPanelContents(int mousex, int mousey, bool allowHover)
	{
		int y          = propertiesPanel.bounds.ymin + panel::panelTitlebarHeight + panelPaddingY;
		int xBaseline  = propertiesPanel.bounds.xmin + panelPaddingX;
		int xMax       = propertiesPanel.bounds.xmax - panelPaddingX;
		int indent     = 0;
		int halfHeight = fontSize / 2;

		constexpr Color accentColor = { 255,255,255, 32 }; // Default
		constexpr Color typeColor_Int = { 237,226,117, 96 }; // Yellow
		constexpr Color typeColor_Float = { 65,105,225, 96 }; // Blue
		constexpr Color typeColor_String = { 227,138,174, 96 }; // Pinkish
		constexpr Color typeColor_Bool = { 212,98,255, 96 }; // Purple
		constexpr Color typeColor_Any = { 1,1,1, 96 }; // Black

		for (size_t i = 0; i < numProps; ++i)
		{
			if (y > propertiesPanel.bounds.ymax)
			{
				break;
			}

			const Property& prop = props[i];
			int x = xBaseline + indent;

			char type = ((!!prop.name) << 1) | (!!prop.value);

			enum PropertyType : char {
				TYPE_CLOSER    = (char)(0b00),
				TYPE_MULTILINE = (char)(0b01),
				TYPE_HEADER    = (char)(0b10),
				TYPE_REGULAR   = (char)(0b11),
			};

			int numLines = 1;
			if (prop.value && !prop.fmt)
			{
				numLines = CountNewlines((const char*)prop.value);
			}
			else if (prop.value && prop.fmt && prop.type == PropType::String)
			{
				numLines = CountNewlines(*(const char* const*)prop.value);
			}
			int yNext = y + lineHeight * numLines;

			bool isDrawable = type != TYPE_CLOSER;

			bool isDrawSkippable = x > xMax && isDrawable;

			if (isDrawSkippable)
			{
				y = yNext;
				continue;
			}

			if (allowHover)
			{

				bool isHovering =
					x <= mousex && mousex <= xMax &&
					y <= mousey && mousey < yNext;

				if (isHovering && isDrawable)
				{
					DrawRectangle(x - 3, y - 2, (xMax - x) + 3, yNext - y - 3, accentColor);
				}
			}

			if (prop.name)
			{
				if (prop.nameWidth == -1) [[unlikely]]
				{
					props[i].nameWidth = MeasureText(prop.name, fontSize);
				}
				Color color = accentColor;
				if (type == TYPE_REGULAR)
				{
					switch (prop.type)
					{
					case PropType::Int:
						color = typeColor_Int;
						break;

					case PropType::Float:
						color = typeColor_Float;
						break;

					case PropType::String:
						color = typeColor_String;
						break;

					case PropType::Bool:
						color = typeColor_Bool;
						break;

					case PropType::Any:
						color = typeColor_Any;
						break;
					}
				}
				DrawRectangle(x - 3, y - 2, prop.nameWidth + 6, fontSize + 5, color);
			}

			switch (type)
			{
			case TYPE_CLOSER: {
				indent -= indentSize;
				{
					int xStart = x - indentSize;
					int xEnd = xMax;
					if (xEnd >= xStart)
					{
						DrawLine(xStart, y, xEnd, y, accentColor);
					}
				}
			} break;

			case TYPE_MULTILINE: {
				DrawText((const char*)prop.value, x, y, fontSize, WHITE);
				int numLines = 1;
				char ch = '\0';
				for (const char* str = (const char*)prop.value; (ch = *str) != '\0'; ++str)
				{
					numLines += (int)(ch == '\n');
				}
			} break;

			case TYPE_HEADER: {
				DrawText(prop.name, x, y, fontSize, WHITE);
				{
					int xStart = x + MeasureText(prop.name, fontSize) + 6;
					int xEnd = xMax;
					if (xEnd >= xStart)
					{
						int yMid = y + halfHeight;
						DrawLine(xStart, yMid, xEnd, yMid, accentColor);
					}
				}
				indent += indentSize;
			} break;

			case TYPE_REGULAR: {
				const char* valueStr = "";
				if (!prop.fmt)
				{
					valueStr = (const char*)prop.value;
				}
				else
				{
					switch (prop.type)
					{
					case PropType::Int:
						valueStr = TextFormat(prop.fmt, *(const int*)prop.value);
						break;

					case PropType::Float:
						valueStr = TextFormat(prop.fmt, *(const float*)prop.value);
						break;

					case PropType::String:
						valueStr = TextFormat(prop.fmt, *(const char* const*)prop.value);
						break;

					case PropType::Bool:
						valueStr = *(const bool*)prop.value ? "true" : "false";
						break;
					}
				}
				DrawText(prop.name, x, y, fontSize, WHITE);
				DrawText(valueStr, x + dividerX, y, fontSize, WHITE);
			} break;

			default: {
				console::Errorf("Type number was %i. Expected a number within [0b00..0b11] ([0..3])", type);
			}

			}
			y = yNext;
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

	// @param type Use `Property::Type::Any` if unsure or if the type changes
	void Addf(const char* name, PropType type, const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* valueStr = Formatted(fmt, args);
		va_end(args);
		_AddProperty({
			.name     = name,
			.fmt      = nullptr,
			.value    = valueStr,
			.type     = type,
			.usesHeap = true,
		});
	}

	void AddBool(const char* name, bool value)
	{
		_AddProperty({
			.name     = name,
			.fmt      = nullptr,
			.value    = value ? "true" : "false",
			.type     = PropType::Bool,
			.usesHeap = false,
		});
	}

	void AddString(const char* name, const char* valueStr)
	{
		_AddProperty({
			.name     = name,
			.fmt      = nullptr,
			.value    = valueStr,
			.type     = PropType::String,
			.usesHeap = false,
		});
	}

	void _AddLinked(const char* name, const char* fmt, PropType type, const void* valueSrcPtr)
	{
#if _DEBUG
		if (!valueSrcPtr || !fmt)
		{
			throw "Neither source pointer nor format can be null when adding a linked property!";
			return;
		}
#endif
		_AddProperty({
			.name     = name,
			.fmt      = fmt,
			.value    = valueSrcPtr,
			.type     = type,
			.usesHeap = false,
			});
	}

	void AddLinkedInt(const char* name, const char* fmt, const int* valueSrcPtr)
	{
		_AddLinked(name, fmt, PropType::Int, valueSrcPtr);
	}

	void AddLinkedFloat(const char* name, const char* fmt, const float* valueSrcPtr)
	{
		_AddLinked(name, fmt, PropType::Float, valueSrcPtr);
	}

	void AddLinkedString(const char* name, const char* fmt, const char* const* valueSrcPtr)
	{
		_AddLinked(name, fmt, PropType::String, valueSrcPtr);
	}

	void AddLinkedBool(const char* name, const bool* valueSrcPtr)
	{
		_AddLinked(name, "%s", PropType::Bool, valueSrcPtr);
	}

	void AddHeader(const char* name)
	{
		_AddProperty({
			.name     = name,
			.fmt      = nullptr,
			.value    = nullptr,
			.usesHeap = false
		});
	}

	void AddCloser()
	{
		_AddProperty({
			.name     = nullptr,
			.fmt      = nullptr,
			.value    = nullptr,
			.usesHeap = false,
		});
	}

	void AddMultiline(const char* name, const char* valueStr)
	{
		AddHeader(name);
		_AddProperty({
			.name     = nullptr,
			.fmt      = nullptr,
			.value    = valueStr,
			.type     = PropType::String,
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
