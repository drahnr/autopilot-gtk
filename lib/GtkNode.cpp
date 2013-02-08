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

#include <string>
#include <gdk/gdk.h>

#include <string>
#include "GtkNode.h"
#include "Variant.h"

const std::string GtkNode::AP_ID_NAME = "id";

GtkNode::GtkNode(GObject* obj)
  : object_(obj) {
}

GVariant* GtkNode::Introspect() const
{
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

  // add a GVariant of all our properties
  guint length;
  GParamSpec** properties = g_object_class_list_properties(G_OBJECT_GET_CLASS(object_), &length);
  variant::BuilderWrapper builder_wrapper(&builder);
  for (uint i = 0; i < length; ++i) {
    GParamSpec* param_spec = properties[i];
    // see Launchpad bug #1108155: GtkTreePath mis-casts while copying, actuates here in "root" property
    if (std::string(g_type_name(param_spec->value_type)) != "GtkTreePath") {
      // some properties are only writeable; some toxic nodes change their names (?)
      if (param_spec->flags & G_PARAM_READABLE) {
        GValue value = G_VALUE_INIT;
        g_value_init(&value, param_spec->value_type);
        g_object_get_property(object_, g_param_spec_get_name(param_spec), &value);
        builder_wrapper.add(param_spec->name, &value);
        g_value_unset(&value); //Free the memory accquired by the value object. Absence of this was causig the applications to crash.
      }
    } else {
      //g_debug("skipped %s of type GtkTreePath", g_param_spec_get_name(param_spec));
    }
  }
  g_free(properties);

  // add our unique autopilot-id
  builder_wrapper.add(AP_ID_NAME.c_str(), GetObjectId());

  // add the names of our children
  builder_wrapper.add("Children", GetChildNodeNames());

  // add the GlobalRect property: "I am a GtkWidget" edition
  if (GTK_IS_WIDGET(object_)) {
    // FIXME: we'd like to remove this duplication (to GetGlobalRect)
    GtkWidget *widget = GTK_WIDGET (object_);
    GdkWindow *gdk_window = gtk_widget_get_window(widget);
    if (GDK_IS_WINDOW(gdk_window)) {
      GdkRectangle rect;
      GetGlobalRect(&rect);
      //g_debug("Rect coords %d, %d, %d, %d", rect.x, rect.y, rect.width, rect.height);
      GVariant *rect_gvariant = ComposeRectVariant(rect.x, rect.y, rect.width, rect.height);
      builder_wrapper.add("globalRect", rect_gvariant);
    }
  }
  return g_variant_builder_end(&builder);
}

GVariant* GtkNode::ComposeRectVariant(gint x, gint y, gint height, gint width) const
{
  //g_debug("composing a rect variant");
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
  g_variant_builder_add(&builder, "i", x);
  g_variant_builder_add(&builder, "i", y);
  g_variant_builder_add(&builder, "i", height);
  g_variant_builder_add(&builder, "i", width);
  return g_variant_builder_end(&builder);
}

intptr_t GtkNode::GetObjectId() const {
  return reinterpret_cast<intptr_t>(object_);
}

void GtkNode::GetGlobalRect(GdkRectangle* rect) const
{
  GtkWidget *widget = GTK_WIDGET(object_);
  GdkWindow *gdk_window = gtk_widget_get_window(widget);
  GtkAllocation allocation;
  gint x, y;

  gtk_widget_get_allocation (widget, &allocation);
  gdk_window_get_root_coords(gdk_window, allocation.x, allocation.y, &x, &y);
  // if we wished to get the root root coords we might do this
  //gdk_window_get_root_coords(gdk_window, 0, 0, &x, &y);
  //g_debug ("root coords for widget %p(%s): %d %d (size %d %d)\n", widget, G_OBJECT_TYPE_NAME(widget), x, y, allocation.width, allocation.height);

  // FIXME: suck this up
  rect->x = x;
  rect->y = y;
  rect->width = allocation.width;
  rect->height = allocation.height;
  return;
}

std::string GtkNode::GetName() const {
  // autopilot uses the name of the GObject type
  return G_OBJECT_TYPE_NAME(object_);
}

bool GtkNode::MatchProperty(const std::string& name,
                            const std::string& value) const {
  //g_debug("attempting to match a node's property");
  // yes we're transforming to strings and comparing there
  if (name == "id") {
    // FIXME: b/c intptr_t makes casting tricky, or just cast to long?
    return value == std::to_string(GetObjectId());
  } else {
    GObjectClass* klass = G_OBJECT_GET_CLASS(object_);
    GParamSpec* pspec = g_object_class_find_property(klass, name.c_str());
    // if we found the property and it can be expressed as a string
    if (pspec && g_value_type_transformable(G_PARAM_SPEC_TYPE(pspec),
                                            G_TYPE_STRING)) {
      GValue dest_value = G_VALUE_INIT;
      g_value_init(&dest_value, G_TYPE_STRING);
      g_object_get_property(object_, name.c_str(), &dest_value);
      std::string dest_string = g_value_get_string(&dest_value);
      g_value_unset(&dest_value);
      return dest_string == value;
    }
    return false;
  }
}

xpathselect::NodeList GtkNode::Children() const {
  //g_debug("getting the children of a node");
  xpathselect::NodeList children;
  if (GTK_IS_CONTAINER(object_)) {
    GList* gtk_children = gtk_container_get_children(GTK_CONTAINER(object_));
    for (GList* elem = gtk_children; elem; elem = elem->next) {
      children.push_back(std::make_shared<GtkNode>(G_OBJECT(elem->data)));
    }
    g_list_free(gtk_children);
  }

  return children;
}

GVariant* GtkNode::GetChildNodeNames() const {
  //g_debug("getting the names of a node's children");
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE_STRING_ARRAY);
  for (xpathselect::Node::Ptr child : Children()) {
    g_variant_builder_add(&builder, "s", child->GetName().c_str());
  }
  return g_variant_builder_end(&builder);
}
