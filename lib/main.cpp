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
#include <string.h>
#include <fstream>
#include <iostream>

#include "main.h"
#include "Introspection.h"
#include "IntrospectionService.h"

namespace
{
  std::string filename;
  GLogLevelFlags levels_to_log;

  std::ostream& get_log_stream()
  {
    if (!filename.empty())
    {
      static std::ofstream fstream(filename);
      return fstream;
    }
    else
    {
      return std::cout;
    }
  }

  std::string get_level_name(GLogLevelFlags lvl)
  {
    switch(lvl)
    {
      case G_LOG_LEVEL_DEBUG:
      return "DEBUG";
      case G_LOG_LEVEL_INFO:
      return "INFO";
      case G_LOG_LEVEL_MESSAGE:
      return "MESSAGE";
      case G_LOG_LEVEL_WARNING:
      return "WARNING";
      case G_LOG_LEVEL_CRITICAL:
      return "CRITICAL";
      case G_LOG_LEVEL_ERROR:
      return "ERROR";
      default:
      return "UNKNOWN";
    }
  }
}
AutopilotIntrospection* autopilot_introspection = NULL;

void LogHandler (const gchar* log_domain,
                GLogLevelFlags log_level,
                const gchar* message,
                gpointer user_data)
{
  if (log_level & levels_to_log)
  {
    std::string domain = log_domain ? log_domain : "default";
    get_log_stream() << "[" << domain << "] " << get_level_name(log_level) << ": " << message << std::endl;
  }
}

void initialise_logging()
{
  if (getenv("AP_GTK_LOG_VERBOSE"))
  {
    levels_to_log = (GLogLevelFlags) (
      G_LOG_LEVEL_MASK |
      G_LOG_FLAG_FATAL |
      G_LOG_FLAG_RECURSION);
  }
  else
  {
    levels_to_log = (GLogLevelFlags)(
      G_LOG_LEVEL_WARNING |
      G_LOG_LEVEL_ERROR |
      G_LOG_LEVEL_CRITICAL |
      G_LOG_FLAG_FATAL |
      G_LOG_FLAG_RECURSION);
  }
  char* fname = getenv("AP_GTK_LOG_FILE");
  if (fname && strcmp(fname, "") != 0)
  {
    filename = fname;
  }

  g_log_set_default_handler(LogHandler, NULL);
}

int gtk_module_init(gint argc, char *argv[]) {
  initialise_logging();
  autopilot_introspection = autopilot_introspection_skeleton_new ();
  g_bus_get (G_BUS_TYPE_SESSION, NULL, bus_acquired, NULL);
  // always log this:
  std::cout << "Autopilot GTK interface loaded. Wire protocol version is " << WIRE_PROTO_VERSION << "." << std::endl;
  return 0;
}

int display_init_func(GdkDisplay* display) {
  // FIXME: module fails to load if this is not defined, but is it necessary?
  return 0;
}
