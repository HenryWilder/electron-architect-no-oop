#pragma once
#include "panel.hpp"
#include <concepts>

enum class PropType
{
    Int,
    Float,
    String,
    Bool,
    Any, // Displays as a string but with a different tag
};

// Functions related to the Properties panel.
namespace properties
{
    extern panel::Panel propertiesPanel;
    void DrawPanelContents(int mousex, int mousey, bool allowHover);

    // A unique ID for tracking whether the property list needs to be cleared.
    extern size_t showingPropertiesFor;

    // Adds a property with formatting
    void Addf(const char* name, PropType type, const char* fmt...);

    // Adds a static, signed integer property
    template<std::signed_integral T> void AddInt(const char* name, T value)
    {
        properties::Addf(name, PropType::Int, "%i", value);
    }

    // Adds a static, unsigned integer property
    template<std::unsigned_integral T> void AddInt(const char* name, T value)
    {
        properties::Addf(name, PropType::Int, "%u", value);
    }

    // Adds a static float property
    template<std::floating_point T> void AddFloat(const char* name, T value)
    {
        properties::Addf(name, PropType::Float, "%f", value);
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
    void AddHeader(const char* name);

    // Unindents following properties
    void AddCloser();

    // Adds a property whose value takes up multiple lines
    // Multiline properties still work with Add(), but this gives the result better separation.
    void AddMultiline(const char* name, const char* valueStr);

    // Clears the properties panel info
    void Clear();

    // Performs any cleanup needed to unload resources associated with this namespace when the program closes
    void Unload();
}
