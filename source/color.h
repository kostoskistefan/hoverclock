#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef union color_u
{
    uint32_t argb;

    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
} color_u;

color_u color_multiply(const color_u color, const float multiplier);

#endif // COLOR_H
