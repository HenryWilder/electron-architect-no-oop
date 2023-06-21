#include <cstdarg>
#include <stdio.h>
#include "console.hpp"

// Creates a string using memory from the heap - Remember to free when done using
// Saves space when the string is expected to be short
char* _Formatted_MinHeapUsage(const char* _Format, va_list _ArgList)
{
	constexpr size_t BUFFER_MAX_SIZE = 1024;
	char staticBuff[BUFFER_MAX_SIZE] = {};
	for (size_t i = 0; i < BUFFER_MAX_SIZE; ++i)
	{
		staticBuff[i] = '\0';
	}
	vsnprintf(staticBuff, BUFFER_MAX_SIZE, _Format, _ArgList);
	size_t buffer_used_size = 0;
	for (char ch : staticBuff)
	{
		++buffer_used_size; // Before `if` because the null terminator also needs to be included in the overhead
		if (ch == '\0')
		{
			break;
		}
	}
	char* result = new char[buffer_used_size];
	for (size_t i = 0; i < buffer_used_size; ++i)
	{
		result[i] = staticBuff[i];
	}
	return result;
}

// Creates a string using memory from the heap - Remember to free when done using
// Saves time when the string is expected to be long (closer to 1024)
char* _Formatted_MinComplexity(const char* _Format, va_list _ArgList)
{
	constexpr size_t BUFFER_MAX_SIZE = 1024;
	char* result = new char[BUFFER_MAX_SIZE];
	for (size_t i = 0; i < BUFFER_MAX_SIZE; ++i)
	{
		result[i] = '\0';
	}
	vsnprintf(result, BUFFER_MAX_SIZE, _Format, _ArgList);
	return result;
}

char* _Formatted_MinComplexity(size_t hintSizeMax, const char* _Format, va_list _ArgList)
{
	char* result = new char[hintSizeMax];
	for (size_t i = 0; i < hintSizeMax; ++i)
	{
		result[i] = '\0';
	}
	vsnprintf(result, hintSizeMax, _Format, _ArgList);
	return result;
}

// Todo: Make this decide when to use less heap vs less complexity.
char* Formatted(const char* _Format, va_list _ArgList)
{
	char* result = _Formatted_MinHeapUsage(_Format, _ArgList);
	return result;
}

char* Formatted(size_t hintSizeMax, const char* _Format, va_list _ArgList)
{
	char* result = _Formatted_MinComplexity(hintSizeMax, _Format, _ArgList);
	return result;
}
