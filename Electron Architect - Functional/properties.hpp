#pragma once
#include <concepts>
#include "panel.hpp"

// Functions related to the Properties panel.
namespace properties
{
    extern panel::Panel propertiesPanel;

    enum class PropValueType
    {
        Bool,
        Byte,
        Int,
        Float,
        String,
        Any, // Displays as a string but with a different tag
    };

    enum class PropCollectionType
    {
        Object, // Default
        Array,
        Map,
    };

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

        // The last time this was hovered - used in animations
        double lastHovered = 0.0;
    };

    constexpr size_t MAX_PROPS = 1024;
    extern Property props[MAX_PROPS];
    extern int numProps;

    constexpr int lineHeight = 16;

    extern int scrollY;
    extern int dividerX;

    // @param mousex: Mouse x
    // @param mousey: Mouse y
    // @param mouseyPrev: Mouse y last frame - Used for making effects appear continuous in spite of discrete mouse position
    // @param allowHover: Whether mouse hover is already consumed for something else
    // @param isPressed: Whether the mouse was clicked
    void DrawPanelContents(int mousex, int mousey, int mouseyPrev, bool allowHover);

    // A unique ID for tracking whether the property list needs to be cleared.
    extern size_t showingPropertiesFor;

    // Adds a property with formatting
    void Addf_hint(const char* name, size_t hintSizeMax, PropValueType type, const char* fmt...);

    // Adds a property with formatting
    void Addf(const char* name, PropValueType type, const char* fmt...);

    // Adds a static, signed integer property
    template<std::signed_integral T> void AddInt(const char* name, T value)
    {
        properties::Addf(name, PropValueType::Int, "%i", value);
    }
    // Adds a static, signed integer property
    template<std::signed_integral T> void AddInt(const char* name, size_t hintSizeMax, T value)
    {
        properties::Addf_hint(name, hintSizeMax, PropValueType::Int, "%i", value);
    }

    // Adds a static, unsigned integer property
    template<std::unsigned_integral T> void AddInt(const char* name, T value)
    {
        properties::Addf(name, PropValueType::Int, "%u", value);
    }
    // Adds a static, unsigned integer property
    template<std::unsigned_integral T> void AddInt(const char* name, size_t hintSizeMax, T value)
    {
        properties::Addf_hint(name, hintSizeMax, PropValueType::Int, "%u", value);
    }

    // Adds a static float property
    template<std::floating_point T> void AddFloat(const char* name, T value)
    {
        properties::Addf(name, PropValueType::Float, "%f", value);
    }
    // Adds a static float property
    template<std::floating_point T> void AddFloat(const char* name, size_t hintSizeMax, T value)
    {
        properties::Addf_hint(name, hintSizeMax, PropValueType::Float, "%f", value);
    }

    // Adds a static boolean property
    void AddBool(const char* name, bool value);

    // Adds a static string property
    void AddString(const char* name, const char* value);

    // Adds a linked integer property with formatting
    // (Prefer AddInt() or Addf() if the value will not change until the properties panel changes)
    void AddLinkedInt(const char* name, const char* fmt, const int* valueSrcPtr);

    // Adds a linked float property with formatting
    // (Prefer AddFloat() or Addf() if the value will not change until the properties panel changes)
    void AddLinkedFloat(const char* name, const char* fmt, const float* valueSrcPtr);

    // Adds a linked string property with formatting
    // (Prefer AddString() or Addf() if the value will not change until the properties panel changes)
    void AddLinkedString(const char* name, const char* fmt, const char* const* valueSrcPtr);

    // Adds a linked boolean property
    void AddLinkedBool(const char* name, const bool* valueSrcPtr);

    // Indents following properties
    void AddHeader(const char* name, PropCollectionType type);

    inline void AddObjectHeader(const char* name) { AddHeader(name, PropCollectionType::Object); }
    inline void AddArrayHeader (const char* name) { AddHeader(name, PropCollectionType::Array); }
    inline void AddMapHeader   (const char* name) { AddHeader(name, PropCollectionType::Map); }

    // Unindents following properties
    void AddCloser();

    // Clears the properties panel info
    void Clear();

    // Performs any cleanup needed to unload resources associated with this namespace when the program closes
    void Unload();
}
