#pragma once
#include "panel.hpp"
#include <concepts>

// Functions related to the Properties panel.
namespace properties
{
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

    extern panel::Panel propertiesPanel;

    constexpr int lineHeight = 16;

    extern int scrollY;
    extern int dividerX;

    void DrawPanelContents(int mousex, int mousey, bool allowHover, bool isPressed);

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

    // Adds an expression property
    // Note: Prefer using anything else if possible. This evaluates the expression lambda every time the property is drawn.
    // ! Do not use the expression evaluation as actual execution code. Expressions are not evaluated when their containing collections are collapsed.
    template<typename Func> void AddLinkedExpression(const char* name, Func expression)
    {
        static_assert(false, "Linked Expressions are not yet supported.");
    }

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
