#include "color.h"
#include "label.h"
#include "settings.h"
#include "window.h"
#include "display_server.h"
#include <signal.h>
#include <stdlib.h>
#include <insight.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <inthreadval.h>

window_s window;
label_s time_label;
label_s date_label;
char time_format[LABEL_MAXIMUM_TEXT_LENGTH];
char date_format[LABEL_MAXIMUM_TEXT_LENGTH];
char formatted_time[LABEL_MAXIMUM_TEXT_LENGTH];
char formatted_date[LABEL_MAXIMUM_TEXT_LENGTH];

void update_formatted_time_date_strings(void);
void update_labels_text(void);
void on_quit_request(int signal_type);
void setup_signal_handler(void);

int main(int argc, char *argv[])
{
    insight_set_project_name("hoverclock");

    setup_signal_handler();

    settings_s settings;
    settings_load(&settings);

    FT_Library font_library;
    FT_Init_FreeType(&font_library);

    display_server_s display_server;
    display_server_create(&display_server);

    window_create(&window, &display_server);
    window_set_background_color(&window, settings.window_background_color);

    strncpy(time_format, settings.time_label_format, LABEL_MAXIMUM_TEXT_LENGTH);
    strncpy(date_format, settings.date_label_format, LABEL_MAXIMUM_TEXT_LENGTH);

    update_formatted_time_date_strings();

    label_create(
        &time_label,
        &font_library,
        &window,
        formatted_time,
        settings.time_label_font_name,
        settings.time_label_font_size
    );

    label_set_text_color(&time_label, settings.time_label_text_color);
    label_set_shadow_color(&time_label, settings.time_label_shadow_color);

    label_create(
        &date_label,
        &font_library,
        &window,
        formatted_date,
        settings.date_label_font_name,
        settings.date_label_font_size
    );

    label_set_text_color(&date_label, settings.date_label_text_color);
    label_set_shadow_color(&date_label, settings.date_label_shadow_color);

    const int32_t time_label_width = (int32_t) time_label.widget.rectangle.width;
    const int32_t date_label_width = (int32_t) date_label.widget.rectangle.width;
    const int32_t label_x_offset = (time_label_width - date_label_width) >> 1;

    // Labels need to be moved to the required position before calling window_add_widget,
    // because their rectangles are used to calculate the window size
    label_move(
        &time_label,
        WINDOW_HORIZONTAL_PADDING + (settings.date_label_visible ? ((label_x_offset < 0) ? -label_x_offset : 0) : 0),
        WINDOW_VERTICAL_PADDING
    );

    label_move(
        &date_label,
        WINDOW_HORIZONTAL_PADDING + ((label_x_offset > 0) ? label_x_offset : 0),
        settings.vertical_spacing_between_labels + (settings.time_label_visible ? time_label.widget.rectangle.height : 0)
    );

    if (settings.time_label_visible)
        window_add_widget(&window, (widget_s *) &time_label);

    if (settings.date_label_visible)
        window_add_widget(&window, (widget_s *) &date_label);

    window_calculate_location(
        &window,
        settings.window_location_name,
        settings.window_horizontal_margin,
        settings.window_vertical_margin
    );

    inthreadval_s *inthreadval = inthreadval_create(update_labels_text, 1000);
    inthreadval_start(inthreadval);

    window_run(&window);

    inthreadval_stop(inthreadval);
    inthreadval_destroy(inthreadval);
    label_destroy(&time_label);
    label_destroy(&date_label);
    settings_destroy(&settings);
    FT_Done_FreeType(font_library);
    window_destroy(&window);
    display_server_destroy(&display_server);

    return 0;
}

void update_formatted_time_date_strings(void)
{
    time_t raw_time = time(0);
    struct tm *current_time_date = localtime(&raw_time);

    strftime(formatted_time, sizeof(formatted_time), time_format, current_time_date);
    strftime(formatted_date, sizeof(formatted_date), date_format, current_time_date);
}

void update_labels_text(void)
{
    update_formatted_time_date_strings();

    label_set_text(&time_label, formatted_time);
    label_set_text(&date_label, formatted_date);

    window_refresh(&window);
}

void on_quit_request(int signal_type)
{
    if (signal_type == SIGINT)
        window_on_quit_request(&window);
}

void setup_signal_handler(void)
{
    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = &on_quit_request;
    sigaction(SIGINT, &signal_action, NULL);
}
