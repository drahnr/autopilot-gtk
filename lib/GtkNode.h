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

#ifndef GTKNODE_H
#define GTKNODE_H

#include <gtk/gtk.h>
#include <glib-object.h>
#include <xpathselect/node.h>
#include <xpathselect/xpathselect.h>
#include <string>
#include <cstdint>
#include "Variant.h"

// #include <memory>

class GtkNode: public xpathselect::Node, public std::enable_shared_from_this<GtkNode>
{
public:
  typedef std::shared_ptr<const GtkNode> Ptr;

  GtkNode(GObject* object, Ptr const& parent);
  GtkNode(GObject* object);
  virtual ~GtkNode();

  virtual GVariant* Introspect() const;

  virtual std::string GetName() const;
  virtual std::string GetPath() const;
  virtual int32_t GetId() const;
  virtual xpathselect::Node::Ptr GetParent() const;
  virtual bool MatchStringProperty(const std::string& name,
                                   const std::string& value) const;
  virtual bool MatchIntegerProperty(const std::string& name,
                                    int32_t value) const;
  virtual bool MatchBooleanProperty(const std::string& name,
                                    bool value) const;
  virtual xpathselect::NodeVector Children() const;

  static const std::string AP_ID_NAME;

private:
  GObject *object_;
  std::string full_path_;
  Ptr parent_;

  virtual GVariant* GetChildNodeNames() const;
  virtual void GetGlobalRect(GdkRectangle* rect) const;
  void AddAtkComponentProperties(variant::BuilderWrapper &builder_wrapper,
                                 AtkComponent *atk_component) const;
};

#endif // GTKNODE_H
