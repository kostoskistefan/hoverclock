#ifndef SETTINGS_H
#define SETTINGS_H

#include "color.h"
#include <stdint.h>

typedef struct settings_s
{
    color_u window_background_color;
    char *window_location_name;
    uint32_t window_vertical_margin;
    uint32_t window_horizontal_margin;

    uint8_t time_label_visible;
    char *time_label_font_name;
    uint8_t time_label_font_size;
    color_u time_label_text_color;
    color_u time_label_shadow_color;
    char *time_label_format;

    uint8_t date_label_visible;
    char *date_label_font_name;
    uint8_t date_label_font_size;
    color_u date_label_text_color;
    color_u date_label_shadow_color;
    char *date_label_format;

    uint8_t vertical_spacing_between_labels;
} settings_s;

void settings_load(settings_s *const settings);
void settings_destroy(settings_s *const settings);

#endif // SETTINGS_H
