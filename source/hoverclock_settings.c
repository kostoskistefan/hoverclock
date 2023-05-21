#include "hoverclock_settings.h"
#include "vault/include/vault.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hoverclock_settings_load(HoverclockSettings *settings)
{
    Vault *vault = vault_create("hoverclock");

    uint8_t vault_exists = vault_file_exists(vault);

    if (vault_exists)
        vault_load(vault);

    else
    {
        vault_insert(vault, "Position", "3");
        vault_insert(vault, "LineSpacing", "5");
        vault_insert(vault, "WindowPadding", "5");
        vault_insert(vault, "VerticalMargin", "20");
        vault_insert(vault, "HorizontalMargin", "20");
        vault_insert(vault, "BackgroundColor", "0x00000000");
        vault_insert(vault, "ForegroundColor", "0xffffff99");
        vault_insert(vault, "TextOutlineColor", "0x00000099");

        vault_insert(vault, "TimeFont", "DejaVu 10");
        vault_insert(vault, "TimeFormat", "%H:%M");

        vault_insert(vault, "DateFont", "DejaVu 8");
        vault_insert(vault, "DateFormat", "%a %d %b");

        vault_save(vault);
    }

    sscanf(vault_find(vault, "Position"), "%d", &settings->position);
    sscanf(vault_find(vault, "LineSpacing"), "%u", &settings->line_spacing);
    sscanf(vault_find(vault, "WindowPadding"), "%u", &settings->window_padding);
    sscanf(vault_find(vault, "VerticalMargin"), "%u", &settings->margins.y);
    sscanf(vault_find(vault, "HorizontalMargin"), "%u", &settings->margins.x);
    sscanf(vault_find(vault, "BackgroundColor"), "%x", &settings->background.rgba);
    sscanf(vault_find(vault, "ForegroundColor"), "%x", &settings->foreground.rgba);
    sscanf(vault_find(vault, "TextOutlineColor"), "%x", &settings->text_outline.rgba);

    settings->time.font = strdup(vault_find(vault, "TimeFont"));
    settings->time.format = strdup(vault_find(vault, "TimeFormat"));

    settings->date.font = strdup(vault_find(vault, "DateFont"));
    settings->date.format = strdup(vault_find(vault, "DateFormat"));

    vault_destroy(vault);
}

void hoverclock_settings_unload(HoverclockSettings *settings)
{
    free(settings->time.font);
    free(settings->date.font);
    free(settings->time.format);
    free(settings->date.format);
}
