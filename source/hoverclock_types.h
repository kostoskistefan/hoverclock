#ifndef HOVERCLOCK_TYPES_H
#define HOVERCLOCK_TYPES_H

#include <stdint.h>

typedef enum HoverclockPosition
{
    HOVERCLOCK_POSITION_TOP_LEFT,
    HOVERCLOCK_POSITION_TOP_RIGHT,
    HOVERCLOCK_POSITION_BOTTOM_LEFT,
    HOVERCLOCK_POSITION_BOTTOM_RIGHT,
} HoverclockPosition;

typedef union HoverclockColor
{
    struct
    {
        uint8_t a;
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };

    uint32_t rgba;
} HoverclockColor;

typedef union HoverclockPair
{
    struct
    {
        unsigned int x;
        unsigned int y;
    };

    struct
    {
        unsigned int width;
        unsigned int height;
    };
} HoverclockPair;

typedef struct HoverclockTextStyle
{
    char *font;
    char *format;
} HoverclockTextStyle;

#endif // HOVERCLOCK_TYPES_H
