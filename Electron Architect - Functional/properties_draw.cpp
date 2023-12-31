#include "utils.hpp"
#include "console.hpp"
#include "properties.hpp"

namespace properties
{
	int scrollY = 0;

	// Position of the divider (offset from left side)
	// Todo: Make this draggable
	int dividerX = 150;
	// Width of the divider visual
	constexpr int dividerWidth = 3;

	int xMax;

	constexpr int indentSize = 8;

	constexpr int panelPaddingX  = 5;
	constexpr int panelPaddingY  = 5;
	constexpr int fontSize       = 8;
	constexpr int halfHeight     = fontSize / 2;
	constexpr int halfFontToLine = (lineHeight - fontSize) / 2 + 1; // Half of the difference between the font height and the line height
	constexpr int closerLineHeight = indentSize;

	// Pixels of transition completed per second
	// Note that this will be bracketed
	// i.e., this will be dependent on panel width at fixed increments
	constexpr double hoverTransitionSpeed = 400;
	// Size of brackets at which hover transition speed is multiplied linearly in steps
	constexpr int hoverTransitionBracketWidth = 600;

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


	struct Indent
	{
		PropCollectionType type;
		int yStart;
	};

	// We draw the indent line when popping off the stack
	Indent indentStack[32] = {};
	// Current size of the indent stack
	size_t indentDepth = 0;

	void PushIndentStack(PropCollectionType type, int y)
	{
		Indent indentInfo = 
		{
			.type = type,
			.yStart = y,
		};

		indentStack[indentDepth++] = indentInfo;
	}

	// Also handles drawing the item
	void PopIndentStack(int indentComplete, int y)
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
		int yEnd = y;

		// Uncollapsed properties with collapsed nested properties seem to trigger this
		if (yStart > yEnd)
		{
			console::Error("properties - Ended prior to beginning");
		}

		// Vertical line
		if (yStart < yEnd)
		{
			DrawLine(xStart, yStart, xStart, yEnd, color);
		}

		// Horizontal line
		if (xStart < xEnd)
		{
			DrawLine(xStart, yEnd, xEnd, yEnd, color);
		}
	};

	void DrawPanelContents(int mousex, int mousey, int mouseyPrev, bool allowHover)
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

		const double currentTime = GetTime();
		const double previousFrameTime = currentTime - (double)GetFrameTime();

		int mouseyMin, mouseyMax;
		{
			if (mousey < mouseyPrev)
			{
				mouseyMin = mousey;
				mouseyMax = mouseyPrev;
			}
			else
			{
				mouseyMin = mouseyPrev;
				mouseyMax = mousey;
			}
		}

		const panel::Bounds clientBounds = panel::PanelClientBounds(propertiesPanel);

		const int panelWidth = clientBounds.xmax - clientBounds.xmin;
		const int panelWidthBracket = 1 + (panelWidth / hoverTransitionBracketWidth);
		const double hoverTransitionSpeed_Contextual = (double)hoverTransitionSpeed * panelWidthBracket;

		// End of anything left of the divider 
		const int dividerXStart = clientBounds.xmin + dividerX - panelPaddingX;

		// Start of anything right of the divider
		const int dividerXEnd = dividerXStart + dividerWidth + panelPaddingX;

		const int xBaseline = clientBounds.xmin + panelPaddingX;

		xMax = clientBounds.xmax - panelPaddingX;

		// Stored in first pass for reuse in second pass.
		// Collapsed properties don't add to previous line's height.
		// Size is `drawableProperties`
		int lineHeights[MAX_PROPS] = {};

		// Stored in first pass for use in second pass.
		size_t drawableProperties = numProps;

		const int startY = clientBounds.ymin + panelPaddingY - scrollY;

		// Pass 1: Draw property text and info
		{
			// Running y position in pixels
			int y = startY;

			// Current line's indentation in pixels.
			// Does not include panel xmin.
			int indent = 0;

			// When a collection is collapsed, hide everything deeper than this until we reach a closer with indentation.
			// Set to -1 when uncollapsed.
			int hideUntilIndent = -1;

			PropertyType typePrev = PropertyType(-1);

			for (size_t i = 0; i < numProps; ++i)
			{
				bool canIndexPrevious = i != 0;

				// Early exit when client bounds are exceeded
				// Only happens once per frame, and not even necessarily every frame.
				if (y > clientBounds.ymax) [[unlikely]]
				{
					drawableProperties = i;
				break;
				}

					// All things past this point are implied to be above the client ymax and potentially visible.

	#pragma region // Frequently used status checks

					// Shows we just want to read the property
				const Property& prop = props[i];
				// Shows we need to modify the property
				Property& propEditable = props[i];

				// Property has a name
				bool isPropNamed = !!prop.name;

				// Property has a value
				bool isPropValued = !!prop.value;

				// Property is linked
				// Implies
				// - Property has fmt
				bool isPropLinked = !!prop.fmt;

				const PropertyType type = (PropertyType)((((char)isPropNamed) << 1) | ((char)isPropValued));

				// Implies
				// - Property is named
				// - Property is NOT valued
				bool isPropHeader  = type == PropertyType::Header;

				// Implies
				// - Property is NOT named
				// - Property is NOT valued
				bool isPropCloser  = type == PropertyType::Closer;

				// Implies
				// - Property is named
				// - Property is valued
				bool isPropRegular = type == PropertyType::Regular;

	#pragma endregion

				// Implies
				// - A collapsed header has been encountered at some point in a past iteration
				// - The indentation has not yet returned to that of the afformentioned header
				bool isInCollapsedSection = hideUntilIndent != -1;

				if (isInCollapsedSection)
				{
					if (isPropHeader)
					{
						indent += indentSize;
					}

					else if (isPropCloser)
					{
						indent -= indentSize;

						// Gets set AFTER unindent.
						// Implies
						// - Property is closer
						// - Property is inside a collapsed collection
						bool isReadyForUnhide = indent == hideUntilIndent;

						// It's hard to determine whether this is likely or unlikely.
						// Exactly one closer per collection will meet this condition.
						// 
						// On one hand, all closers within flat collections (not containing sub-collections)
						// are 100% guaranteed to meet this condition.
						// 
						// On the other hand, the more sub collections inside a collapsed collection,
						// the more closers there are that do not meet this condition.
						if (isReadyForUnhide)
						{
							hideUntilIndent = -1;
						}
					}

					// All but the first index meet this condition
					if (canIndexPrevious) [[likely]]
					{
						lineHeights[i] = lineHeights[i - 1];
					}
						// Unlikely edge case - would require the first property to be a header and also collapsed
						// Even in that case, would only even happen once per draw
					else [[unlikely]]
					{
						lineHeights[i] = 0;
					}

					continue;
				}

				// All things after this point are implied to NOT be within a collapsed section and therefore definitely visible.

	#pragma region // Additional booleans that aren't necessary for skippable properties

				// Implies
				// - Property is NOT a closer
				bool isPropHoverable = !isPropCloser;

				// Implies
				// - Property has a value
				bool isPropUnlinked = isPropValued && !isPropLinked;

				// Implies
				// - Property has a value
				// - Property is linked
				bool isPropLinkedString = isPropValued && isPropLinked && prop.valueType == PropValueType::String;

				// Implies
				// - Property is named
				bool isPropWidthDirty = isPropNamed && prop.nameWidth == -1;

				// Implies
				// - Property is a header
				bool isPropCollapsable = isPropHeader;

	#pragma endregion

				int x = xBaseline + indent;

				Color color = GetPropertyAccentColor(type, prop);

				// The height of the current property
				int propHeight;
				{
					if (isPropCloser)
					{
						propHeight = closerLineHeight;
					}
					else if (isPropValued)
					{
						int numLines = 1;
						{
							if (isPropUnlinked || isPropLinkedString)
							{
								const char* textToMeasure = isPropUnlinked
									?  (const char*       )prop.value
									: *(const char* const*)prop.value;

								numLines = CountNewlines(textToMeasure);
							}
						}

						propHeight = lineHeight * numLines;
					}
					else
					{
						propHeight = lineHeight;
					}
				}

				int yNext = y + propHeight;

				lineHeights[i] = yNext;

				// Draw background for hovered/named properties
				if (isPropNamed || isPropHoverable)
				{
					bool isInHorizontalBounds = x <= mousex && mousex <= xMax;
					bool isInVerticalBounds = y <= mousey && mousey < yNext;

					bool isHovering = isInHorizontalBounds && isInVerticalBounds;

					// Mouse would have hovered this property within the last frame if it was tracked continously
					bool isVirtuallyInVerticalBounds = mouseyMin <= y && (yNext - 1) <= mouseyMax;
					bool isVirtuallyHovered = isInHorizontalBounds && isVirtuallyInVerticalBounds;

					bool isHoverVis = allowHover && isHovering;
					bool isVirtualHoverVis = allowHover && isVirtuallyHovered;

					if (isHoverVis)
					{
						propEditable.lastHovered = currentTime;
					}
					else if (isVirtualHoverVis)
					{
						propEditable.lastHovered = previousFrameTime;
					}

					bool isUserCollapsingProp = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isHoverVis && isPropCollapsable;

					// This only happens when a name changes, which should be far less frequent than every frame.
					if (isPropWidthDirty) [[unlikely]]
					{
						propEditable.nameWidth = MeasureText(prop.name, fontSize);
					}

					int paddedXMax_Normal = x + prop.nameWidth + propertyPaddingR;
					int paddedXMax_Hover = xMax + propertyPaddingR;

					double timeSinceHover = currentTime - prop.lastHovered;
					double fadeDuration = GetAnimatedFadeDuration(hoverTransitionSpeed_Contextual, paddedXMax_Hover, paddedXMax_Normal);
					bool isInAnimationTransition = !isHoverVis && timeSinceHover < fadeDuration;

					int paddedXMin = x - propertyPaddingL;
					int paddedYMin = y - propertyPaddingT;
					int paddedXMax = x + propertyPaddingR;

					// Nameless, unhovered property backgrounds will appear the same as those whose name is an empty string.

					// Whether this is likely or unlikely is unpredictable.
					if (isInAnimationTransition)
					{
						int paddedXMax_Animated = (int)AnimatedFade(timeSinceHover, fadeDuration, paddedXMax_Hover, paddedXMax_Normal);
						paddedXMax = paddedXMax_Animated;
					}
					// Even if hover is allowed and true, only one property will ever be hovered at a time.
					// Implies the transition has finished.
					else if (isHoverVis) [[unlikely]]
					{
						paddedXMax = paddedXMax_Hover;
					}
						// Only closers won't have a name.
					else if (isPropNamed) [[likely]]
					{
						paddedXMax = paddedXMax_Normal;
					}

					int paddedYMax = yNext + propertyPaddingB;
					int paddedW = paddedXMax - paddedXMin;
					int paddedH = paddedYMax - paddedYMin;

					DrawRectangle(paddedXMin, paddedYMin, paddedW, paddedH, color);

					// Already only one property will ever be hovered at a time.
					// And on top of that, the user will be pressing m1 a lot less frequently than the visuals are drawn.
					if (isUserCollapsingProp) [[unlikely]]
					{
						console::Log("Header collapse toggled");
					propEditable.isCollapsed = !prop.isCollapsed;
					}
				}

				// Gets set AFTER user has potentially toggled the collapsed state of the property.
				// Implies
				// - Property is a header
				// - This property in particular is collapsed, not just within a collapsed collection
				bool isPropCollapsedHeader = isPropHeader && prop.isCollapsed;

				if (isPropCollapsedHeader)
				{
					hideUntilIndent = indent;
				}

				switch (type)
				{
				case PropertyType::Closer:  DrawCloserProperty (indent, x, y                   ); break;
				case PropertyType::Header:  DrawHeaderProperty (indent, x, y, prop             ); break;
				case PropertyType::Regular: DrawRegularProperty(        x, y, prop, dividerXEnd); break;
				}

				y = yNext;
			}
		}

		// Pass 2: Collection grouping lines
		{
			int lineHeightPrev = startY;

			for (size_t i = 0; i < drawableProperties; ++i)
			{
				const Property& prop = props[i];
				PropertyType type = (PropertyType)((((char)!!prop.name) << 1) | ((char)!!prop.value));

				bool isPropHeader = type == PropertyType::Header;
				bool isPropCloser = type == PropertyType::Closer;

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
				bool isCollapsedHeader = isPropHeader && prop.isCollapsed;

				bool isCollapsedProp = isCollapsedValue || isCollapsedHeader;

				int indent = (indentDepth - 1) * indentSize;
				int indentComplete = xBaseline + indent;

				if (!isCollapsedValue && isPropHeader)
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
				if (isPropHeader)
				{
					PushIndentStack(prop.collectionType, lineHeightCurr);
				}

				// Pop
				else if (isPropCloser)
				{
					PopIndentStack(indentComplete, y);
				}
			}

			// Complete remaining indent lines on the stack
			while (indentDepth > 0)
			{
				int indent = (indentDepth - 1) * indentSize;
				int indentComplete = xBaseline + indent;
				int y = propertiesPanel.bounds.ymax;
				PopIndentStack(indentComplete, y);
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
}
