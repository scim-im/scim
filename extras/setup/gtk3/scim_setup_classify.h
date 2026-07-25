/** @file scim_setup_classify.h
 *  @brief Classify a SetupUI plugin by the GUI toolkit it links against.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2026 SCIM developers
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#pragma once

#include <scim.h>

using namespace scim;

// Which GUI toolkit a SetupUI plugin is linked against, decided purely from
// its direct DT_NEEDED entries (no dlopen -- loading a foreign-toolkit plugin
// into this process would map an incompatible GTK and abort).
enum SetupToolkit
{
    SETUP_TOOLKIT_GTK4 = 0,     // libgtk-4.so.1     -> load in-process
    SETUP_TOOLKIT_GTK3,         // libgtk-3.so.0     -> hand to the legacy GTK3 helper
    SETUP_TOOLKIT_GTK2,         // libgtk-x11-2.0.so -> skip (EOL toolkit)
    SETUP_TOOLKIT_UNKNOWN       // no GTK dependency / unreadable -> skip
};

// Resolve a SetupUI module base name (as returned by scim_get_setup_module_list)
// to the absolute path of its shared object, searching the same directories the
// core module loader uses. Returns an empty string if not found.
String scim_setup_find_module_path (const String &name);

// Classify the given SetupUI module by reading the DT_NEEDED entries of its
// shared object with libelf. Returns SETUP_TOOLKIT_UNKNOWN if the module cannot
// be located or read.
SetupToolkit scim_setup_classify_module (const String &name);
