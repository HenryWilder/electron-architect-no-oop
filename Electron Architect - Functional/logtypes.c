#include <raylib.h>
#include "logtypes.h"

int logTypeWidth[_LOGTYPE_NUM_TYPES];

#define BLUE_STYLE   { .color = { 255,255,255, 255 }, .backgroundColor = {   0, 64,127, 255 } }
#define YELLOW_STYLE { .color = { 255,255,127, 255 }, .backgroundColor = { 127,127,  0, 255 } }
#define RED_STYLE    { .color = { 255, 64, 64, 255 }, .backgroundColor = { 127,  0,  0, 255 } }

const Style logTypeStyles[_LOGTYPE_NUM_TYPES] =
{
	[LOGTYPE_NORMAL]           = BLUE_STYLE,
	[LOGTYPE_WARNING]          = YELLOW_STYLE,
	[LOGTYPE_ERROR]            = RED_STYLE,
	[LOGTYPE_FAILED_ASSERTION] = RED_STYLE,
};

const char* logTypeStr[_LOGTYPE_NUM_TYPES] =
{
	"Info",
	"Error",
	"Warning",
	"Failed Assertion"
};

#undef BLUE_STYLE
#undef YELLOW_STYLE
#undef RED_STYLE
