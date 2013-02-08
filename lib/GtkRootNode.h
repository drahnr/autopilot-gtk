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

#ifndef ROOTGTKNODE_H
#define ROOTGTKNODE_H

#include "GtkNode.h"

#include <glib.h>
#include <gio/gio.h>

#include <list>

class GtkRootNode: public GtkNode
{
 public:
  GtkRootNode();

  virtual GVariant* Introspect() const;
  virtual intptr_t GetObjectId() const;

  virtual std::string GetName() const;
  virtual bool MatchProperty(const std::string& name, const std::string& value) const;
  virtual xpathselect::NodeList Children() const;

 private:
  virtual GVariant* GetChildNodeNames() const;
};

#endif // ROOTGTKNODE_H
