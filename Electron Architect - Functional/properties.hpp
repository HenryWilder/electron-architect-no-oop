#pragma once
#include "panel.hpp"

// Functions related to the Properties panel.
namespace properties
{
    extern panel::Panel propertiesPanel;
    void DrawPanelContents();

    // A unique ID for tracking whether the property list needs to be cleared.
    extern size_t showingPropertiesFor;

    // Adds a property
    void Add(const char* name, const char* valueStr);
    // Adds a property with formatting
    // (Warning: uses heap memory)
    void Addf(const char* name, const char* fmt...);
    // Indents following properties
    void AddHeader(const char* name);
    // Unindents following properties
    void AddCloser();
    // Adds a property whose value takes up multiple lines
    // TODO
    void AddMultiline(const char* name, const char* valueStr);

    // Clears the properties panel info
    void Clear();

    // Performs any cleanup needed to unload resources associated with this namespace when the program closes
    void Unload();
}
