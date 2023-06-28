#include <cstdarg>
#include "utils.hpp"
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

	Property props[MAX_PROPS] = {};
	int numProps = 0;

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
