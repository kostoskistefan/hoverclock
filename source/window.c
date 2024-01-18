#include "window.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <insight.h>
#include <xcb/shape.h>

xcb_visualid_t window_get_truecolor_visual(xcb_screen_t *const screen);
void window_render_widgets(const window_s *const window, uint8_t force_render);
void window_make_clickthrough(const window_s *const window);
void window_set_wm_hints(const window_s *const window);
void window_set_wm_hint_wm_name(const window_s *const window);
void window_set_wm_hint_wm_class(const window_s *const window);
void window_set_wm_hint_net_wm_name(const window_s *const window);
void window_set_wm_hint_net_wm_state(const window_s *const window);
void window_set_wm_hint_net_wm_desktop(const window_s *const window);
void window_set_wm_hint_net_wm_window_type(const window_s *const window);

void window_create(window_s *const window, const display_server_s *const display_server)
{
    window->display_server = display_server;

    const rectangle_s window_rectangle = {
        .x = 0,
        .y = 0,
        .width = (WINDOW_HORIZONTAL_PADDING << 1),
        .height = (WINDOW_VERTICAL_PADDING << 1)
    };

    window->stop_requested = 0;
    window->number_of_widgets = 0;
    window->rectangle = window_rectangle;
    window->background_color.argb = 0xffffffff;
    window->id = xcb_generate_id(window->display_server->connection);

    const xcb_visualid_t visual_id = window_get_truecolor_visual(window->display_server->screen);
    const xcb_colormap_t colormap = xcb_generate_id(window->display_server->connection);

    xcb_create_colormap(
        window->display_server->connection,
        XCB_COLORMAP_ALLOC_NONE,
        colormap,
        window->display_server->screen->root,
        visual_id
    );

    xcb_create_window(
        window->display_server->connection,
        32,
        window->id,
        window->display_server->screen->root,
        window_rectangle.x,
        window_rectangle.y,
        window_rectangle.width,
        window_rectangle.height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        visual_id,
        XCB_CW_BACK_PIXEL |
            XCB_CW_BORDER_PIXEL |
            XCB_CW_OVERRIDE_REDIRECT |
            XCB_CW_EVENT_MASK |
            XCB_CW_COLORMAP,
        (uint32_t[]) {
            window->background_color.argb,
            0,
            1,
            XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
            colormap }
    );

    xcb_free_colormap(window->display_server->connection, colormap);

    window->graphical_context = xcb_generate_id(window->display_server->connection);

    xcb_create_gc(
        window->display_server->connection,
        window->graphical_context,
        window->id,
        0,
        NULL
    );

    window->client_message_event = calloc(1, sizeof(*window->client_message_event));
    window->client_message_event->format = 32;
    window->client_message_event->window = window->id;
    window->client_message_event->response_type = XCB_CLIENT_MESSAGE;

    window_set_wm_hints(window);
    window_make_clickthrough(window);
}

void window_run(window_s *const window)
{
    xcb_generic_event_t *event;

    xcb_map_window(window->display_server->connection, window->id);
    xcb_flush(window->display_server->connection);

    while (window->stop_requested == 0)
    {
        event = xcb_wait_for_event(window->display_server->connection);

        switch (event->response_type & 0x7f)
        {
            // Expose event is needed on the initial render and for rendering the widgets forcefully
            // after computer wake up since we have the XCB_CW_OVERRIDE_REDIRECT set to 1.
            case XCB_EXPOSE:
                window_render_widgets(window, 1);
                break;

            case XCB_CLIENT_MESSAGE:
                window_render_widgets(window, 0);
                break;

            default:
                break;
        }

        free(event);
    }
}

void window_destroy(window_s *const window)
{
    xcb_flush(window->display_server->connection);

    free(window->client_message_event);

    xcb_free_gc(window->display_server->connection, window->graphical_context);
    xcb_destroy_window(window->display_server->connection, window->id);
}

void window_add_widget(window_s *const window, widget_s *const widget)
{
    if (window->number_of_widgets >= WINDOW_MAXIUMUM_NUMBER_OF_WIDGETS)
    {
        return;
    }

    window->widgets[window->number_of_widgets] = widget;

    ++window->number_of_widgets;

    if (widget->rectangle.width > window->rectangle.width)
    {
        window->rectangle.width = widget->rectangle.width + (WINDOW_HORIZONTAL_PADDING << 1);
    }

    const uint32_t new_height = widget->rectangle.y + widget->rectangle.height + WINDOW_VERTICAL_PADDING;

    if (window->rectangle.height < new_height)
    {
        window->rectangle.height = new_height;
    }

    xcb_configure_window(
        window->display_server->connection,
        window->id,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        (uint32_t[]) { window->rectangle.width, window->rectangle.height }
    );

    xcb_flush(window->display_server->connection);
}

void window_set_background_color(window_s *const window, const color_u color)
{
    window->background_color = color;

    xcb_change_window_attributes(
        window->display_server->connection,
        window->id,
        XCB_CW_BACK_PIXEL,
        (uint32_t[]) { window->background_color.argb }
    );

    xcb_flush(window->display_server->connection);
}

void window_move(window_s *const window, const uint32_t x, const uint32_t y)
{
    window->rectangle.x = x;
    window->rectangle.y = y;

    xcb_configure_window(
        window->display_server->connection,
        window->id,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
        (uint32_t[]) { window->rectangle.x, window->rectangle.y }
    );

    xcb_flush(window->display_server->connection);
}

void window_calculate_location(
    window_s *const window,
    const char *location_name,
    const uint32_t horizontal_margin,
    const uint32_t vertical_margin
)
{
    char *lowercase_location_name = strdup(location_name);

    for (uint8_t i = 0; lowercase_location_name[i]; i++)
    {
        lowercase_location_name[i] = tolower(lowercase_location_name[i]);
    }

    const char *delimiter = " ";
    const char *vertical_location_name = strtok(lowercase_location_name, delimiter);
    const char *horizontal_location_name = strtok(NULL, delimiter);

    const uint8_t window_is_left = (strcmp(horizontal_location_name, "left") == 0);
    const uint8_t window_is_top = (strcmp(vertical_location_name, "top") == 0);

    free(lowercase_location_name);

    uint32_t x_location = 0;
    uint32_t y_location = 0;

    if (window_is_left)
    {
        x_location = horizontal_margin;
    }

    else
    {
        x_location =
            window->display_server->screen->width_in_pixels -
            window->rectangle.width -
            horizontal_margin;
    }

    if (window_is_top)
    {
        y_location = vertical_margin;
    }

    else
    {
        y_location =
            window->display_server->screen->height_in_pixels -
            window->rectangle.height -
            vertical_margin;
    }

    window_move(window, x_location, y_location);
}

void window_refresh(window_s *const window)
{
    xcb_send_event(
        window->display_server->connection,
        0,
        window->id,
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
        (const char *) window->client_message_event
    );

    xcb_flush(window->display_server->connection);
}

void window_on_quit_request(window_s *const window)
{
    window->stop_requested = 1;
    window_refresh(window);
}

xcb_visualid_t window_get_truecolor_visual(xcb_screen_t *const screen)
{
    xcb_depth_iterator_t depth_iterator = xcb_screen_allowed_depths_iterator(screen);

    while (depth_iterator.rem)
    {
        xcb_visualtype_iterator_t visual_iterator = xcb_depth_visuals_iterator(depth_iterator.data);

        if (depth_iterator.data->depth == 32)
        {
            return visual_iterator.data->visual_id;
        }

        xcb_depth_next(&depth_iterator);
    }

    insight_error("Failed to acquire truecolor visual");
    exit(EXIT_FAILURE);
}

void window_render_widgets(const window_s *const window, uint8_t force_render)
{
    for (uint8_t i = 0; i < window->number_of_widgets; ++i)
    {
        window->widgets[i]->render_requested |= force_render;
        window->widgets[i]->render(window->widgets[i]);
    }

    xcb_flush(window->display_server->connection);
}

void window_make_clickthrough(const window_s *const window)
{
    xcb_shape_rectangles(
        window->display_server->connection,
        XCB_SHAPE_SO_SET,
        XCB_SHAPE_SK_INPUT,
        XCB_CLIP_ORDERING_UNSORTED,
        window->id,
        0,
        0,
        0,
        NULL
    );
}

void window_set_wm_hints(const window_s *const window)
{
    window_set_wm_hint_wm_name(window);
    window_set_wm_hint_wm_class(window);
    window_set_wm_hint_net_wm_name(window);
    window_set_wm_hint_net_wm_state(window);
    window_set_wm_hint_net_wm_desktop(window);
    window_set_wm_hint_net_wm_window_type(window);

    xcb_flush(window->display_server->connection);
}

void window_set_wm_hint_wm_name(const window_s *const window)
{
    const char name[] = "hoverclock";
    const uint32_t name_length = strlen(name);

    xcb_change_property(
        window->display_server->connection,
        XCB_PROP_MODE_REPLACE,
        window->id,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        name_length,
        name
    );
}

void window_set_wm_hint_wm_class(const window_s *const window)
{
    const char name[] = "hoverclock";
    const uint32_t name_length = strlen(name);
    char *class_name = malloc((name_length * 2) + 2);

    strcpy(class_name, name);
    strcat(class_name, " ");
    strcat(class_name, name);
    class_name[name_length] = '\0';

    const uint32_t class_name_length = (name_length * 2) + 2;

    xcb_change_property(
        window->display_server->connection,
        XCB_PROP_MODE_REPLACE,
        window->id,
        XCB_ATOM_WM_CLASS,
        XCB_ATOM_STRING,
        8,
        class_name_length,
        class_name
    );

    free(class_name);
}

void window_set_wm_hint_net_wm_name(const window_s *const window)
{
    const xcb_atom_t net_wm_name_atom = display_server_get_atom(window->display_server, "_NET_WM_NAME");
    const xcb_atom_t utf8_string_atom = display_server_get_atom(window->display_server, "UTF8_STRING");

    if (net_wm_name_atom == XCB_ATOM_NONE || utf8_string_atom == XCB_ATOM_NONE)
    {
        return;
    }

    const char name[] = "hoverclock";
    const uint32_t name_length = strlen(name);

    xcb_change_property(
        window->display_server->connection,
        XCB_PROP_MODE_REPLACE,
        window->id,
        net_wm_name_atom,
        utf8_string_atom,
        8,
        name_length,
        name
    );
}

void window_set_wm_hint_net_wm_state(const window_s *const window)
{
    const xcb_atom_t net_wm_state_atom = display_server_get_atom(window->display_server, "_NET_WM_STATE");

    const xcb_atom_t net_wm_state_atom_values[] = {
        display_server_get_atom(window->display_server, "_NET_WM_STATE_ABOVE"),
        display_server_get_atom(window->display_server, "_NET_WM_STATE_STICKY"),
        display_server_get_atom(window->display_server, "_NET_WM_STATE_SKIP_PAGER"),
        display_server_get_atom(window->display_server, "_NET_WM_STATE_SKIP_TASKBAR")
    };

    if (net_wm_state_atom == XCB_ATOM_NONE)
    {
        return;
    }

    xcb_change_property(
        window->display_server->connection,
        XCB_PROP_MODE_REPLACE,
        window->id,
        net_wm_state_atom,
        XCB_ATOM_ATOM,
        32,
        sizeof(net_wm_state_atom_values) / sizeof(xcb_atom_t),
        net_wm_state_atom_values
    );
}

void window_set_wm_hint_net_wm_desktop(const window_s *const window)
{
    const xcb_atom_t net_wm_desktop_atom = display_server_get_atom(window->display_server, "_NET_WM_DESKTOP");
    const uint32_t net_wm_desktop_atom_value = 0xffffffff;

    if (net_wm_desktop_atom == XCB_ATOM_NONE)
    {
        return;
    }

    xcb_change_property(
        window->display_server->connection,
        XCB_PROP_MODE_REPLACE,
        window->id,
        net_wm_desktop_atom,
        XCB_ATOM_CARDINAL,
        32,
        1,
        &net_wm_desktop_atom_value
    );
}

void window_set_wm_hint_net_wm_window_type(const window_s *const window)
{
    const xcb_atom_t net_wm_window_type_atom = display_server_get_atom(window->display_server, "_NET_WM_WINDOW_TYPE");
    const xcb_atom_t net_wm_window_type_utility_atom =
        display_server_get_atom(window->display_server, "_NET_WM_WINDOW_TYPE_UTILITY");

    if (net_wm_window_type_atom == XCB_ATOM_NONE)
    {
        return;
    }

    xcb_change_property(
        window->display_server->connection,
        XCB_PROP_MODE_REPLACE,
        window->id,
        net_wm_window_type_atom,
        XCB_ATOM_ATOM,
        32,
        1,
        &net_wm_window_type_utility_atom
    );
}
