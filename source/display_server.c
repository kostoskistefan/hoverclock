#include "display_server.h"
#include <stdlib.h>
#include <string.h>
#include <insight.h>

void display_server_create(display_server_s *const display_server)
{
    display_server->connection = xcb_connect(0, 0);

    if (xcb_connection_has_error(display_server->connection))
    {
        insight_error("Failed to establish X Server connection");
        free(display_server);
        exit(EXIT_FAILURE);
    }

    display_server->screen = xcb_setup_roots_iterator(xcb_get_setup(display_server->connection)).data;
}

void display_server_destroy(display_server_s *const display_server)
{
    xcb_disconnect(display_server->connection);
}

xcb_atom_t display_server_get_atom(const display_server_s *const display_server, const char *name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(display_server->connection, 0, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(display_server->connection, cookie, NULL);

    if (!reply)
        return XCB_ATOM_NONE;

    xcb_atom_t atom = reply->atom;

    free(reply);

    return atom;
}
