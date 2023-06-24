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
	// If name is nullptr but not valueStr, this is INVALID
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

		// Only meaningful on values
		PropValueType valueType = PropValueType::Any;

		// Only meaningful on headers
		PropCollectionType collectionType = PropCollectionType::Object;

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
	int dividerX = 150;
	// Width of the divider visual
	constexpr int dividerWidth = 3;

	int xMax;

	constexpr int indentSize = 8;

	constexpr int panelPaddingX  = 6;
	constexpr int panelPaddingY  = 5;
	constexpr int fontSize       = 8;
	constexpr int halfHeight     = fontSize / 2;
	constexpr int halfFontToLine = (lineHeight - fontSize) / 2 + 1; // Half of the difference between the font height and the line height

	constexpr int propertyPaddingL = 3;
	constexpr int propertyPaddingR = 3;
	constexpr int propertyPaddingT = 2;
	constexpr int propertyPaddingB = -halfFontToLine;

	constexpr Color accentColor      = { 255,255,255, 32 }; // Default

	constexpr Color typeColor_Bool   = { 144,  0,  2,127 }; // Maroon
	constexpr Color typeColor_Byte   = {   0,109,102,127 }; // Sherpa blue
	constexpr Color typeColor_Int    = {  44,222,174,127 }; // Sea green
	constexpr Color typeColor_Float  = { 157,255, 63,127 }; // Yellow green
	constexpr Color typeColor_String = { 248,  3,204,127 }; // Magenta
	constexpr Color typeColor_Any    = {  10, 10, 10,127 }; // Black

	constexpr Color typeColor_Object = {   0,163,230,127 }; // Blue
	constexpr Color typeColor_Array  = { 252,200, 39,127 }; // Gold
	constexpr Color typeColor_Map    = {   0, 88,196,127 }; // Dark Blue

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
					int yUpper = yMid;
					int yLower = yMid + 3;
					DrawLine(xStart, yUpper, xEnd, yUpper, accentColor);
					DrawLine(xStart, yUpper, xStart, yLower, accentColor);
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
				int halfwayY = y + lineHeight / 2;
				DrawLine(xStart, halfwayY, xEnd, halfwayY, accentColor);
			}
		}
	}

	void DrawRegularProperty(int x, int y, const Property& prop, int dividerXEnd)
	{
		const char* valueStr = "";
		if (!prop.fmt)
		{
			valueStr = (const char*)prop.value;
		}
		else
		{
			switch (prop.valueType)
			{
			case PropValueType::Int:    valueStr = TextFormat(prop.fmt, *(const   int*       )prop.value); break;
			case PropValueType::Float:  valueStr = TextFormat(prop.fmt, *(const float*       )prop.value); break;
			case PropValueType::String: valueStr = TextFormat(prop.fmt, *(const  char* const*)prop.value); break;
			case PropValueType::Bool:   valueStr = *(const bool*)prop.value ? "true" : "false"; break;
			}
		}
		DrawText(prop.name, x, y, fontSize, WHITE);
		DrawText(valueStr, dividerXEnd, y, fontSize, WHITE);
	}

	enum class PropertyType : char {
		Closer    = '\0',
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

		// End of anything left of the divider 
		int dividerXStart = clientBounds.xmin + dividerX - panelPaddingX;
		// Start of anything right of the divider
		int dividerXEnd = dividerXStart + dividerWidth + panelPaddingX;

		int y          = clientBounds.ymin + panelPaddingY - scrollY;
		int xBaseline  = clientBounds.xmin + panelPaddingX;
		int indent     = 0;

		xMax = clientBounds.xmax - panelPaddingX;

		// When a collection is collapsed, hide everything deeper than this until we reach a closer with indentation.
		// Set to -1 when uncollapsed.
		int hideUntilIndent = -1;

		PropertyType typePrev = PropertyType(-1);

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
			else if (prop.value && prop.fmt && prop.valueType == PropValueType::String)
			{
				numLines = CountNewlines(*(const char* const*)prop.value);
			}
			int yNext = y + lineHeight * numLines;

			// Indentation lines
			if (indent > 0)
			{
				int yStart = y;
				int yEnd = yNext;
				int drawIndent = indent;

				// First line
				{
					if (type == PropertyType::Closer)
					{
						yEnd -= lineHeight / 2;
					}
					if (typePrev == PropertyType::Header && !props[i - 1].isCollapsed)
					{
						yStart -= halfFontToLine;
					}

					int drawIndentComplete = xBaseline + drawIndent - indentSize;
					DrawLine(drawIndentComplete, yStart, drawIndentComplete, yEnd, accentColor);
					drawIndent -= indentSize;
				}

				for (; drawIndent > 0; drawIndent -= indentSize)
				{
					int drawIndentComplete = xBaseline + drawIndent - indentSize;
					DrawLine(drawIndentComplete, y, drawIndentComplete, yNext, accentColor);
				}
			}
			typePrev = type;

			bool isHoverableProperty = type != PropertyType::Closer;

			Color color = accentColor;
			if (type == PropertyType::Regular)
			{
				switch (prop.valueType)
				{
				case PropValueType::Bool:   color = typeColor_Bool;   break;
				case PropValueType::Byte:   color = typeColor_Byte;   break;
				case PropValueType::Int:    color = typeColor_Int;    break;
				case PropValueType::Float:  color = typeColor_Float;  break;
				case PropValueType::String: color = typeColor_String; break;
				case PropValueType::Any:    color = typeColor_Any;    break;
				}
			}
			else if (type == PropertyType::Header)
			{
				switch (prop.collectionType)
				{
				case PropCollectionType::Object: color = typeColor_Object; break;
				case PropCollectionType::Array:  color = typeColor_Array;  break;
				case PropCollectionType::Map:    color = typeColor_Map;    break;
				}
			}

			bool isHovering =
				x <= mousex && mousex <= xMax &&
				y <= mousey && mousey < yNext;

			if (isHoverableProperty)
			{
				if (allowHover && isHovering) [[unlikely]] // Even if hover is allowed and true, only one property will ever be hovered at a time.
				{
					int paddedXMin = x - propertyPaddingL;
					int paddedYMin = y - propertyPaddingT;
					int paddedXMax = xMax + propertyPaddingR;
					int paddedYMax = yNext + propertyPaddingB;

					int paddedW = paddedXMax - paddedXMin;
					int paddedH = paddedYMax - paddedYMin;

					DrawRectangle(paddedXMin, paddedYMin, paddedW, paddedH, color);

					if (type == PropertyType::Header && isPressed)
					{
						console::Log("Header collapse toggled");
						props[i].isCollapsed = !prop.isCollapsed;
					}
				}
				else if (prop.name) [[likely]] // Only closers won't have a name
				{
					if (prop.nameWidth == -1) [[unlikely]]
					{
						props[i].nameWidth = MeasureText(prop.name, fontSize);
					}

					int paddedXMin = x - propertyPaddingL;
					int paddedYMin = y - propertyPaddingT;
					int paddedXMax = x + prop.nameWidth + propertyPaddingR;
					int paddedYMax = yNext + propertyPaddingB;

					int paddedW = paddedXMax - paddedXMin;
					int paddedH = paddedYMax - paddedYMin;

					DrawRectangle(paddedXMin, paddedYMin, paddedW, paddedH, color);
				}
			}

			if (type == PropertyType::Header && prop.isCollapsed)
			{
				hideUntilIndent = indent;
			}

			switch (type)
			{
			case PropertyType::Closer:    DrawCloserProperty   (indent, x, y                   ); break;
			case PropertyType::Header:    DrawHeaderProperty   (indent, x, y, prop             ); break;
			case PropertyType::Regular:   DrawRegularProperty  (        x, y, prop, dividerXEnd); break;
			}

			y = yNext;
		}

		int panelX = clientBounds.xmin;
		int panelY = clientBounds.ymin;
		int panelH = clientBounds.ymax - panelY;
		DrawRectangle(dividerXStart, panelY, dividerWidth, panelH, accentColor);
	}

#pragma endregion

#pragma region Property list manipulation

	void _AddProperty(Property newProperty)
	{
		if (numProps == MAX_PROPS)
		{
			console::Error("Too many properties, out of property memory");
			throw "Too many properties, out of property memory";
			return; // In case someone tries to continue anyway
		}

		props[numProps++] = newProperty;
	}

	// @param type Use `Any` if unsure or if the type changes
	void Addf_hint(const char* name, size_t hintSizeMax, PropValueType type, const char* fmt...)
	{
		va_list args;
		va_start(args, fmt);
		char* valueStr = Formatted(hintSizeMax, fmt, args);
		va_end(args);
		_AddProperty({
			.name      = name,
			.fmt       = nullptr,
			.value     = valueStr,
			.valueType = type,
			.usesHeap  = true,
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
			.name      = name,
			.fmt       = nullptr,
			.value     = valueStr,
			.valueType = type,
			.usesHeap  = true,
		});
	}

	void AddBool(const char* name, bool value)
	{
		_AddProperty({
			.name      = name,
			.fmt       = nullptr,
			.value     = value ? "true" : "false",
			.valueType = PropValueType::Bool,
			.usesHeap  = false,
		});
	}

	void AddString(const char* name, const char* valueStr)
	{
		_AddProperty({
			.name      = name,
			.fmt       = nullptr,
			.value     = valueStr,
			.valueType = PropValueType::String,
			.usesHeap  = false,
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
			.name      = name,
			.fmt       = fmt,
			.value     = valueSrcPtr,
			.valueType = type,
			.usesHeap  = false,
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

	void AddHeader(const char* name, PropCollectionType type)
	{
		_AddProperty({
			.name           = name,
			.fmt            = nullptr,
			.value          = nullptr,
			.collectionType = type,
			.usesHeap       = false,
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
