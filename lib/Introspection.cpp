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

#include "Introspection.h"

#include <glib.h>
#include <xpathselect/node.h>
#include <xpathselect/xpathselect.h>

#include <string>
#include <list>
#include "GtkNode.h"
#include "GtkRootNode.h"

std::string AUTOPILOT_INTROSPECTION_OBJECT_PATH = "/com/canonical/Autopilot/Introspection";

void bus_acquired (GObject *object,
                   GAsyncResult * res,
                   gpointer user_data)
{
  //g_debug("bus acquired");
  GDBusConnection *bus;
  GError *error = NULL;
  bus = g_bus_get_finish (res, &error);
  if (!bus) {
    //g_warning ("unable to connect to the session bus: %s", error->message);
    g_error_free (error);
    return;
  }
  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (autopilot_introspection),
                                    bus,
                                    AUTOPILOT_INTROSPECTION_OBJECT_PATH.c_str(),
                                    &error);
  if (error) {
    //g_warning ("unable to export autopilot introspection service on dbus: %s", error->message);
    g_error_free (error);
    return;
  }
  g_signal_connect (autopilot_introspection,
                    "handle-get-state",
                    G_CALLBACK(handle_get_state),
                    NULL);
  g_signal_connect (autopilot_introspection,
                    "handle-get-version",
                    G_CALLBACK(handle_get_version),
                    NULL);
  g_object_unref (bus);
}

gboolean handle_get_state (AutopilotIntrospection* introspection_service,
                           GDBusMethodInvocation* invocation,
                           const gchar * arg,
                           gpointer user_data)
{
  //g_debug("handling get-state method call");
  GVariant* state;
  state = Introspect(arg);
  autopilot_introspection_complete_get_state(introspection_service,
                                             invocation,
                                             state);
  return TRUE;
}

gboolean handle_get_version (AutopilotIntrospection *introspection_service,
                             GDBusMethodInvocation *invocation)
{
    autopilot_introspection_complete_get_version(introspection_service,
                                                 invocation,
                                                 WIRE_PROTO_VERSION.c_str());
    return TRUE;
}

GVariant* Introspect(std::string const& query_string) {
  //g_debug("introspecting our current GTK+ context");

  GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a(sv)"));
  std::list<GtkNode::Ptr> node_list = GetNodesThatMatchQuery(query_string);

  for (auto node: node_list) {
    std::string object_path = node->GetPath();
    g_variant_builder_add(builder, "(sv)", object_path.c_str(), node->Introspect());
    //g_debug("dumped object '%s'", object_path.c_str());
  }

  GVariant* state = g_variant_new("a(sv)", builder);
  g_variant_builder_unref(builder);
  return state;
}

std::list<GtkNode::Ptr> GetNodesThatMatchQuery(std::string const& query_string) {
  //g_debug("getting nodes that match query");

  std::shared_ptr<GtkRootNode> root = std::make_shared<GtkRootNode>();

  //g_debug("selecting nodes");
  std::list<GtkNode::Ptr> node_list;
  xpathselect::NodeVector selected_nodes_list;
  selected_nodes_list = xpathselect::SelectNodes(root, query_string);
  //g_debug("finished selecting nodes");
  for (auto node : selected_nodes_list) {
    // node may be our root node wrapper *or* an ordinary GObject wrapper
    auto object_ptr = std::static_pointer_cast<const GtkNode>(node);
    if (object_ptr)
      node_list.push_back(object_ptr);
  }

  return node_list;
}
