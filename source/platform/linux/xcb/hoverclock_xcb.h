#ifndef HOVERCLOCK_XCB_H
#define HOVERCLOCK_XCB_H

#include <xcb/xcb.h>
#include <pango/pangocairo.h>
#include "../../../hoverclock_settings.h"
#include "../../../inthreadval/include/inthreadval.h"

#define HOVERCLOCK_MAX_TEXT_LENGTH 64

typedef struct HoverclockPlatformXcb
{
    xcb_window_t window;
    xcb_screen_t *screen;
    xcb_visualtype_t *visual;
    xcb_connection_t *connection;
    xcb_expose_event_t *expose_event;

    cairo_t *cairo_xcb_context;
    cairo_t *cairo_paint_context;
    cairo_surface_t *cairo_xcb_surface;
    cairo_surface_t *cairo_paint_surface;

    PangoLayout *pango_layout;
    PangoContext *pango_context;
    PangoFontDescription *time_font_description;
    PangoFontDescription *date_font_description;

    HoverclockPair window_size;
    HoverclockPair window_coordinates;

    HoverclockSettings settings;

    Inthreadval *timer;

    uint16_t date_padding;
    char text_buffer[HOVERCLOCK_MAX_TEXT_LENGTH];
} HoverclockPlatformXcb;

// XCB functions
void hoverclock_xcb_initialize(void);
void hoverclock_xcb_terminate(void);

void hoverclock_xcb_create_window(void);
void hoverclock_xcb_calculate_window_size(void);
void hoverclock_xcb_calculate_window_coordinates(void);

// Cairo/Pango functions
void hoverclock_xcb_pangocairo_initialize(void);
void hoverclock_xcb_pangocairo_terminate(void);

void hoverclock_xcb_pangocairo_paint_background(void);
void hoverclock_xcb_pangocairo_draw_text_shadow(void);
void hoverclock_xcb_pangocairo_draw_text(HoverclockColor text_color);

void hoverclock_xcb_pangocairo_remove_default_padding(void);
unsigned int hoverclock_xcb_pangocairo_calculate_date_padding(void);
unsigned int hoverclock_xcb_pangocairo_get_text_width(const char *text, PangoFontDescription *font);

// XCB Helper functions
void hoverclock_xcb_set_wm_hints(void);
void hoverclock_xcb_find_truecolor_visual(void);
xcb_atom_t hoverclock_xcb_get_atom(const char *name);

// Signals
void hoverclock_xcb_on_quit_request(int signal_type);

#endif // HOVERCLOCK_XCB_H
