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

#ifndef INTROSPECTION_H
#define INTROSPECTION_H

#include <list>
#include <string>
#include "GtkNode.h"
#include "IntrospectionService.h"

extern AutopilotIntrospection* autopilot_introspection;

void bus_acquired (GObject *object,
                          GAsyncResult * res,
                          gpointer user_data);
gboolean handle_get_state (AutopilotIntrospection* introspection_service,
                           GDBusMethodInvocation* invocation,
                           const gchar * arg,
                           gpointer user_data);
GVariant* Introspect(std::string const& query_string);
std::list<GtkNode::Ptr> GetNodesThatMatchQuery(std::string const& query_string);

#endif // INTROSPECTION_H
