#pragma once
#include "panel.hpp"

// Functions related to the Properties panel.
namespace properties
{
    extern panel::Panel propertiesPanel;
    void DrawPanelContents();

    // A unique ID for tracking whether the property list needs to be cleared.
    extern size_t showingPropertiesFor;

    // Todo
#if false
    void InsertIntProperty(const char* name, long long value, size_t index);
    inline void InsertIntProperty(const char* name, int value, size_t index) { InsertIntProperty(name, (long long)value, index); }
    void InsertFloatProperty(const char* name, double value, size_t index);
    inline void InsertFloatProperty(const char* name, float value, size_t index) { InsertFloatProperty(name, (double)value, index); }
    void InsertStringProperty(const char* name, const char* value, size_t index);
    void InsertPropertyCollectionHeader(const char* name, const char* collectionName, size_t index);
    void InsertTopLevelPropertyCollectionHeader(const char* collectionName, size_t index);
    void InsertPropertyCollectionCloser(size_t index);
#endif

    void AddIntProperty(const char* name, long long value);
    inline void AddIntProperty(const char* name, int value) { AddIntProperty(name, (long long)value); }
    void AddFloatProperty(const char* name, double value);
    inline void AddFloatProperty(const char* name, float value) { AddFloatProperty(name, (double)value); }
    void AddStringProperty(const char* name, const char* value);
    void AddPropertyCollectionHeader(const char* name, const char* collectionName);
    void AddTopLevelPropertyCollectionHeader(const char* collectionName);
    void AddPropertyCollectionCloser();

    void ClearProperties();
}
