#ifndef DISPLAY_SERVER_H
#define DISPLAY_SERVER_H

#include <xcb/xcb.h>

typedef struct display_server_s
{
    xcb_screen_t *screen;
    xcb_connection_t *connection;
} display_server_s;

void display_server_create(display_server_s *const display_server);
void display_server_destroy(display_server_s *const display_server);

xcb_atom_t display_server_get_atom(const display_server_s *const display_server, const char *name);

#endif // DISPLAY_SERVER_H
