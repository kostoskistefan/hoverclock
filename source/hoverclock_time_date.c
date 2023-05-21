#include "hoverclock_time_date.h"
#include <time.h>

void hoverclock_get_formatted_time_date(const char *format, int out_buffer_length, char *out_buffer)
{
    time_t raw_time = time(NULL);
    struct tm *current_time_date = localtime(&raw_time);

    strftime(out_buffer, out_buffer_length, format, current_time_date);
}
