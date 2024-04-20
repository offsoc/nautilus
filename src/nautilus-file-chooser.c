/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Original Author: António Fernandes <antoniof@gnome.org>
 */

#define G_LOG_DOMAIN "nautilus-file-chooser"

#include "nautilus-file-chooser.h"

#include <config.h>
#include <gtk/gtk.h>

#include "gtk/nautilusgtkplacessidebarprivate.h"

#include "nautilus-shortcut-manager.h"
#include "nautilus-toolbar.h"
#include "nautilus-window-slot.h"

struct _NautilusFileChooser
{
    AdwWindow parent_instance;

    NautilusWindowSlot *slot;
    GtkWidget *places_sidebar;
    GtkWidget *toolbar;
    GtkWidget *accept_button;
};

G_DEFINE_FINAL_TYPE (NautilusFileChooser, nautilus_file_chooser, ADW_TYPE_WINDOW)

enum
{
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
update_cursor (NautilusFileChooser *self)
{
    if (self->slot != NULL &&
        nautilus_window_slot_get_allow_stop (self->slot))
    {
        gtk_widget_set_cursor_from_name (GTK_WIDGET (self), "progress");
    }
    else
    {
        gtk_widget_set_cursor (GTK_WIDGET (self), NULL);
    }
}

static void
on_slot_location_changed (NautilusFileChooser *self)
{
    GFile *location = nautilus_window_slot_get_location (self->slot);

    nautilus_gtk_places_sidebar_set_location (NAUTILUS_GTK_PLACES_SIDEBAR (self->places_sidebar),
                                              location);
}

static void
on_slot_search_global_changed (NautilusFileChooser *self)
{
    NautilusGtkPlacesSidebar *sidebar = NAUTILUS_GTK_PLACES_SIDEBAR (self->places_sidebar);

    if (nautilus_window_slot_get_search_global (self->slot))
    {
        nautilus_gtk_places_sidebar_set_location (sidebar, NULL);
    }
    else
    {
        nautilus_gtk_places_sidebar_set_location (sidebar, nautilus_window_slot_get_location (self->slot));
    }
}

static void
nautilus_file_chooser_finalize (GObject *object)
{
    NautilusFileChooser *self = (NautilusFileChooser *) object;

    G_OBJECT_CLASS (nautilus_file_chooser_parent_class)->finalize (object);
}

static void
nautilus_file_chooser_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
    NautilusFileChooser *self = NAUTILUS_FILE_CHOOSER (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
nautilus_file_chooser_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
    NautilusFileChooser *self = NAUTILUS_FILE_CHOOSER (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
nautilus_file_chooser_class_init (NautilusFileChooserClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->finalize = nautilus_file_chooser_finalize;
    object_class->get_property = nautilus_file_chooser_get_property;
    object_class->set_property = nautilus_file_chooser_set_property;

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gnome/nautilus/ui/nautilus-file-chooser.ui");
    gtk_widget_class_bind_template_child (widget_class, NautilusFileChooser, slot);
    gtk_widget_class_bind_template_child (widget_class, NautilusFileChooser, places_sidebar);
    gtk_widget_class_bind_template_child (widget_class, NautilusFileChooser, toolbar);
    gtk_widget_class_bind_template_child (widget_class, NautilusFileChooser, accept_button);
}

static void
nautilus_file_chooser_init (NautilusFileChooser *self)
{
    g_type_ensure (NAUTILUS_TYPE_TOOLBAR);
    g_type_ensure (NAUTILUS_TYPE_GTK_PLACES_SIDEBAR);
    g_type_ensure (NAUTILUS_TYPE_SHORTCUT_MANAGER);
    g_type_ensure (NAUTILUS_TYPE_WINDOW_SLOT);
    gtk_widget_init_template (GTK_WIDGET (self));

    /* Setup slot */
    nautilus_window_slot_set_active (self->slot, TRUE);
    g_signal_connect_swapped (self->slot, "notify::allow-stop",
                              G_CALLBACK (update_cursor), self);
    g_signal_connect_swapped (self->slot, "notify::location",
                              G_CALLBACK (on_slot_location_changed), self);
    g_signal_connect_swapped (self->slot, "notify::search-global",
                              G_CALLBACK (on_slot_search_global_changed), self);

    /* Setup sidebar */
    nautilus_gtk_places_sidebar_set_open_flags (NAUTILUS_GTK_PLACES_SIDEBAR (self->places_sidebar),
                                                NAUTILUS_GTK_PLACES_OPEN_NORMAL);

#if 0
    g_signal_connect_swapped (window->places_sidebar, "open-location",
                              G_CALLBACK (open_location_cb), window);
    g_signal_connect (window->places_sidebar, "show-error-message",
                      G_CALLBACK (places_sidebar_show_error_message_cb), window);
#endif
}

NautilusFileChooser *
nautilus_file_chooser_new (void)
{
    return g_object_new (NAUTILUS_TYPE_FILE_CHOOSER, NULL);
}

void
nautilus_file_chooser_set_starting_location (NautilusFileChooser *self,
                                             GFile               *starting_location)
{
    nautilus_window_slot_open_location_full (self->slot, starting_location, 0, NULL);

    nautilus_window_slot_set_active (self->slot, TRUE);
}

void
nautilus_file_chooser_set_accept_label (NautilusFileChooser *self,
                                        const char          *accept_label)
{
    gtk_button_set_label (GTK_BUTTON (self->accept_button), accept_label);
}
