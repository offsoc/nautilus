/* nautilusgtkplacesview.c
 *
 * Copyright (C) 2015 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n.h>

#include <gio/gio.h>

#include "nautilusgtkplacesviewprivate.h"

struct _NautilusGtkPlacesView
{
    GtkBox parent_instance;

    GFile *server_list_file;
    GFileMonitor *server_list_monitor;

    guint loading : 1;
};

static void        populate_servers (NautilusGtkPlacesView *view);

static void        nautilus_gtk_places_view_set_loading (NautilusGtkPlacesView *view,
                                                         gboolean               loading);

G_DEFINE_TYPE (NautilusGtkPlacesView, nautilus_gtk_places_view, GTK_TYPE_BOX)

enum
{
    PROP_0,
    PROP_LOADING,
    LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

static void
server_file_changed_cb (NautilusGtkPlacesView *view)
{
    populate_servers (view);
}

static GBookmarkFile *
server_list_load (NautilusGtkPlacesView *view)
{
    GBookmarkFile *bookmarks;
    GError *error = NULL;
    char *datadir;
    char *filename;

    bookmarks = g_bookmark_file_new ();
    datadir = g_build_filename (g_get_user_config_dir (), "gtk-4.0", NULL);
    filename = g_build_filename (datadir, "servers", NULL);

    g_mkdir_with_parents (datadir, 0700);
    g_bookmark_file_load_from_file (bookmarks, filename, &error);

    if (error)
    {
        if (!g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
        {
            /* only warn if the file exists */
            g_warning ("Unable to open server bookmarks: %s", error->message);
            g_clear_pointer (&bookmarks, g_bookmark_file_free);
        }

        g_clear_error (&error);
    }

    /* Monitor the file in case it's modified outside this code */
    if (!view->server_list_monitor)
    {
        view->server_list_file = g_file_new_for_path (filename);

        if (view->server_list_file)
        {
            view->server_list_monitor = g_file_monitor_file (view->server_list_file,
                                                             G_FILE_MONITOR_NONE,
                                                             NULL,
                                                             &error);

            if (error)
            {
                g_warning ("Cannot monitor server file: %s", error->message);
                g_clear_error (&error);
            }
            else
            {
                g_signal_connect_swapped (view->server_list_monitor,
                                          "changed",
                                          G_CALLBACK (server_file_changed_cb),
                                          view);
            }
        }

        g_clear_object (&view->server_list_file);
    }

    g_free (datadir);
    g_free (filename);

    return bookmarks;
}

static void
server_list_save (GBookmarkFile *bookmarks)
{
    char *filename;

    filename = g_build_filename (g_get_user_config_dir (), "gtk-4.0", "servers", NULL);
    g_bookmark_file_to_file (bookmarks, filename, NULL);
    g_free (filename);
}

static void
server_list_add_server (NautilusGtkPlacesView *view,
                        GFile                 *file)
{
    GBookmarkFile *bookmarks;
    GFileInfo *info;
    GError *error;
    char *title;
    char *uri;
    GDateTime *now;

    error = NULL;
    bookmarks = server_list_load (view);

    if (!bookmarks)
    {
        return;
    }

    uri = g_file_get_uri (file);

    info = g_file_query_info (file,
                              G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              &error);
    title = g_file_info_get_attribute_as_string (info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

    g_bookmark_file_set_title (bookmarks, uri, title);
    now = g_date_time_new_now_utc ();
    g_bookmark_file_set_visited_date_time (bookmarks, uri, now);
    g_date_time_unref (now);
    g_bookmark_file_add_application (bookmarks, uri, NULL, NULL);

    server_list_save (bookmarks);

    g_bookmark_file_free (bookmarks);
    g_clear_object (&info);
    g_free (title);
    g_free (uri);
}

static void
server_list_remove_server (NautilusGtkPlacesView *view,
                           const char            *uri)
{
    GBookmarkFile *bookmarks;

    bookmarks = server_list_load (view);

    if (!bookmarks)
    {
        return;
    }

    g_bookmark_file_remove_item (bookmarks, uri, NULL);
    server_list_save (bookmarks);

    g_bookmark_file_free (bookmarks);
}

static void
nautilus_gtk_places_view_finalize (GObject *object)
{
    NautilusGtkPlacesView *view = (NautilusGtkPlacesView *) object;

    g_clear_object (&view->server_list_file);
    g_clear_object (&view->server_list_monitor);

    G_OBJECT_CLASS (nautilus_gtk_places_view_parent_class)->finalize (object);
}

static void
nautilus_gtk_places_view_dispose (GObject *object)
{
    NautilusGtkPlacesView *view = (NautilusGtkPlacesView *) object;

    if (view->server_list_monitor)
    {
        g_signal_handlers_disconnect_by_func (view->server_list_monitor, server_file_changed_cb, object);
    }

    G_OBJECT_CLASS (nautilus_gtk_places_view_parent_class)->dispose (object);
}

static void
nautilus_gtk_places_view_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
    NautilusGtkPlacesView *self = NAUTILUS_GTK_PLACES_VIEW (object);

    switch (prop_id)
    {
        case PROP_LOADING:
        {
            g_value_set_boolean (value, nautilus_gtk_places_view_get_loading (self));
            break;
        }

        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
    }
}

static void
populate_servers (NautilusGtkPlacesView *view)
{
    GBookmarkFile *server_list;
    char **uris;
    gsize num_uris;

    server_list = server_list_load (view);

    if (!server_list)
    {
        return;
    }

    uris = g_bookmark_file_get_uris (server_list, &num_uris);

    if (!uris)
    {
        g_bookmark_file_free (server_list);
        return;
    }

    /* clear previous items */

    for (gsize i = 0; i < num_uris; i++)
    {
        char *name;
        char *dup_uri;

        name = g_bookmark_file_get_title (server_list, uris[i], NULL);
        dup_uri = g_strdup (uris[i]);

        g_free (name);
    }

    g_strfreev (uris);
    g_bookmark_file_free (server_list);
}

static void
nautilus_gtk_places_view_class_init (NautilusGtkPlacesViewClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = nautilus_gtk_places_view_finalize;
    object_class->dispose = nautilus_gtk_places_view_dispose;
    object_class->get_property = nautilus_gtk_places_view_get_property;

    properties[PROP_LOADING] =
        g_param_spec_boolean ("loading",
                              "Loading",
                              "Whether the view is loading locations",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);

    g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
nautilus_gtk_places_view_init (NautilusGtkPlacesView *self)
{
}

GtkWidget *
nautilus_gtk_places_view_new (void)
{
    return g_object_new (NAUTILUS_TYPE_GTK_PLACES_VIEW, NULL);
}

gboolean
nautilus_gtk_places_view_get_loading (NautilusGtkPlacesView *view)
{
    g_return_val_if_fail (NAUTILUS_IS_GTK_PLACES_VIEW (view), FALSE);

    return view->loading;
}

static void
nautilus_gtk_places_view_set_loading (NautilusGtkPlacesView *view,
                                      gboolean               loading)
{
    g_return_if_fail (NAUTILUS_IS_GTK_PLACES_VIEW (view));

    if (view->loading != loading)
    {
        view->loading = loading;
        g_object_notify_by_pspec (G_OBJECT (view), properties [PROP_LOADING]);
    }
}
