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
#include <sstream>

#include "GtkNode.h"
#include "Variant.h"

const std::string GtkNode::AP_ID_NAME = "id";

GtkNode::GtkNode(GObject* obj, std::string const& parent_path)
  : object_(obj) {
  full_path_ = parent_path + "/" + GetName();
  if (object_ != NULL) g_object_ref(object_);
}

GtkNode::~GtkNode()
{
  g_clear_object(&object_);
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
    // ATK's accessible-table-* properties generate "invalid property id" warnings
    std::string prefix("accessible-table-");
    if (std::string(g_param_spec_get_name(param_spec)).compare(0, prefix.length(), prefix) == 0)
      continue;
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
  } else if (ATK_IS_COMPONENT(object_)) {
    AddAtkComponentProperties(builder_wrapper, ATK_COMPONENT(object_));
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

void GtkNode::AddAtkComponentProperties(variant::BuilderWrapper &builder_wrapper,
                                        AtkComponent *atk_component) const
{
  AtkStateSet *states = atk_object_ref_state_set(ATK_OBJECT(atk_component));

  /* Expose a few states which might be especially interesting for autopilot */
  bool visible = atk_state_set_contains_state(states, ATK_STATE_VISIBLE);
  builder_wrapper.add("visible", visible);
  if (visible) {
    gint x, y, width, height;
    atk_component_get_extents(atk_component, &x, &y, &width, &height,
                              ATK_XY_SCREEN);
    GVariant *rect_gvariant = ComposeRectVariant(x, y, width, height);
    builder_wrapper.add("globalRect", rect_gvariant);
  }

  builder_wrapper.add("active",
                      bool(atk_state_set_contains_state(states, ATK_STATE_ACTIVE)));
  builder_wrapper.add("checked",
                      bool(atk_state_set_contains_state(states, ATK_STATE_CHECKED)));
  builder_wrapper.add("editable",
                      bool(atk_state_set_contains_state(states, ATK_STATE_EDITABLE)));
  builder_wrapper.add("enabled",
                      bool(atk_state_set_contains_state(states, ATK_STATE_ENABLED)));
  builder_wrapper.add("focused",
                      bool(atk_state_set_contains_state(states, ATK_STATE_FOCUSED)));
  builder_wrapper.add("pressed",
                      bool(atk_state_set_contains_state(states, ATK_STATE_PRESSED)));
  builder_wrapper.add("selected",
                      bool(atk_state_set_contains_state(states, ATK_STATE_SELECTED)));
  builder_wrapper.add("sensitive",
                      bool(atk_state_set_contains_state(states, ATK_STATE_SENSITIVE)));
  builder_wrapper.add("showing",
                      bool(atk_state_set_contains_state(states, ATK_STATE_SHOWING)));
  g_object_unref(G_OBJECT(states));
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
  if (!object_) {
    return std::string();
  }
  return G_OBJECT_TYPE_NAME(object_);
}

std::string GtkNode::GetPath() const {
  return full_path_;
}

bool GtkNode::MatchProperty(const std::string& name,
                            const std::string& value) const {

  // g_debug("attempting to match a node's property");

  if (name == "id") {
    return value == std::to_string(GetObjectId());

  } else {
    GObjectClass* klass = G_OBJECT_GET_CLASS(object_);
    GParamSpec* pspec = g_object_class_find_property(klass, name.c_str());
    if (pspec == NULL)
      return false;

    g_debug("Matching a property of type (%s).", g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspec)));

    if (pspec && G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_INT) {
      GValue dest_value = G_VALUE_INIT;
      g_value_init(&dest_value, G_TYPE_INT);
      g_object_get_property(object_, name.c_str(), &dest_value);

      const gint cint = g_value_get_int(&dest_value);
      g_value_unset(&dest_value);
      std::stringstream out;
      out << cint;
      std::string dest_string(out.str());
      return dest_string == value;

    } else if (pspec && G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_DOUBLE) {
      GValue dest_value = G_VALUE_INIT;
      g_value_init(&dest_value, G_TYPE_DOUBLE);
      g_object_get_property(object_, name.c_str(), &dest_value);

      const gdouble cdbl = g_value_get_double(&dest_value);
      g_value_unset(&dest_value);
      std::stringstream out;
      out << cdbl;
      std::string dest_string(out.str());
      return dest_string == value;

    } else if (pspec && G_PARAM_SPEC_VALUE_TYPE(pspec) == G_TYPE_STRING) {
      gchar *strval = NULL;
      g_object_get(object_, name.c_str(), &strval, NULL);
      if (strval == NULL)
        return false;

      std::string dest_string(strval);
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
      children.push_back(std::make_shared<GtkNode>(G_OBJECT(elem->data), GetPath()));
    }
    g_list_free(gtk_children);
  } else if (ATK_IS_OBJECT(object_)) {
    AtkObject *atk_object = ATK_OBJECT(object_);
    int n_children = atk_object_get_n_accessible_children(atk_object);
    for (int i = 0; i < n_children; i++) {
      AtkObject *child = atk_object_ref_accessible_child(atk_object, i);
      children.push_back(std::make_shared<GtkNode>(G_OBJECT(child), GetPath()));
    }
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
