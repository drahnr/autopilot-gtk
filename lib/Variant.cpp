// -*- Mode: C++; indent-tabs-mode: nil; tab-width: 2 -*-
/*
 * Copyright (C) 2010 Canonical Ltd
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
 * Authored by: Tim Penhey <tim.penhey@canonical.com>
 */

#include <iostream>
#include "Variant.h"

namespace variant
{

BuilderWrapper::BuilderWrapper(GVariantBuilder* builder)
  : builder_(builder)
{}

BuilderWrapper& BuilderWrapper::add(char const* name, bool value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_boolean(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, char const* value)
{
  if (value)
    g_variant_builder_add(builder_, "{sv}", name, g_variant_new_string(value));
  else
    g_variant_builder_add(builder_, "{sv}", name, g_variant_new_string(""));

  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, std::string const& value)
{
  g_variant_builder_add(builder_, "{sv}", name,
                        g_variant_new_string(value.c_str()));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, int value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_int32(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, long int value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_int64(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, long long int value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_int64(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, unsigned int value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_uint32(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, long unsigned int value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_uint64(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, long long unsigned int value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_uint64(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, float value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_double(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, double value)
{
  g_variant_builder_add(builder_, "{sv}", name, g_variant_new_double(value));
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, GVariant* value)
{
  g_variant_builder_add(builder_, "{sv}", name, value);
  return *this;
}

BuilderWrapper& BuilderWrapper::add(char const* name, GValue* value)
{
  switch (G_VALUE_TYPE(value)) {
  case G_TYPE_CHAR:
    {
      add(name, g_value_get_schar(value));
    }
    break;
  case G_TYPE_UCHAR:
    {
      add(name, g_value_get_uchar(value));
    }
    break;
  case G_TYPE_BOOLEAN:
    {
      add(name, g_value_get_boolean(value));
    }
    break;
  case G_TYPE_INT:
    {
      add(name, g_value_get_int(value));
    }
    break;
  case G_TYPE_UINT:
    {
      add(name, g_value_get_uint(value));
    }
    break;
  case G_TYPE_LONG:
    {
      add(name, g_value_get_long(value));
    }
    break;
  case G_TYPE_ULONG:
    {
      add(name, g_value_get_ulong(value));
    }
    break;
  case G_TYPE_INT64:
    {
      add(name, g_value_get_int64(value));
    }
    break;
  case G_TYPE_UINT64:
    {
      add(name, g_value_get_uint64(value));
    }
    break;
  case G_TYPE_ENUM:
    {
      add(name, g_value_get_enum(value));
    }
    break;
  case G_TYPE_FLAGS:
    {
      add(name, g_value_get_flags(value));
    }
    break;
  case G_TYPE_FLOAT:
    {
      add(name, g_value_get_float(value));
    }
    break;
  case G_TYPE_DOUBLE:
    {
      add(name, g_value_get_double(value));
    }
    break;
  case G_TYPE_STRING:
    {
      add(name, g_value_get_string(value));
    }
    break;
  case G_TYPE_POINTER:
    {
      add(name, g_value_get_pointer(value));
    }
    break;
  case G_TYPE_BOXED:
    {
      add(name, g_value_get_boxed(value));
    }
    break;
  case G_TYPE_PARAM:
    {
      add(name, g_value_get_param(value));
    }
    break;
  case G_TYPE_OBJECT:
    {
      add(name, g_value_get_object(value));
    }
    break;
  case G_TYPE_VARIANT:
    {
      add(name, g_value_get_variant(value));
    }
    break;
  default:
    //g_warning("unsupported type: %s", g_type_name(G_VALUE_TYPE(value)));
    {}
    break;
  }
  return *this;
}

}
