#include "hoverclock_xcb.h"
#include "../../../hoverclock.h"
#include "../../../hoverclock_time_date.h"
#include "cairo.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <xcb/shape.h>
#include <cairo/cairo-xcb.h>

static sig_atomic_t keep_running = 0;
static HoverclockPlatformXcb this;

void hoverclock_create(void)
{
    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = &hoverclock_xcb_on_quit_request;
    sigaction(SIGINT, &signal_action, NULL);

    hoverclock_settings_load(&this.settings);

    hoverclock_xcb_initialize();
    hoverclock_xcb_create_window();
    hoverclock_xcb_set_wm_hints();
    hoverclock_xcb_pangocairo_initialize();

    // Every X11 event is 32 bytes long
    this.expose_event = calloc(32, 1);
    this.expose_event->response_type = XCB_EXPOSE;
    this.expose_event->window = this.window;

    this.timer = inthreadval_create(hoverclock_refresh, 1000);
}

void hoverclock_destroy(void)
{
    xcb_flush(this.connection);

    inthreadval_stop(this.timer);
    inthreadval_destroy(this.timer);

    hoverclock_xcb_pangocairo_terminate();

    hoverclock_xcb_terminate();

    hoverclock_settings_unload(&this.settings);
}

void hoverclock_run(void)
{
    keep_running = 1;

    inthreadval_start(this.timer);

    uint32_t xcb_window_stack_mode_above = XCB_STACK_MODE_ABOVE;

    xcb_generic_event_t *event;

    while ((event = xcb_wait_for_event(this.connection)))
    {
        if ((event->response_type & ~0x80) == XCB_EXPOSE)
        {
            hoverclock_xcb_pangocairo_paint_background();
            hoverclock_xcb_pangocairo_draw_text_shadow();
            hoverclock_xcb_pangocairo_draw_text(this.settings.foreground);

            xcb_configure_window(this.connection,
                                 this.window,
                                 XCB_CONFIG_WINDOW_STACK_MODE,
                                 &xcb_window_stack_mode_above);

            cairo_paint(this.cairo_xcb_context);
            xcb_flush(this.connection);
        }

        free(event);

        if (!keep_running)
            break;
    }

    hoverclock_destroy();
}

void hoverclock_refresh(void)
{
    xcb_send_event(this.connection, 0, this.window, XCB_EVENT_MASK_EXPOSURE, (char *) this.expose_event);

    xcb_flush(this.connection);
}

void hoverclock_xcb_initialize(void)
{
    this.connection = xcb_connect(NULL, NULL);

    if (xcb_connection_has_error(this.connection))
    {
        fprintf(stderr, "hoverclock::error: Failed to establish X Server connection\n");
        exit(EXIT_FAILURE);
    }

    this.screen = xcb_setup_roots_iterator(xcb_get_setup(this.connection)).data;

    this.window_size.width = this.screen->width_in_pixels;
    this.window_size.height = this.screen->height_in_pixels;

    this.time_font_description = pango_font_description_from_string(this.settings.time.font);
    this.date_font_description = pango_font_description_from_string(this.settings.date.font);
}

void hoverclock_xcb_terminate(void)
{
    xcb_destroy_window(this.connection, this.window);
    free(this.expose_event);
    xcb_disconnect(this.connection);
}

void hoverclock_xcb_create_window(void)
{
    this.window = xcb_generate_id(this.connection);
    xcb_colormap_t colormap = xcb_generate_id(this.connection);

    // Find the visual before calculating the window size
    hoverclock_xcb_find_truecolor_visual();

    hoverclock_xcb_calculate_window_size();
    hoverclock_xcb_calculate_window_coordinates();

    xcb_create_colormap(this.connection, XCB_COLORMAP_ALLOC_NONE, colormap, this.screen->root, this.visual->visual_id);

    uint32_t mask =
        XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

    uint32_t values[] = { this.screen->black_pixel, 0, 1, XCB_EVENT_MASK_EXPOSURE, colormap };

    uint8_t color_depth = 32;

    if (this.visual->visual_id == this.screen->root_visual)
        color_depth = XCB_COPY_FROM_PARENT;

    xcb_create_window(this.connection,
                      color_depth,
                      this.window,
                      this.screen->root,
                      this.window_coordinates.x,
                      this.window_coordinates.y,
                      this.window_size.width,
                      this.window_size.height,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      this.visual->visual_id,
                      mask,
                      values);

    xcb_shape_rectangles(this.connection,
                         XCB_SHAPE_SO_SET,
                         XCB_SHAPE_SK_INPUT,
                         XCB_CLIP_ORDERING_UNSORTED,
                         this.window,
                         0,
                         0,
                         0,
                         NULL);

    xcb_map_window(this.connection, this.window);

    xcb_flush(this.connection);

    xcb_free_colormap(this.connection, colormap);
}

void hoverclock_xcb_set_wm_hints(void)
{
    const char name[] = "hoverclock";
    int name_length = strlen(name);

    // Set WM_NAME
    xcb_change_property(this.connection,
                        XCB_PROP_MODE_REPLACE,
                        this.window,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        name_length,
                        name);

    // Set WM_CLASS
    char *class_name = malloc((name_length * 2) + 2);

    strcpy(class_name, name);
    strcat(class_name, " ");
    strcat(class_name, name);
    class_name[name_length] = '\0';

    int class_name_length = (name_length * 2) + 2;

    xcb_change_property(this.connection,
                        XCB_PROP_MODE_REPLACE,
                        this.window,
                        XCB_ATOM_WM_CLASS,
                        XCB_ATOM_STRING,
                        8,
                        class_name_length,
                        class_name);

    free(class_name);

    // Set _NET_WM_NAME
    xcb_atom_t net_wm_name_atom = hoverclock_xcb_get_atom("_NET_WM_NAME");
    xcb_atom_t utf8_string_atom = hoverclock_xcb_get_atom("UTF8_STRING");

    if (net_wm_name_atom != XCB_ATOM_NONE && utf8_string_atom != XCB_ATOM_NONE)
    {
        xcb_change_property(this.connection,
                            XCB_PROP_MODE_REPLACE,
                            this.window,
                            net_wm_name_atom,
                            utf8_string_atom,
                            8,
                            name_length,
                            name);
    }

    // _NET_WM_WINDOW_TYPE
    xcb_atom_t net_wm_window_type_atom = hoverclock_xcb_get_atom("_NET_WM_WINDOW_TYPE");
    xcb_atom_t net_wm_window_type_utility_atom = hoverclock_xcb_get_atom("_NET_WM_WINDOW_TYPE_UTILITY");

    if (net_wm_window_type_atom != XCB_ATOM_NONE)
    {
        xcb_change_property(this.connection,
                            XCB_PROP_MODE_REPLACE,
                            this.window,
                            net_wm_window_type_atom,
                            XCB_ATOM_ATOM,
                            32,
                            1,
                            &net_wm_window_type_utility_atom);
    }

    // Set _NET_WM_STATE
    xcb_atom_t net_wm_state_atom = hoverclock_xcb_get_atom("_NET_WM_STATE");
    xcb_atom_t net_wm_state_atom_values[] = { hoverclock_xcb_get_atom("_NET_WM_STATE_ABOVE"),
                                              hoverclock_xcb_get_atom("_NET_WM_STATE_STICKY"),
                                              hoverclock_xcb_get_atom("_NET_WM_STATE_SKIP_PAGER"),
                                              hoverclock_xcb_get_atom("_NET_WM_STATE_SKIP_TASKBAR") };

    if (net_wm_state_atom != XCB_ATOM_NONE)
    {
        xcb_change_property(this.connection,
                            XCB_PROP_MODE_REPLACE,
                            this.window,
                            net_wm_state_atom,
                            XCB_ATOM_ATOM,
                            32,
                            sizeof(net_wm_state_atom_values) / sizeof(xcb_atom_t),
                            net_wm_state_atom_values);
    }

    // Set _NET_WM_DESKTOP to make hoverclock appear on all desktops
    xcb_atom_t net_wm_desktop_atom = hoverclock_xcb_get_atom("_NET_WM_DESKTOP");
    uint32_t net_wm_desktop_atom_value = 0xffffffff;

    if (net_wm_desktop_atom != XCB_ATOM_NONE)
    {
        xcb_change_property(this.connection,
                            XCB_PROP_MODE_REPLACE,
                            this.window,
                            net_wm_desktop_atom,
                            XCB_ATOM_CARDINAL,
                            32,
                            1,
                            &net_wm_desktop_atom_value);
    }

    xcb_flush(this.connection);
}

void hoverclock_xcb_calculate_window_size(void)
{
    cairo_surface_t *cairo_surface_priv = cairo_xcb_surface_create(this.connection,
                                                                   this.screen->root,
                                                                   this.visual,
                                                                   this.screen->width_in_pixels,
                                                                   this.screen->height_in_pixels);

    cairo_t *cairo_context_priv = cairo_create(cairo_surface_priv);
    PangoContext *pango_context_priv = pango_cairo_create_context(cairo_context_priv);

    unsigned int time_font_width = 0;
    unsigned int time_font_height = 0;

    unsigned int date_font_width = 0;
    unsigned int date_font_height = 0;

    uint8_t settings_have_time_format = strcmp(this.settings.time.format, " ") != 0;
    uint8_t settings_have_date_format = strcmp(this.settings.date.format, " ") != 0;

    if (settings_have_time_format)
    {
        hoverclock_get_formatted_time_date(this.settings.time.format, HOVERCLOCK_MAX_TEXT_LENGTH, this.text_buffer);
        time_font_height = pango_font_description_get_size(this.time_font_description) / PANGO_SCALE;
        time_font_width = hoverclock_xcb_pangocairo_get_text_width(this.text_buffer, this.time_font_description);
    }

    if (settings_have_date_format)
    {
        hoverclock_get_formatted_time_date(this.settings.date.format, HOVERCLOCK_MAX_TEXT_LENGTH, this.text_buffer);
        date_font_height = pango_font_description_get_size(this.date_font_description) / PANGO_SCALE;
        date_font_width = hoverclock_xcb_pangocairo_get_text_width(this.text_buffer, this.date_font_description);
    }

    this.window_size.width =
        (time_font_width > date_font_width ? time_font_width : date_font_width) + (this.settings.window_padding * 2);

    this.window_size.height = time_font_height + date_font_height + (this.settings.window_padding * 2) + 1;

    if (settings_have_time_format && settings_have_date_format)
        this.window_size.height += this.settings.line_spacing;

    g_object_unref(pango_context_priv);

    cairo_destroy(cairo_context_priv);
    cairo_surface_finish(cairo_surface_priv);
    cairo_surface_destroy(cairo_surface_priv);
}

void hoverclock_xcb_calculate_window_coordinates(void)
{
    this.window_coordinates.x = this.settings.margins.x;
    this.window_coordinates.y = this.settings.margins.y;

    unsigned int screen_width = this.screen->width_in_pixels;
    unsigned int screen_height = this.screen->height_in_pixels;

    // If the requested position is for the right side of the screen,
    // update the x coordinate of the window
    if (this.settings.position % 2 != 0)
        this.window_coordinates.x = screen_width - this.window_size.width - this.settings.margins.x;

    // If the requested position is for the bottom side of the screen,
    // update the y coordinate of the window
    if (this.settings.position >= 2)
        this.window_coordinates.y = screen_height - this.window_size.height - this.settings.margins.y;
}

void hoverclock_xcb_pangocairo_initialize(void)
{
    this.cairo_xcb_surface = cairo_xcb_surface_create(this.connection,
                                                      this.window,
                                                      this.visual,
                                                      this.window_size.width,
                                                      this.window_size.height);

    this.cairo_xcb_context = cairo_create(this.cairo_xcb_surface);

    this.cairo_paint_surface =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this.window_size.width, this.window_size.height);

    this.cairo_paint_context = cairo_create(this.cairo_paint_surface);

    this.pango_context = pango_cairo_create_context(this.cairo_paint_context);
    this.pango_layout = pango_layout_new(this.pango_context);

    pango_layout_set_width(this.pango_layout, this.window_size.width * PANGO_SCALE);
    pango_layout_set_height(this.pango_layout, this.window_size.height * PANGO_SCALE);
    pango_layout_set_alignment(this.pango_layout, PANGO_ALIGN_CENTER);

    cairo_set_source_surface(this.cairo_xcb_context, this.cairo_paint_surface, 0, 0);
    cairo_set_operator(this.cairo_xcb_context, CAIRO_OPERATOR_SOURCE);
    cairo_set_operator(this.cairo_paint_context, CAIRO_OPERATOR_SOURCE);

    hoverclock_xcb_pangocairo_remove_default_padding();

    this.date_padding = hoverclock_xcb_pangocairo_calculate_date_padding();
}

void hoverclock_xcb_pangocairo_terminate(void)
{
    pango_font_description_free(this.time_font_description);
    pango_font_description_free(this.date_font_description);

    pango_cairo_font_map_set_default(NULL);

    g_object_unref(this.pango_context);
    g_object_unref(this.pango_layout);

    cairo_destroy(this.cairo_xcb_context);
    cairo_surface_finish(this.cairo_xcb_surface);
    cairo_surface_destroy(this.cairo_xcb_surface);
    cairo_debug_reset_static_data();
}

void hoverclock_xcb_pangocairo_remove_default_padding(void)
{
    PangoFontMetrics *font_metrics = pango_context_get_metrics(this.pango_context, this.time_font_description, NULL);

    int font_ascent = pango_font_metrics_get_ascent(font_metrics) / PANGO_SCALE;
    int font_size = pango_font_description_get_size(this.time_font_description) / PANGO_SCALE;

    pango_font_metrics_unref(font_metrics);

    cairo_translate(this.cairo_paint_context, 0, font_size - font_ascent);
}

void hoverclock_xcb_pangocairo_draw_text_shadow(void)
{
    hoverclock_xcb_pangocairo_draw_text(this.settings.text_outline);

    uint8_t *pixels = cairo_image_surface_get_data(this.cairo_paint_surface);

    int width = cairo_image_surface_get_width(this.cairo_paint_surface);
    int height = cairo_image_surface_get_height(this.cairo_paint_surface);

    int box_size = 3;
    int radius = (int) (box_size / 2.0);

    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;
    int a_sum = 0;

    int pixel_index;

    for (int passes = 0; passes < 2; passes++)
    {
        for (int row = radius; row < height - radius; ++row)
        {
            for (int column = radius; column < width - radius; ++column)
            {
                r_sum = 0;
                g_sum = 0;
                b_sum = 0;
                a_sum = 0;

                for (int i = -radius; i <= radius; ++i)
                {
                    for (int j = -radius; j <= radius; ++j)
                    {
                        pixel_index = 4 * ((row + i) * width + (column + j));

                        a_sum += pixels[pixel_index + 3];
                        r_sum += pixels[pixel_index + 2];
                        g_sum += pixels[pixel_index + 1];
                        b_sum += pixels[pixel_index + 0];
                    }
                }

                pixels[(4 * (row * width + column)) + 3] = a_sum / (box_size * box_size);
                pixels[(4 * (row * width + column)) + 2] = r_sum / (box_size * box_size);
                pixels[(4 * (row * width + column)) + 1] = g_sum / (box_size * box_size);
                pixels[(4 * (row * width + column)) + 0] = b_sum / (box_size * box_size);
            }
        }
    }
}

void hoverclock_xcb_pangocairo_draw_text(HoverclockColor text_color)
{
    // Set text foreground color
    cairo_set_source_rgba(this.cairo_paint_context,
                          text_color.r / 255.0,
                          text_color.g / 255.0,
                          text_color.b / 255.0,
                          text_color.a / 255.0);

    uint8_t time_format_is_defined = strcmp(this.settings.time.format, " ") != 0;
    uint8_t date_format_is_defined = strcmp(this.settings.date.format, " ") != 0;

    // Draw the time text if the time format is not empty
    if (time_format_is_defined)
    {
        // Get the current system time
        hoverclock_get_formatted_time_date(this.settings.time.format, HOVERCLOCK_MAX_TEXT_LENGTH, this.text_buffer);

        // Set font and text
        pango_layout_set_font_description(this.pango_layout, this.time_font_description);
        pango_layout_set_text(this.pango_layout, this.text_buffer, -1);

        // Set padding
        cairo_move_to(this.cairo_paint_context, 0, this.settings.window_padding);

        // Display the text
        pango_cairo_show_layout(this.cairo_paint_context, this.pango_layout);
    }

    // Draw the date text if the date format is not empty
    if (date_format_is_defined)
    {
        // Get the current system date
        hoverclock_get_formatted_time_date(this.settings.date.format, HOVERCLOCK_MAX_TEXT_LENGTH, this.text_buffer);

        // Set font and text
        pango_layout_set_font_description(this.pango_layout, this.date_font_description);
        pango_layout_set_text(this.pango_layout, this.text_buffer, -1);

        // Set padding
        cairo_move_to(this.cairo_paint_context, 0, this.date_padding);

        // Display the text
        pango_cairo_show_layout(this.cairo_paint_context, this.pango_layout);
    }
}

void hoverclock_xcb_pangocairo_paint_background(void)
{
    cairo_set_source_rgba(this.cairo_paint_context,
                          this.settings.background.r / 255.0,
                          this.settings.background.g / 255.0,
                          this.settings.background.b / 255.0,
                          this.settings.background.a / 255.0);

    cairo_paint(this.cairo_paint_context);
}

unsigned int hoverclock_xcb_pangocairo_calculate_date_padding(void)
{
    uint8_t time_format_is_defined = strcmp(this.settings.time.format, " ") != 0;
    uint8_t date_format_is_defined = strcmp(this.settings.date.format, " ") != 0;

    if (!time_format_is_defined || !date_format_is_defined)
        return this.settings.window_padding + 1;

    PangoFontMetrics *time_font_metrics =
        pango_context_get_metrics(this.pango_context, this.time_font_description, NULL);

    PangoFontMetrics *date_font_metrics =
        pango_context_get_metrics(this.pango_context, this.date_font_description, NULL);

    int time_ascent = pango_font_metrics_get_ascent(time_font_metrics);
    int date_descent = pango_font_metrics_get_descent(date_font_metrics);
    
    pango_font_metrics_unref(time_font_metrics);
    pango_font_metrics_unref(date_font_metrics);

    int font_height = ((time_ascent - date_descent) / PANGO_SCALE);

    return font_height + this.settings.line_spacing + this.settings.window_padding;
}

unsigned int hoverclock_xcb_pangocairo_get_text_width(const char *text, PangoFontDescription *font)
{
    cairo_surface_t *surface = cairo_xcb_surface_create(this.connection,
                                                        this.screen->root,
                                                        this.visual,
                                                        this.screen->width_in_pixels,
                                                        this.screen->height_in_pixels);
    cairo_t *context = cairo_create(surface);

    PangoLayout *layout = pango_cairo_create_layout(context);

    pango_layout_set_text(layout, text, -1);
    pango_layout_set_font_description(layout, font);

    int width;
    pango_layout_get_pixel_size(layout, &width, NULL);

    g_object_unref(layout);
    cairo_destroy(context);
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);

    return width;
}

void hoverclock_xcb_find_truecolor_visual(void)
{
    xcb_depth_iterator_t depth_iterator = xcb_screen_allowed_depths_iterator(this.screen);

    xcb_visualtype_t *fallback_visual = NULL;

    while (depth_iterator.rem)
    {
        xcb_visualtype_iterator_t visual_iterator = xcb_depth_visuals_iterator(depth_iterator.data);

        // If a truecolor (32 bit) visual is found, set this.visual to the found one
        if (depth_iterator.data->depth == 32)
        {
            this.visual = visual_iterator.data;
            return;
        }

        // While looping through the depth iterator, also get the screen's root visual as a fallback
        if (!fallback_visual)
        {
            while (visual_iterator.rem)
            {
                if (visual_iterator.data->visual_id == this.screen->root_visual)
                    fallback_visual = visual_iterator.data;

                xcb_visualtype_next(&visual_iterator);
            }
        }

        xcb_depth_next(&depth_iterator);
    }

    if (fallback_visual)
    {
        fprintf(stderr, "hoverclock::xcb_error: Failed to find a truecolor visual. Using fallback visual\n");
        this.visual = fallback_visual;
    }
}

xcb_atom_t hoverclock_xcb_get_atom(const char *name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(this.connection, 0, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(this.connection, cookie, NULL);

    if (!reply)
        return XCB_ATOM_NONE;

    xcb_atom_t atom = reply->atom;

    free(reply);

    return atom;
}

void hoverclock_xcb_on_quit_request(int signal_type)
{
    if (signal_type == SIGINT)
    {
        keep_running = 0;
        hoverclock_refresh();
    }
}
