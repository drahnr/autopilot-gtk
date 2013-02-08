// -*- Mode: C++; indent-tabs-mode: nil; tab-width: 2 -*-
/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Allan LeSage <allan.lesage@canonical.com>
 */

#include <glib.h>
#include <gdk/gdk.h>

#include "main.h"
#include "Introspection.h"
#include "IntrospectionService.h"

AutopilotIntrospection* autopilot_introspection = NULL;

void LogToFile (const gchar* log_domain,
                GLogLevelFlags log_level,
                const gchar* message,
                gpointer user_data)
{
  FILE* logfile = fopen ("autopilot-gtk.log", "a");
  if (logfile == NULL) {
    //g_warning("rerouting logger to console");
    return;
  }
  fprintf (logfile, "%s\n", message);
  fclose (logfile);
}

int gtk_module_init(gint argc, char *argv[]) {
  //g_log_set_handler (NULL, G_LOG_LEVEL_MASK, LogToFile, NULL);
  //g_log_set_handler ("GLib", G_LOG_LEVEL_MASK, LogToFile, NULL);
  autopilot_introspection = autopilot_introspection_skeleton_new ();
  g_bus_get (G_BUS_TYPE_SESSION, NULL, bus_acquired, NULL);
  return 0;
}

int display_init_func(GdkDisplay* display) {
  // FIXME: module fails to load if this is not defined, but is it necessary?
  return 0;
}
