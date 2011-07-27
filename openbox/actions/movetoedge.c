#include "openbox/actions.h"
#include "openbox/actions_value.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/geom.h"
#include <glib.h>

typedef struct {
    ObDirection dir;
} Options;

static gpointer setup_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(ObActionsData *data, gpointer options);
/* 3.4-compatibility */
static gpointer setup_north_func(GHashTable *config);
static gpointer setup_south_func(GHashTable *config);
static gpointer setup_east_func(GHashTable *config);
static gpointer setup_west_func(GHashTable *config);

void action_movetoedge_startup(void)
{
    actions_register("MoveToEdge", setup_func, free_func, run_func);
    /* 3.4-compatibility */
    actions_register("MoveToEdgeNorth", setup_north_func, free_func, run_func);
    actions_register("MoveToEdgeSouth", setup_south_func, free_func, run_func);
    actions_register("MoveToEdgeEast", setup_east_func, free_func, run_func);
    actions_register("MoveToEdgeWest", setup_west_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_NORTH;

    v = g_hash_table_lookup(config, "direction");
    if (v && actions_value_is_string(v)) {
        const gchar *s = actions_value_string(v);
        if (!g_ascii_strcasecmp(s, "north") ||
            !g_ascii_strcasecmp(s, "up"))
            o->dir = OB_DIRECTION_NORTH;
        else if (!g_ascii_strcasecmp(s, "south") ||
                 !g_ascii_strcasecmp(s, "down"))
            o->dir = OB_DIRECTION_SOUTH;
        else if (!g_ascii_strcasecmp(s, "west") ||
                 !g_ascii_strcasecmp(s, "left"))
            o->dir = OB_DIRECTION_WEST;
        else if (!g_ascii_strcasecmp(s, "east") ||
                 !g_ascii_strcasecmp(s, "right"))
            o->dir = OB_DIRECTION_EAST;
    }

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        gint x, y;

        client_find_move_directional(data->client, o->dir, &x, &y);
        if (x != data->client->area.x || y != data->client->area.y) {
            actions_client_move(data, TRUE);
            client_move(data->client, x, y);
            actions_client_move(data, FALSE);
        }
    }

    return FALSE;
}

/* 3.4-compatibility */
static gpointer setup_north_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_WEST;
    return o;
}

