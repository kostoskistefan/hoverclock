#include "color.h"

color_u color_multiply(const color_u color, const float multiplier)
{
    color_u multiplied_color = {
        .a = color.a * multiplier,
        .r = color.r * multiplier,
        .g = color.g * multiplier,
        .b = color.b * multiplier,
    };

    return multiplied_color;
}
