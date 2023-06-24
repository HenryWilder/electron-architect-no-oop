#include <cstdarg>
#include "textfmt.hpp"
#include "console.hpp"
#include "properties.hpp"

namespace properties
{
	panel::Panel propertiesPanel = {
		.title = "Properties",
		.id = panel::PanelID::Properties,
		.bounds = {
			1000,
			&panel::windowBounds.ymin,
			&panel::windowBounds.xmax,
			&panel::windowBounds.ymax,
		},
		.draggable = panel::DraggableEdges::EdgeL
	};

#pragma region Property storage

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
		PropValueType type = PropValueType::Any;

		// Set anytime name changes (hopefully only once)
		int nameWidth = -1;

		// Whether the valueStr content is heap memory and needs to be freed before overwriting or closing the program
		// Technically, it just means whether it is the properties panel's duty to free it.
		// Heap memory may still be passed in; but if it wasn't allocated by the properties panel, it won't be freed by the properties panel.
		// ! Should not be used if fmt is non-null
		bool usesHeap = false;

		// Only meaningful on headers
		// If true, hide everything until the matching closer
		bool isCollapsed = true;
	};

	constexpr size_t MAX_PROPS = 1024;
	Property props[MAX_PROPS] = {};
	int numProps = 0;

#pragma endregion

#pragma region Rendering

	int scrollY = 0;

	// Position of the divider (offset from left side)
	// Todo: Make this draggable
	int dividerX = 100;

	// Recalculates when panel window is resized
	int xMax;

	constexpr int lineHeight = 16;
	constexpr int indentSize = 8;

	constexpr int panelPaddingX = 6;
	constexpr int panelPaddingY = 5;
	constexpr int fontSize      = 8;
	constexpr int halfHeight    = fontSize / 2;

	constexpr Color accentColor      = { 255,255,255, 32 }; // Default
	constexpr Color typeColor_Int    = { 237,226,117, 96 }; // Yellow
	constexpr Color typeColor_Float  = {  65,105,225, 96 }; // Blue
	constexpr Color typeColor_String = { 227,138,174, 96 }; // Pinkish
	constexpr Color typeColor_Bool   = { 212, 98,255, 96 }; // Purple
	constexpr Color typeColor_Any    = {   1,  1,  1, 96 }; // Black

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

	void DrawHeaderProperty(int& indent, int x, int y, const Property& prop)
	{
		DrawText(prop.name, x, y, fontSize, WHITE);
		{
			int xStart = x + MeasureText(prop.name, fontSize) + 6;
			int xEnd = xMax;
			if (xEnd >= xStart)
			{
				int yMid = y + halfHeight;
				if (prop.isCollapsed)
				{
					int yUpper = yMid - 2;
					int yLower = yMid + 2;
					DrawLine(xStart, yUpper, xEnd, yUpper, accentColor);
					DrawLine(xStart, yLower, xEnd, yLower, accentColor);
				}
				else
				{
					DrawLine(xStart, yMid, xEnd, yMid, accentColor);
				}
			}
		}
		indent += indentSize;
	}

	void DrawCloserProperty(int& indent, int x, int y)
	{
		indent -= indentSize;
		{
			int xStart = x - indentSize;
			int xEnd = xMax;
			if (xEnd >= xStart)
			{
				DrawLine(xStart, y, xEnd, y, accentColor);
			}
		}
	}

	void DrawRegularProperty(int x, int y, const Property& prop)
	{
		const char* valueStr = "";
		if (!prop.fmt)
		{
			valueStr = (const char*)prop.value;
		}
		else
		{
			switch (prop.type)
			{
			case PropValueType::Int:    valueStr = TextFormat(prop.fmt, *(const   int*       )prop.value); break;
			case PropValueType::Float:  valueStr = TextFormat(prop.fmt, *(const float*       )prop.value); break;
			case PropValueType::String: valueStr = TextFormat(prop.fmt, *(const  char* const*)prop.value); break;
			case PropValueType::Bool:   valueStr = *(const bool*)prop.value ? "true" : "false"; break;
			}
		}
		DrawText(prop.name, x, y, fontSize, WHITE);
		DrawText(valueStr, x + dividerX, y, fontSize, WHITE);
	}

	void DrawMultilineProperty(int x, int y, const Property& prop)
	{
		DrawText((const char*)prop.value, x, y, fontSize, WHITE);
		int numLines = 1;
		char ch = '\0';
		for (const char* str = (const char*)prop.value; (ch = *str) != '\0'; ++str)
		{
			numLines += (int)(ch == '\n');
		}
	}

	enum class PropertyType : char {
		Closer    = '\0',
		Multiline = '\1',
		Header    = '\2',
		Regular   = '\3',
	};

	void DrawPanelContents(int mousex, int mousey, bool allowHover, bool isPressed)
	{
#if _DEBUG
		// Check for unbalanced collections
		{
			int n = 0;
			for (size_t i = 0; i < numProps; ++i)
			{
				const Property& prop = props[i];
				bool isHeader =  prop.name && !prop.value;
				bool isCloser = !prop.name && !prop.value;

				n += (int)isHeader - (int)isCloser;
			}
			if (n > 0)
			{
				console::Error("Imbalanced properties: More headers than closers");
			}
			else if (n < 0)
			{
				console::Error("Imbalanced properties: More closers than headers");
			}
		}
#endif

		panel::Bounds clientBounds = panel::PanelClientBounds(propertiesPanel);

		int y          = clientBounds.ymin + panelPaddingY - scrollY;
		int xBaseline  = clientBounds.xmin + panelPaddingX;
		int indent     = 0;

		xMax = clientBounds.xmax - panelPaddingX;

		// When a collection is collapsed, hide everything deeper than this until we reach a closer with indentation.
		// Set to -1 when uncollapsed.
		int hideUntilIndent = -1;

		for (size_t i = 0; i < numProps; ++i)
		{
			if (y > clientBounds.ymax)
			{
				break;
			}

			const Property& prop = props[i];
			PropertyType type = (PropertyType)((((char)!!prop.name) << 1) | ((char)!!prop.value));

			if (hideUntilIndent != -1)
			{
				if (type == PropertyType::Header)
				{
					indent += indentSize;
				}
				if (type == PropertyType::Closer)
				{
					indent -= indentSize;
					if (indent == hideUntilIndent)
					{
						hideUntilIndent = -1;
					}
				}
				continue;
			}

			int x = xBaseline + indent;

			int numLines = 1;
			if (prop.value && !prop.fmt)
			{
				numLines = CountNewlines((const char*)prop.value);
			}
			else if (prop.value && prop.fmt && prop.type == PropValueType::String)
			{
				numLines = CountNewlines(*(const char* const*)prop.value);
			}
			int yNext = y + lineHeight * numLines;

			bool isDrawable = type != PropertyType::Closer;

			bool isDrawSkippable = x > xMax && isDrawable;

			if (isDrawSkippable)
			{
				y = yNext;
				continue;
			}

			Color color = accentColor;
			if (type == PropertyType::Regular)
			{
				switch (prop.type)
				{
				case PropValueType::Int:    color = typeColor_Int;    break;
				case PropValueType::Float:  color = typeColor_Float;  break;
				case PropValueType::String: color = typeColor_String; break;
				case PropValueType::Bool:   color = typeColor_Bool;   break;
				case PropValueType::Any:    color = typeColor_Any;    break;
				}
			}

			if (allowHover)
			{
				bool isHovering =
					x <= mousex && mousex <= xMax &&
					y <= mousey && mousey < yNext;

				if (isHovering && isDrawable)
				{
					DrawRectangle(x - 3, y - 2, (xMax - x) + 3, yNext - y - 3, color);

					if (type == PropertyType::Header && isPressed)
					{
						console::Log("Header collapse toggled");
						props[i].isCollapsed = !prop.isCollapsed;
					}
				}
			}

			if (type == PropertyType::Header && prop.isCollapsed)
			{
				hideUntilIndent = indent;
			}

			if (prop.name)
			{
				if (prop.nameWidth == -1) [[unlikely]]
				{
					props[i].nameWidth = MeasureText(prop.name, fontSize);
				}
				DrawRectangle(x - 3, y - 2, prop.nameWidth + 6, fontSize + 5, color);
			}

			switch (type)
			{
			case PropertyType::Closer:    DrawCloserProperty   (indent, x, y      ); break;
			case PropertyType::Multiline: DrawMultilineProperty(        x, y, prop); break;
			case PropertyType::Header:    DrawHeaderProperty   (indent, x, y, prop); break;
			case PropertyType::Regular:   DrawRegularProperty  (        x, y, prop); break;
			}
			y = yNext;
		}
	}

#pragma endregion

#pragma region Property list manipulation

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
	void Addf_hint(const char* name, size_t hintSizeMax, PropValueType type, const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* valueStr = Formatted(hintSizeMax, fmt, args);
		va_end(args);
		_AddProperty({
			.name     = name,
			.fmt      = nullptr,
			.value    = valueStr,
			.type     = type,
			.usesHeap = true,
		});
	}

	// @param type Use `Property::Type::Any` if unsure or if the type changes
	void Addf(const char* name, PropValueType type, const char* fmt...)
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
			.type     = PropValueType::Bool,
			.usesHeap = false,
		});
	}

	void AddString(const char* name, const char* valueStr)
	{
		_AddProperty({
			.name     = name,
			.fmt      = nullptr,
			.value    = valueStr,
			.type     = PropValueType::String,
			.usesHeap = false,
		});
	}

	void _AddLinked(const char* name, const char* fmt, PropValueType type, const void* valueSrcPtr)
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
		_AddLinked(name, fmt, PropValueType::Int, valueSrcPtr);
	}

	void AddLinkedFloat(const char* name, const char* fmt, const float* valueSrcPtr)
	{
		_AddLinked(name, fmt, PropValueType::Float, valueSrcPtr);
	}

	void AddLinkedString(const char* name, const char* fmt, const char* const* valueSrcPtr)
	{
		_AddLinked(name, fmt, PropValueType::String, valueSrcPtr);
	}

	void AddLinkedBool(const char* name, const bool* valueSrcPtr)
	{
		_AddLinked(name, "%s", PropValueType::Bool, valueSrcPtr);
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
			.type     = PropValueType::String,
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

#pragma endregion
}
