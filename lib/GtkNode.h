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

class GtkNode: public xpathselect::Node {
public:
  typedef std::shared_ptr<GtkNode> Ptr;

  GtkNode(GObject* object);

  virtual GVariant* Introspect() const;

  virtual std::string GetName() const;
  virtual bool MatchProperty(const std::string& name,
                             const std::string& value) const;
  virtual xpathselect::NodeList Children() const;

  static const std::string AP_ID_NAME;

private:
  GObject *object_;

  virtual GVariant* GetChildNodeNames() const;
  virtual intptr_t GetObjectId() const;
  virtual void GetGlobalRect(GdkRectangle* rect) const;
  virtual GVariant* ComposeRectVariant(gint x, gint y, gint height, gint width) const;
};

#endif // GTKNODE_H
