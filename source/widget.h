#ifndef WIDGET_H
#define WIDGET_H

#include "rectangle.h"
#include "surface.h"
#include <stdint.h>

typedef struct window_s window_s;

typedef struct widget_s
{
    uint8_t render_requested;

    surface_s surface;

    rectangle_s rectangle;

    const window_s *window;

    void (*render)(const struct widget_s *const widget);
} widget_s;

#endif // WIDGET_H
