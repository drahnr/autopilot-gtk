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

#include "GtkRootNode.h"

#include <glib.h>
#include <glib-object.h>
#include <list>
#include <string>

#include "Variant.h"

GtkRootNode::GtkRootNode()
  : GtkNode(NULL) {
}

GVariant* GtkRootNode::Introspect() const {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
  variant::BuilderWrapper builder_wrapper(&builder);
  // add our unique autopilot-id
  builder_wrapper.add(AP_ID_NAME.c_str(), GetObjectId());
  // add the names of our children
  builder_wrapper.add("Children", GetChildNodeNames());
  return g_variant_builder_end(&builder);
}

intptr_t GtkRootNode::GetObjectId() const {
  // FIXME: we could be consistent and return the memory address
  return 1;
}

std::string GtkRootNode::GetName() const {
  return "Root";
}

bool GtkRootNode::MatchProperty(const std::string& name,
                                const std::string& value) const {
  //g_debug("matching a property for the root node");
  if (name == "id")
    // yes we just compare as a string; use of intptr_t makes casting tricky
    return value == std::to_string(GetObjectId());
  return false;
}

xpathselect::NodeList GtkRootNode::Children() const {
  //g_debug("getting the children of a node");
  xpathselect::NodeList children;

  // add all the toplevel nodes as children to the root node
  GList* toplevels_list = gtk_window_list_toplevels();
  GList* elem;
  for (elem = toplevels_list; elem; elem = elem->next) {
    GObject *node = reinterpret_cast<GObject*>(elem->data);
    children.push_back(std::make_shared<GtkNode>(node));
  }
  g_list_free(toplevels_list);
  return children;
}

GVariant* GtkRootNode::GetChildNodeNames() const {
  //g_debug("getting the names of a node's children");
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE_STRING_ARRAY);
  for (xpathselect::Node::Ptr child : Children()) {
    g_variant_builder_add(&builder, "s", child->GetName().c_str());
  }
  return g_variant_builder_end(&builder);
}
