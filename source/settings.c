#include "settings.h"
#include <vault.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t settings_get_vault_uint(vault_s *const vault, const char *vault_key);
uint32_t settings_get_vault_hex(vault_s *const vault, const char *vault_key);
char *settings_get_vault_string(vault_s *const vault, const char *vault_key);

void settings_load(settings_s *const settings)
{
    vault_s *vault = vault_create("hoverclock");

    const uint8_t vault_exists = vault_file_exists(vault);

    if (vault_exists)
    {
        vault_load(vault);
    }

    else
    {
        vault_insert(vault, "window_vertical_margin", "30");
        vault_insert(vault, "window_horizontal_margin", "40");
        vault_insert(vault, "window_location_name", "top left");
        vault_insert(vault, "window_background_color", "0x00000000");

        vault_insert(vault, "time_label_visible", "1");
        vault_insert(vault, "time_label_format", "%H:%M");
        vault_insert(vault, "time_label_font_name", "Roboto:bold");
        vault_insert(vault, "time_label_font_size", "13");
        vault_insert(vault, "time_label_text_color", "0xffffffff");
        vault_insert(vault, "time_label_shadow_color", "0xff000000");

        vault_insert(vault, "date_label_visible", "1");
        vault_insert(vault, "date_label_format", "%a %d %b");
        vault_insert(vault, "date_label_font_name", "Roboto:regular");
        vault_insert(vault, "date_label_font_size", "11");
        vault_insert(vault, "date_label_text_color", "0xffffffff");
        vault_insert(vault, "date_label_shadow_color", "0xff000000");

        vault_insert(vault, "vertical_spacing_between_labels", "5");

        vault_save(vault);
    }

    settings->window_location_name = settings_get_vault_string(vault, "window_location_name");
    settings->window_background_color.argb = settings_get_vault_hex(vault, "window_background_color");
    settings->window_vertical_margin = settings_get_vault_uint(vault, "window_vertical_margin");
    settings->window_horizontal_margin = settings_get_vault_uint(vault, "window_horizontal_margin");

    settings->time_label_visible = settings_get_vault_uint(vault, "time_label_visible");
    settings->time_label_font_name = settings_get_vault_string(vault, "time_label_font_name");
    settings->time_label_font_size = settings_get_vault_uint(vault, "time_label_font_size");
    settings->time_label_text_color.argb = settings_get_vault_hex(vault, "time_label_text_color");
    settings->time_label_shadow_color.argb = settings_get_vault_hex(vault, "time_label_shadow_color");
    settings->time_label_format = settings_get_vault_string(vault, "time_label_format");

    settings->date_label_visible = settings_get_vault_uint(vault, "date_label_visible");
    settings->date_label_font_name = settings_get_vault_string(vault, "date_label_font_name");
    settings->date_label_font_size = settings_get_vault_uint(vault, "date_label_font_size");
    settings->date_label_text_color.argb = settings_get_vault_hex(vault, "date_label_text_color");
    settings->date_label_shadow_color.argb = settings_get_vault_hex(vault, "date_label_shadow_color");
    settings->date_label_format = settings_get_vault_string(vault, "date_label_format");

    settings->vertical_spacing_between_labels = settings_get_vault_uint(vault, "vertical_spacing_between_labels");

    vault_destroy(vault);
}

void settings_destroy(settings_s *const settings)
{
    free(settings->window_location_name);

    free(settings->time_label_format);
    free(settings->time_label_font_name);

    free(settings->date_label_format);
    free(settings->date_label_font_name);
}

uint32_t settings_get_vault_uint(vault_s *const vault, const char *vault_key)
{
    uint32_t value;

    sscanf(vault_find(vault, vault_key), "%u", &value);

    return value;
}

uint32_t settings_get_vault_hex(vault_s *const vault, const char *vault_key)
{
    uint32_t value;

    sscanf(vault_find(vault, vault_key), "%x", &value);

    return value;
}

char *settings_get_vault_string(vault_s *const vault, const char *vault_key)
{
    return strdup(vault_find(vault, vault_key));
}
