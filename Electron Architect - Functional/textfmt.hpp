#pragma once

// Creates a string using memory from the heap - Remember to free when done using
char* Formatted(const char* _Format, va_list _ArgList);

// Creates a string using memory from the heap - Remember to free when done using
// Saves time AND space when the string size is roughly known
// Examples:
// - We are printing a number with fixed digits (("0x%06i", n): hintSizeMax = 9)
//     Note: Remember that the terminator is a character
// - We are printing a number with limited size (("%i", ClampInt(n, 0, 99)): hintSizeMax = 3)
// - We are printing a string from enumeration  (("%s", x ? "Exists" : "Does not exist"): hintSizeMax = 15)
//     Note: Remember to hint the MAXIMUM size
char* Formatted(size_t hintSizeMax, const char* _Format, va_list _ArgList);
