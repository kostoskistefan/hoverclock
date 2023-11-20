#ifndef LABEL_H
#define LABEL_H

#include "color.h"
#include "window.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#define LABEL_MAXIMUM_TEXT_LENGTH 64

typedef struct label_s
{
    widget_s widget;

    FT_Face font;
    uint8_t font_size;
    FT_Library *font_library;

    color_u text_color;
    color_u shadow_color;

    char text[LABEL_MAXIMUM_TEXT_LENGTH];
} label_s;

void label_create(
    label_s *const label,
    FT_Library *font_library,
    window_s *const window,
    const char *text,
    const char *font,
    const uint8_t font_size
);
void label_destroy(label_s *const label);

void label_move(label_s *const label, int32_t x, int32_t y);
void label_set_text(label_s *const label, const char *text);
void label_set_text_color(label_s *const label, const color_u color);
void label_set_shadow_color(label_s *const label, const color_u color);

int32_t label_get_text_width(const label_s *const label);
int32_t label_get_font_height_from_baseline(const label_s *const label);

#endif // LABEL_H
