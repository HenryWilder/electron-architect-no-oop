#pragma once
#include "panel.hpp"

// Functions related to the Properties panel.
namespace properties
{
    extern panel::Panel propertiesPanel;
    void DrawPanelContents();

    // A unique ID for tracking whether the property list needs to be cleared.
    size_t showingPropertiesFor = NULL;

    constexpr size_t npos = (size_t)(-1ull);

    // Inserts an integer property at the specified index
    void AddProperty(const char* name, long long value, size_t index);
    // Inserts an integer property at the specified index
    inline void AddProperty(const char* name, int value, size_t index) { AddProperty(name, (long long)value, index); }
    // Inserts a float property at the specified index
    void AddProperty(const char* name, double value, size_t index);
    // Inserts a string property at the specified index
    void AddProperty(const char* name, const char* value, size_t index);
    // Inserts a named property collection header at the specified index
    void BeginPropertyCollection(const char* name, const char* collectionName, size_t index);
    // Inserts an anonymous property collection header at the specified index
    void BeginPropertyCollection(const char* collectionName, size_t index);
    // Applies ownership to properties between the last property collection header and the specified index
    void EndPropertyCollection(size_t index);

    // Appends an integer property
    void AddProperty(const char* name, long long value);
    // Appends a float property
    void AddProperty(const char* name, double value);
    // Appends a string property
    void AddProperty(const char* name, const char* value);
    // Appends a named property collection header
    void BeginPropertyCollection(const char* name, const char* collectionName);
    // Appends an anonymous property collection header
    void BeginPropertyCollection(const char* collectionName);
    // Applies ownership to properties since the last property collection header
    void EndPropertyCollection();

    void ClearProperties();
}
