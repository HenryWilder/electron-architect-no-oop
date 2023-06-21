#pragma once
#include "panel.hpp"

// Functions related to the Properties panel.
namespace properties
{
    extern panel::Panel propertiesPanel;
    void DrawPanelContents(int mousex, int mousey, bool allowHover);

    // A unique ID for tracking whether the property list needs to be cleared.
    extern size_t showingPropertiesFor;

    // Adds a property
    void Add(const char* name, const char* valueStr);

    // Adds a property with formatting
    void Addf(const char* name, const char* fmt...);

    // Adds a linked property with formatting
    // (Prefer Add() or Addf() if the value will not change until the properties panel changes)
    void AddLinkedInt(const char* name, const char* fmt, const int* valueSrcPtr);

    // Adds a linked property with formatting
    // (Prefer Add() or Addf() if the value will not change until the properties panel changes)
    void AddLinkedFloat(const char* name, const char* fmt, const float* valueSrcPtr);

    // Adds a linked property with formatting
    // (Prefer Add() or Addf() if the value will not change until the properties panel changes)
    void AddLinkedString(const char* name, const char* fmt, const char* const* valueSrcPtr);

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
