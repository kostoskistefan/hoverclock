#include "surface.h"
#include "color.h"
#include <stdlib.h>

#define SURFACE_BITS_PER_CHANNEL 4

void surface_create(surface_s *const surface, uint32_t width, uint32_t height)
{
    surface->width = width;
    surface->height = height;
    surface->stride = width * SURFACE_BITS_PER_CHANNEL;

    surface->data_length = height * surface->stride;
    surface->data = calloc(surface->data_length, sizeof(uint8_t));
}

void surface_destroy(surface_s *const surface)
{
    free(surface->data);
}

void surface_paint(surface_s *const surface, const color_u color)
{
    // Premultiply RGB channels with the alpha channel
    const color_u premultiplied_color = color_multiply(color, color.a / 255.0);

    for (uint32_t row = 0; row < surface->height; ++row)
    {
        const uint32_t row_stride = row * surface->stride;

        for (uint32_t column = 0; column < surface->width; ++column)
        {
            *((uint32_t *) &surface->data[row_stride + (column << 2)]) = premultiplied_color.argb;
        }
    }
}

void surface_blur(surface_s *const surface, const uint8_t radius, const uint8_t number_of_passes)
{
    uint32_t r_sum = 0;
    uint32_t g_sum = 0;
    uint32_t b_sum = 0;
    uint32_t a_sum = 0;
    uint32_t divisor = 0;

    for (uint8_t passes = 0; passes < number_of_passes; ++passes)
    {
        for (uint32_t row = radius; row < surface->height - radius; ++row)
        {
            const uint32_t row_stride = row * surface->stride;

            for (uint32_t column = radius; column < surface->width - radius; ++column)
            {
                const uint32_t pixel_index = row_stride + (column << 2);

                r_sum = 0;
                g_sum = 0;
                b_sum = 0;
                a_sum = 0;
                divisor = 0;

                for (int32_t kernel_row = -radius; kernel_row <= radius; ++kernel_row)
                {
                    const int32_t kernel_row_offset = row + kernel_row;

                    if (kernel_row_offset >= surface->height)
                        continue;

                    const int32_t kernel_row_stride = kernel_row_offset * surface->stride;

                    for (int32_t kernel_column = -radius; kernel_column <= radius; ++kernel_column)
                    {
                        const int32_t kernel_column_offset = column + kernel_column;

                        if (kernel_column_offset >= surface->width)
                            continue;

                        const int32_t kernel_pixel_index = kernel_row_stride + (kernel_column_offset << 2);

                        a_sum += surface->data[kernel_pixel_index + 3];
                        r_sum += surface->data[kernel_pixel_index + 2];
                        g_sum += surface->data[kernel_pixel_index + 1];
                        b_sum += surface->data[kernel_pixel_index];

                        ++divisor;
                    }
                }

                surface->data[pixel_index + 3] = a_sum / divisor;
                surface->data[pixel_index + 2] = r_sum / divisor;
                surface->data[pixel_index + 1] = g_sum / divisor;
                surface->data[pixel_index] = b_sum / divisor;
            }
        }
    }
}
