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
		indent += indentSize;
	}

	void DrawCloserProperty(int& indent, int x, int y)
	{
		indent -= indentSize;
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

	Color GetValueAccentColor(PropValueType type)
	{
		switch (type)
		{
		case PropValueType::Bool:   return typeColor_Bool;
		case PropValueType::Byte:   return typeColor_Byte;
		case PropValueType::Int:    return typeColor_Int;
		case PropValueType::Float:  return typeColor_Float;
		case PropValueType::String: return typeColor_String;
		case PropValueType::Any:    return typeColor_Any;
		}
		return accentColor;
	}

	Color GetCollectionAccentColor(PropCollectionType type)
	{
		switch (type)
		{
		case PropCollectionType::Object: return typeColor_Object;
		case PropCollectionType::Array:  return typeColor_Array;
		case PropCollectionType::Map:    return typeColor_Map;
		}
		return accentColor;
	}

	Color GetPropertyAccentColor(PropertyType type, const Property& prop)
	{
		if (type == PropertyType::Regular)
		{
			return GetValueAccentColor(prop.valueType);
		}
		else if (type == PropertyType::Header)
		{
			return GetCollectionAccentColor(prop.collectionType);
		}
		else
		{
			return accentColor;
		}
	}

	void DrawPanelContents(int mousex, int mousey, bool allowHover, bool isPressed)
	{
#if _DEBUG
		// Check for unbalanced collections
		{
			int n = 0;
			for (size_t i = 0; i < numProps; ++i)
			{
				const Property& prop = props[i];
				bool isHeader = prop.name && !prop.value;
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

		int xBaseline = clientBounds.xmin + panelPaddingX;

		xMax = clientBounds.xmax - panelPaddingX;

		// Stored in first pass for reuse in second pass.
		// Collapsed properties don't add to previous line's height.
		// Size is `drawableProperties`
		int lineHeights[MAX_PROPS] = {};

		// Stored in first pass for use in second pass.
		size_t drawableProperties = numProps;

		// Pass 1: Draw property text and info
		{
			// Running y position in pixels
			int y = clientBounds.ymin + panelPaddingY - scrollY;

			// Current line's indentation in pixels.
			// Does not include panel xmin.
			int indent = 0;

			// When a collection is collapsed, hide everything deeper than this until we reach a closer with indentation.
			// Set to -1 when uncollapsed.
			int hideUntilIndent = -1;

			PropertyType typePrev = PropertyType(-1);

			for (size_t i = 0; i < numProps; ++i)
			{
				if (y > clientBounds.ymax)
				{
					drawableProperties = i;
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

					if (i != 0) [[likely]]
					{
						lineHeights[i] = lineHeights[i - 1];
					}
					else [[unlikely]] // Special case that would only even happen once in a draw anyway
					{
						lineHeights[i] = 0;
					}

					continue;
				}

				bool isHoverableProperty = type != PropertyType::Closer;

				int x = xBaseline + indent;

				Color color = GetPropertyAccentColor(type, prop);

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
				lineHeights[i] = yNext;

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
				case PropertyType::Closer:  DrawCloserProperty(indent, x, y); break;
				case PropertyType::Header:  DrawHeaderProperty(indent, x, y, prop); break;
				case PropertyType::Regular: DrawRegularProperty(x, y, prop, dividerXEnd); break;
				}

				y = yNext;
			}
		}

		// Pass 2: Collection grouping lines
		{
			struct Indent
			{
				PropCollectionType type;
				int yStart;
			};

			// We draw the indent line when popping off the stack
			Indent indentStack[32] = {};

			// Current size of the indent stack
			size_t indentDepth = 0;

			int lineHeightPrev = 0;

			for (size_t i = 0; i < drawableProperties; ++i)
			{
				const Property& prop = props[i];
				PropertyType type = (PropertyType)((((char)!!prop.name) << 1) | ((char)!!prop.value));

				int lineHeightCurr = lineHeights[i];

				int y = lineHeightPrev;
				int h = lineHeightCurr - y;

				// No change in height - indicates the property is collapsed
				bool isCollapsedValue;
				{
					isCollapsedValue = h == 0;
					lineHeightPrev = lineHeightCurr;
				}

				// A collapsed header does not contribute to indentation
				bool isCollapsedHeader = type == PropertyType::Header && prop.isCollapsed;

				bool isCollapsedProp = isCollapsedValue || isCollapsedHeader;
				
				int indent = (indentDepth - 1) * indentSize;
				int indentComplete = xBaseline + indent;

				if (!isCollapsedValue && type == PropertyType::Header)
				{
				 	Color color = GetCollectionAccentColor(prop.collectionType);
				 
				 	int xStart = indentComplete + indentSize + MeasureText(prop.name, fontSize) + propertyPaddingL;
				 	int xEnd = xMax;
				 	if (xEnd >= xStart)
				 	{
				 		int yUpper = y + halfHeight;
				 		DrawLine(xStart, yUpper, xEnd, yUpper, color);
				 		if (prop.isCollapsed)
				 		{
				 			int yLower = lineHeightCurr - (lineHeight - fontSize);
				 			DrawLine(xStart, yLower, xEnd, yLower, color);
				 		}
				 	}
				}

				if (isCollapsedProp)
				{
					continue;
				}

				// Push
				if (type == PropertyType::Header)
				{
					Indent indentInfo = 
					{
						.type = prop.collectionType,
						.yStart = lineHeightCurr,
					};

					indentStack[indentDepth++] = indentInfo;
				}

				// Pop
				else if (type == PropertyType::Closer)
				{
					if (indentDepth == 0)
					{
						console::Error("properties - Collection popped when no collections remained");
						return;
					}

					Indent indentInfo = indentStack[--indentDepth];

					Color color = GetCollectionAccentColor(indentInfo.type);

					int xStart = indentComplete;
					int xEnd = xMax;

					int yStart = indentInfo.yStart - halfFontToLine;
					int yEnd = y + lineHeight / 2;

					// Uncollapsed properties with collapsed nested properties seem to trigger this
					if (yStart > yEnd)
					{
						console::Error("properties - Ended prior to beginning");
					}

					if (xStart < yEnd)
					{
						DrawLine(xStart, yStart, xStart, yEnd, color);
					}
					if (xStart < xEnd)
					{
						DrawLine(xStart, yEnd, xEnd, yEnd, color);
					}
				}
			}
		}

		// Draw divider
		{
			int panelX = clientBounds.xmin;
			int panelY = clientBounds.ymin;
			int panelH = clientBounds.ymax - panelY;
			DrawRectangle(dividerXStart, panelY, dividerWidth, panelH, accentColor);
		}
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
