/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 2 -*-
 * Copyright (C) 2013 Canonical Ltd
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
 */

#ifndef AUTOPILOT_TYPES_H
#define AUTOPILOT_TYPES_H

/// IMPORTANT: THese constants are taken from the autopilot XPathSelect protocol document.
/// Only add options here if the support has been added for them in autopilot itself.
enum autopilot_type_id
{
    TYPE_PLAIN = 0,
    TYPE_RECT = 1,
    TYPE_POINT = 2,
    TYPE_SIZE = 3,
    TYPE_COLOR = 4,
    TYPE_DATETIME = 5,
    TYPE_TIME = 6,
};

#endif
