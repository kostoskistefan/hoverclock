#ifndef WINDOW_H
#define WINDOW_H

#include "color.h"
#include "widget.h"
#include "rectangle.h"
#include "display_server.h"

#define WINDOW_VERTICAL_PADDING           5
#define WINDOW_HORIZONTAL_PADDING         5
#define WINDOW_MAXIUMUM_NUMBER_OF_WIDGETS 2

typedef struct window_s
{
    uint8_t stop_requested;

    rectangle_s rectangle;

    xcb_window_t id;
    xcb_gcontext_t graphical_context;

    color_u background_color;

    xcb_expose_event_t *expose_event;
    xcb_client_message_event_t *client_message_event;

    const display_server_s *display_server;

    uint8_t number_of_widgets;
    widget_s *widgets[WINDOW_MAXIUMUM_NUMBER_OF_WIDGETS];
} window_s;

void window_create(window_s *const window, const display_server_s *const display_server);
void window_run(window_s *const window);
void window_destroy(window_s *const window);

void window_move(window_s *const window, const uint32_t x, const uint32_t y);

void window_add_widget(window_s *const window, widget_s *const widget);
void window_set_background_color(window_s *const window, const color_u color);

void window_calculate_location(
    window_s *const window,
    const char *location_name,
    const uint32_t horizontal_margin,
    const uint32_t vertical_margin
);

void window_refresh(window_s *const window);

void window_on_quit_request(window_s *const window);

#endif // WINDOW_H
