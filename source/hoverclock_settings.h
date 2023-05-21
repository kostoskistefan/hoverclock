#ifndef HOVERCLOCK_SETTINGS_H
#define HOVERCLOCK_SETTINGS_H

#include "hoverclock_types.h"

typedef struct HoverclockSettings
{
    unsigned int line_spacing;
    unsigned int window_padding;

    HoverclockPosition position;

    HoverclockPair margins;

    HoverclockColor foreground;
    HoverclockColor background;
    HoverclockColor text_outline;

    HoverclockTextStyle time;
    HoverclockTextStyle date;
} HoverclockSettings;

void hoverclock_settings_load(HoverclockSettings *settings);
void hoverclock_settings_unload(HoverclockSettings *settings);

#endif // HOVERCLOCK_SETTINGS_H
