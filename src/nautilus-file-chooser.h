/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Original Author: António Fernandes <antoniof@gnome.org>
 */

#pragma once

#include <gio/gio.h>
#include <adwaita.h>

G_BEGIN_DECLS

#define NAUTILUS_TYPE_FILE_CHOOSER (nautilus_file_chooser_get_type())

G_DECLARE_FINAL_TYPE (NautilusFileChooser, nautilus_file_chooser, NAUTILUS, FILE_CHOOSER, AdwWindow)

NautilusFileChooser *
nautilus_file_chooser_new                   (void);

void
nautilus_file_chooser_set_starting_location (NautilusFileChooser *self,
                                             GFile               *starting_location);

void
nautilus_file_chooser_set_accept_label (NautilusFileChooser *self,
                                        const char          *accept_label);

G_END_DECLS
