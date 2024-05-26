/*
 * Copyright (C) 2024 António Fernandes <antoniof@gnome.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define NAUTILUS_TYPE_RECENT_SERVERS (nautilus_recent_servers_get_type ())

G_DECLARE_FINAL_TYPE (NautilusRecentServers, nautilus_recent_servers, NAUTILUS, RECENT_SERVERS, GObject);

NautilusRecentServers* nautilus_recent_servers_new (void);

gboolean                nautilus_recent_servers_get_loading    (NautilusRecentServers *self);

G_END_DECLS
