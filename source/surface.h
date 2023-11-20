#ifndef SURFACE_H
#define SURFACE_H

#include "color.h"

typedef struct surface_s
{
    uint32_t width;
    uint32_t height;

    uint32_t stride;

    uint32_t data_length;
    uint8_t *data;
} surface_s;

void surface_create(surface_s *const surface, uint32_t width, uint32_t height);
void surface_destroy(surface_s *const surface);

void surface_paint(surface_s *const, const color_u color);
void surface_blur(surface_s *const surface, const uint8_t radius, const uint8_t number_of_passes);

#endif // SURFACE_H
