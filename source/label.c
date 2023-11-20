#include "label.h"
#include "color.h"
#include "surface.h"
#include "rectangle.h"
#include <stdlib.h>
#include <string.h>
#include <fontconfig/fontconfig.h>

#define LABEL_SHADOW_BLUR_RADIUS   1
#define LABEL_SHADOW_BLUR_PASSES   2
#define LABEL_SHADOW_BLUR_BOX_SIZE ((uint32_t)((LABEL_SHADOW_BLUR_RADIUS + 0.5) * 2))
#define LABEL_VERTICAL_PADDING     (LABEL_SHADOW_BLUR_RADIUS)
#define LABEL_HORIZONTAL_PADDING   10

char *label_get_font_path_by_font_name(const char *font_name);
void label_render_character(
    surface_s *const surface,
    const FT_Face font,
    const uint32_t x,
    const uint32_t y,
    const char character,
    const color_u color
);
void label_clear(label_s *const label);
void label_render(label_s *const label);
void label_render_text(label_s *const label, const color_u text_color);

void label_create(
    label_s *const label,
    FT_Library *font_library,
    window_s *const window,
    const char *text,
    const char *font,
    const uint8_t font_size
)
{
    label->font = NULL;
    label->font_size = font_size;
    label->font_library = font_library;

    label->text[0] = 0;
    strncpy(label->text, text, LABEL_MAXIMUM_TEXT_LENGTH);

    label->text_color = (color_u) { .argb = 0xffffffff };
    label->shadow_color = (color_u) { .argb = 0xff000000 };

    label->widget.window = window;
    label->widget.render_requested = 1;

    char *font_path = label_get_font_path_by_font_name(font);
    FT_New_Face(*label->font_library, font_path, 0, &label->font);
    free(font_path);

    FT_Set_Pixel_Sizes(label->font, label->font_size, label->font_size);

    label->widget.rectangle = (rectangle_s) { 0 };

    label->widget.rectangle.width =
        label_get_text_width(label) +
        (LABEL_SHADOW_BLUR_BOX_SIZE * 2) +
        (LABEL_HORIZONTAL_PADDING << 1);

    label->widget.rectangle.height =
        font_size +
        LABEL_SHADOW_BLUR_BOX_SIZE +
        (LABEL_VERTICAL_PADDING << 1);

    surface_create(
        &label->widget.surface,
        label->widget.rectangle.width,
        label->widget.rectangle.height
    );

    label->widget.render = (void (*)(const widget_s *const)) label_render;
}

void label_destroy(label_s *const label)
{
    surface_destroy(&label->widget.surface);
    FT_Done_Face(label->font);
}

void label_move(label_s *const label, int32_t x, int32_t y)
{
    label->widget.rectangle.x = x;
    label->widget.rectangle.y = y;

    label->widget.render_requested = 1;
}

void label_set_text(label_s *const label, const char *text)
{
    if (strncmp(label->text, text, LABEL_MAXIMUM_TEXT_LENGTH) == 0)
        return;

    strncpy(label->text, text, LABEL_MAXIMUM_TEXT_LENGTH);

    label->widget.render_requested = 1;
}

void label_set_text_color(label_s *const label, const color_u color)
{
    label->text_color = color;
    label->widget.render_requested = 1;
}

void label_set_shadow_color(label_s *const label, const color_u color)
{
    label->shadow_color = color;
    label->widget.render_requested = 1;
}

int32_t label_get_text_width(const label_s *const label)
{
    int32_t text_width = 0;

    for (int32_t i = 0; i < strlen(label->text); ++i)
    {
        FT_Load_Char(label->font, label->text[i], FT_LOAD_ADVANCE_ONLY);
        text_width += (label->font->glyph->advance.x >> 6);
    }

    return text_width;
}

int32_t label_get_font_height_from_baseline(const label_s *const label)
{
    return (label->font->size->metrics.ascender - labs(label->font->size->metrics.descender)) >> 6;
}

void label_clear(struct label_s *const label)
{
    surface_paint(&label->widget.surface, label->widget.window->background_color);
}

void label_render(label_s *const label)
{
    if (label->widget.render_requested == 0)
        return;

    label_clear(label);

    label_render_text(label, label->shadow_color);

    surface_blur(
        &label->widget.surface,
        LABEL_SHADOW_BLUR_RADIUS,
        LABEL_SHADOW_BLUR_PASSES
    );

    label_render_text(label, label->text_color);

    xcb_put_image(
        label->widget.window->display_server->connection,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        label->widget.window->id,
        label->widget.window->graphical_context,
        label->widget.rectangle.width,
        label->widget.rectangle.height,
        label->widget.rectangle.x,
        label->widget.rectangle.y,
        0,
        32,
        label->widget.surface.data_length,
        label->widget.surface.data
    );

    label->widget.render_requested = 0;
}

void label_render_text(label_s *const label, const color_u text_color)
{
    uint32_t x_position = ((label->widget.rectangle.width - label_get_text_width(label)) >> 1);
    const uint32_t y_position = label->font_size + (LABEL_SHADOW_BLUR_RADIUS >> 1) + LABEL_VERTICAL_PADDING;

    for (uint8_t i = 0; i < strlen(label->text); ++i)
    {
        label_render_character(
            &label->widget.surface,
            label->font,
            x_position,
            y_position,
            label->text[i],
            text_color
        );

        x_position += label->font->glyph->advance.x >> 6;
    }
}

void label_render_character(
    surface_s *const surface,
    const FT_Face font,
    const uint32_t x,
    const uint32_t y,
    const char character,
    const color_u color
)
{
    FT_Load_Char(font, character, FT_LOAD_RENDER);

    const int32_t horizontal_bearing = font->glyph->metrics.horiBearingX >> 6;
    const uint32_t row_offset = y - font->glyph->bitmap_top;
    const uint32_t column_offset = horizontal_bearing + x;
    const float alpha_scaler = ((float) color.a) / UINT16_MAX;

    for (uint32_t row = 0; row < font->glyph->bitmap.rows; ++row)
    {
        const uint32_t pixel_row = row + row_offset;

        // If the current row is outside of the surface, there is no need to continue onto the following rows
        if (pixel_row >= surface->height)
            break;

        const uint32_t pixel_row_index = pixel_row * surface->width;
        const uint32_t bitmap_row_offset = row * font->glyph->bitmap.width;

        for (uint32_t column = 0; column < font->glyph->bitmap.width; ++column)
        {
            const uint32_t pixel_column_index = column + column_offset;

            // If the current column is outside of the surface, there is no need to continue onto the following columns
            if (pixel_column_index >= surface->width)
                break;

            const uint8_t current_pixel = font->glyph->bitmap.buffer[bitmap_row_offset + column];

            // If the current glyph pixel is transparent, continue onto the next pixel
            if (current_pixel == 0x00)
                continue;

            const uint32_t pixel_index = (pixel_row_index + pixel_column_index) << 2;

            // If the current glyph pixel is opaque, skip alpha compositing,
            // copy the input color directly and continue onto the next pixel
            if (current_pixel == 0xff)
            {
                *(uint32_t *) &surface->data[pixel_index] = color.argb;
                continue;
            }

            const float normalized_alpha = current_pixel * alpha_scaler;
            const float inverse_alpha = 1.0f - normalized_alpha;

            const color_u source_color = color;
            const color_u destination_color = { .argb = *(uint32_t *) &surface->data[pixel_index] };

            const color_u color_out = {
                .a = normalized_alpha * 255.0 + inverse_alpha * destination_color.a,
                .r = (uint8_t) ((source_color.r * normalized_alpha) + (destination_color.r * inverse_alpha)),
                .g = (uint8_t) ((source_color.g * normalized_alpha) + (destination_color.g * inverse_alpha)),
                .b = (uint8_t) ((source_color.b * normalized_alpha) + (destination_color.b * inverse_alpha)),
            };

            *(uint32_t *) &surface->data[pixel_index] = color_out.argb;
        }
    }
}

char *label_get_font_path_by_font_name(const char *font_name)
{
    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pattern = FcNameParse((const FcChar8 *) font_name);

    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;
    FcPattern *font = FcFontMatch(config, pattern, &result);

    FcPatternDestroy(pattern);
    FcConfigDestroy(config);

    if (!font)
        return NULL;

    FcChar8 *file = NULL;
    char *path = NULL;

    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
    {
        path = malloc(strlen((char *) file) * sizeof(char) + 1);
        strcpy(path, (char *) file);
    }

    FcPatternDestroy(font);
    FcFini();

    return path;
}
