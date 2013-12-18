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
static guint32 cur_obj_id = 2; // start at 2 since 1 is reserved for the root node

GtkNode::GtkNode(GObject* obj, GtkNode::Ptr const& parent)
  : object_(obj)
  , parent_(parent)
{
  std::string parent_path = parent ? parent->GetPath() : "";
  full_path_ = parent_path + "/" + GetName();
  if (object_ != NULL)
  {
    g_object_ref(object_);
    GQuark OBJ_ID = g_quark_from_static_string("AUTOPILOT_OBJECT_ID");
    gpointer val = g_object_get_qdata (object_, OBJ_ID);
    if (val == NULL)
    {
      g_object_set_qdata (object_, OBJ_ID, reinterpret_cast<gpointer>(cur_obj_id++));
    }
  }
}

GtkNode::GtkNode(GObject* obj)
  : object_(obj)
{
  full_path_ = "/" + GetName();
  if (object_ != NULL)
  {
    g_object_ref(object_);
    GQuark OBJ_ID = g_quark_from_static_string("AUTOPILOT_OBJECT_ID");
    gpointer val = g_object_get_qdata (object_, OBJ_ID);
    if (val == NULL)
    {
      g_object_set_qdata (object_, OBJ_ID, reinterpret_cast<gpointer>(cur_obj_id++));
    }
  }
}

GtkNode::~GtkNode()
{
  g_clear_object(&object_);
}

// we cannot represent GEnums, GFlags, etc. through D-BUS and autopilot's API,
// so convert them to strings, ints, and other primitive types
static void convert_value (GParamSpec *pspec, GValue *value)
{
  if (G_VALUE_HOLDS_ENUM(value)) {
    GEnumValue *ev = g_enum_get_value(G_PARAM_SPEC_ENUM(pspec)->enum_class,
                                      g_value_get_enum(value));
    if (ev != NULL) {
      //g_debug("attribute %s of type %s holds enum %s", g_param_spec_get_name(pspec),
      //        g_type_name(pspec->value_type), ev->value_name);
      g_value_unset(value);
      *value = G_VALUE_INIT;
      g_value_init(value, G_TYPE_STRING);
      g_value_set_string(value, ev->value_name);
    }
  }

  // representing flags as strings is too unwieldy; let's just represent them
  // as integer
  if (G_VALUE_HOLDS_FLAGS(value)) {
      guint flags = g_value_get_flags(value);
      //g_debug("attribute %s of type %s holds flags %x", g_param_spec_get_name(pspec),
      //        g_type_name(pspec->value_type), flags);
      g_value_unset(value);
      *value = G_VALUE_INIT;
      g_value_init(value, G_TYPE_UINT);
      g_value_set_uint(value, flags);
  }

  if (pspec->value_type == GTK_TYPE_TEXT_BUFFER) {
    GtkTextBuffer *buf = GTK_TEXT_BUFFER(g_value_get_object(value));
    if (buf != NULL) {
      //g_debug("attribute %s of type %s holds GtkTextBuffer", g_param_spec_get_name(pspec),
      //        g_type_name(pspec->value_type));
      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(buf, &start);
      gtk_text_buffer_get_end_iter(buf, &end);
      gchar* text = gtk_text_iter_get_text(&start, &end);

      g_value_unset(value);
      *value = G_VALUE_INIT;
      g_value_init(value, G_TYPE_STRING);
      g_value_set_string(value, (text != NULL) ? text : "");

      g_free(text);
    }
  }
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
    if (g_str_has_prefix(g_param_spec_get_name(param_spec), "accessible-table-"))
      continue;
    // see Launchpad bug #1108155: GtkTreePath mis-casts while copying, actuates here in "root" property
    if (g_strcmp0(g_type_name(param_spec->value_type), "GtkTreePath") != 0) {
      // some properties are only writeable; some toxic nodes change their names (?)
      if (param_spec->flags & G_PARAM_READABLE) {
        GValue value = G_VALUE_INIT;
        g_value_init(&value, param_spec->value_type);
        g_object_get_property(object_, g_param_spec_get_name(param_spec), &value);
        convert_value(param_spec, &value);
        builder_wrapper.add_gvalue(param_spec->name, &value);
        g_value_unset(&value); //Free the memory acquired by the value object. Absence of this was causig the applications to crash.
      }
    } else {
      //g_debug("skipped %s of type GtkTreePath", g_param_spec_get_name(param_spec));
    }
  }
  g_free(properties);

  // add our unique autopilot-id
  builder_wrapper.add(AP_ID_NAME.c_str(), GetId());

  // add the names of our children
  builder_wrapper.add("Children", GetChildNodeNames());

  // add the GtkBuilder name
  if (GTK_IS_BUILDABLE (object_))
    builder_wrapper.add("BuilderName", gtk_buildable_get_name(GTK_BUILDABLE (object_)));

  // add the GlobalRect property: "I am a GtkWidget" edition
  if (GTK_IS_WIDGET(object_)) {
    // FIXME: we'd like to remove this duplication (to GetGlobalRect)
    GtkWidget *widget = GTK_WIDGET (object_);
    GdkWindow *gdk_window = gtk_widget_get_window(widget);
    if (GDK_IS_WINDOW(gdk_window)) {
      GdkRectangle rect;
      GetGlobalRect(&rect);
      builder_wrapper.add("globalRect", rect);
    }
  } else if (ATK_IS_COMPONENT(object_)) {
    AddAtkComponentProperties(builder_wrapper, ATK_COMPONENT(object_));
  }
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
    x = y = width = height = -1;
    atk_component_get_extents(atk_component, &x, &y, &width, &height,
                              ATK_XY_SCREEN);
    GdkRectangle r;
    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;
    builder_wrapper.add("globalRect", r);
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

int32_t GtkNode::GetId() const
{
  GQuark OBJ_ID = g_quark_from_static_string("AUTOPILOT_OBJECT_ID");
  gpointer val = g_object_get_qdata (object_, OBJ_ID);
  // this uglyness is required in order to stop the compiler complaining about the fact
  // that we're casting a 64 bit type (gpointer) down to a 32 bit type (gint32) and may
  // be truncating the value. It's safe to do, however, since we control what values are
  // set in this quark, and they were initial gint32 values anyway.
  guint32 id = static_cast<gint32>(reinterpret_cast<intptr_t>(val));
  return id;
}

xpathselect::Node::Ptr GtkNode::GetParent() const
{
  return parent_;
}
bool GtkNode::MatchStringProperty(const std::string& name,
                                  const std::string& value) const {
  if (name == "BuilderName" && GTK_IS_BUILDABLE(object_)) {
      const gchar* builder_name = gtk_buildable_get_name(GTK_BUILDABLE (object_));
      return builder_name != NULL && value.compare(builder_name) == 0;
  }

  GObjectClass* klass = G_OBJECT_GET_CLASS(object_);
  GParamSpec* pspec = g_object_class_find_property(klass, name.c_str());
  if (pspec == NULL)
    return false;

  // read the property into a GValue
  g_debug("Matching property %s of type (%s).", g_param_spec_get_name(pspec),
          g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspec)));

  GValue dest_value = G_VALUE_INIT;
  g_value_init(&dest_value, G_PARAM_SPEC_VALUE_TYPE(pspec));
  g_object_get_property(object_, name.c_str(), &dest_value);
  convert_value(pspec, &dest_value);

  if (G_VALUE_TYPE(&dest_value) == G_TYPE_STRING) {
      const gchar *str = g_value_get_string(&dest_value);
      int result = g_strcmp0 (str, value.c_str());
      g_value_unset(&dest_value);
      return result == 0;
  }
  else {
      g_debug("Property %s exists, but is not a string (is %s).",
              g_param_spec_get_name(pspec),
              g_type_name(G_VALUE_TYPE(&dest_value))
              );
      g_value_unset(&dest_value);
      return false;
    }

}

bool GtkNode::MatchIntegerProperty(const std::string& name,
                                  int32_t value) const {
  if (name == "id")
    return value == GetId();

  GObjectClass* klass = G_OBJECT_GET_CLASS(object_);
  GParamSpec* pspec = g_object_class_find_property(klass, name.c_str());
  if (pspec == NULL)
    return false;

  // read the property into a GValue
  g_debug("Matching property %s of type (%s).", g_param_spec_get_name(pspec),
          g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspec)));

  GValue dest_value = G_VALUE_INIT;
  g_value_init(&dest_value, G_PARAM_SPEC_VALUE_TYPE(pspec));
  g_object_get_property(object_, name.c_str(), &dest_value);
  convert_value(pspec, &dest_value);

  if (G_VALUE_TYPE(&dest_value) == G_TYPE_INT) {
    int v = g_value_get_int(&dest_value);
    g_value_unset(&dest_value);
    return value == v;
  }
  else if (G_VALUE_TYPE(&dest_value) == G_TYPE_UINT) {
    int v = g_value_get_uint(&dest_value);
    g_value_unset(&dest_value);
    return value == v;
  }
  else {
      g_debug("Property %s exists, but is not an integer (is %s).",
              g_param_spec_get_name(pspec),
              g_type_name(G_VALUE_TYPE(&dest_value))
              );
      g_value_unset(&dest_value);
      return false;
    }
}

bool GtkNode::MatchBooleanProperty(const std::string& name,
                                   bool value) const {
  GObjectClass* klass = G_OBJECT_GET_CLASS(object_);
  GParamSpec* pspec = g_object_class_find_property(klass, name.c_str());
  if (pspec == NULL)
    return false;

  // read the property into a GValue
  g_debug("Matching property %s of type (%s).", g_param_spec_get_name(pspec),
          g_type_name(G_PARAM_SPEC_VALUE_TYPE(pspec)));

  GValue dest_value = G_VALUE_INIT;
  g_value_init(&dest_value, G_PARAM_SPEC_VALUE_TYPE(pspec));
  g_object_get_property(object_, name.c_str(), &dest_value);
  convert_value(pspec, &dest_value);

  if (G_VALUE_TYPE(&dest_value) == G_TYPE_BOOLEAN) {
    bool v = g_value_get_boolean(&dest_value);
    g_value_unset(&dest_value);
    return value == v;
  }
  else {
      g_debug("Property %s exists, but is not a boolean (is %s).",
              g_param_spec_get_name(pspec),
              g_type_name(G_VALUE_TYPE(&dest_value))
              );
      g_value_unset(&dest_value);
      return false;
    }
}

xpathselect::NodeVector GtkNode::Children() const {
  //g_debug("getting the children of a node");
  xpathselect::NodeVector children;
  if (GTK_IS_CONTAINER(object_)) {
    GList* gtk_children = gtk_container_get_children(GTK_CONTAINER(object_));
    for (GList* elem = gtk_children; elem; elem = elem->next) {
      children.push_back(std::make_shared<GtkNode>(G_OBJECT(elem->data), shared_from_this()));
    }
    g_list_free(gtk_children);
  } else if (ATK_IS_OBJECT(object_)) {
    AtkObject *atk_object = ATK_OBJECT(object_);
    int n_children = atk_object_get_n_accessible_children(atk_object);
    for (int i = 0; i < n_children; i++) {
      AtkObject *child = atk_object_ref_accessible_child(atk_object, i);
      children.push_back(std::make_shared<GtkNode>(G_OBJECT(child), shared_from_this()));
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
