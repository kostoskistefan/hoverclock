#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <stdint.h>

typedef struct rectangle_s
{
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
} rectangle_s;

#endif // RECTANGLE_H
