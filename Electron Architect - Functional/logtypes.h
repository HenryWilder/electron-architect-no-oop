#ifndef LOGTYPES_H
#define LOGTYPES_H

typedef enum LogType
{
	LOGTYPE_NORMAL = 0,
	LOGTYPE_WARNING,
	LOGTYPE_ERROR,
	LOGTYPE_FAILED_ASSERTION,

	// Helper for tracking the number of log types
	_LOGTYPE_NUM_TYPES
} LogType;

typedef struct Style
{
	Color color, backgroundColor;
} Style;

// Width of the "log type" segment of the log element
// Evalutes once at runtime
extern int logTypeWidth[_LOGTYPE_NUM_TYPES];

extern const Style logTypeStyles[_LOGTYPE_NUM_TYPES];

extern const char* logTypeStr[_LOGTYPE_NUM_TYPES];

#endif
