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
    void AddProperty(const char* name, const char* valueStr);
    // Indents following properties
    void AddPropertyHeader(const char* name);
    // Unindents following properties
    void AddPropertyCloser();

    // Clears the properties panel info
    void ClearProperties();
}
