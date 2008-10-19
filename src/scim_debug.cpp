/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
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
 *
 * $Id: scim_debug.cpp,v 1.10 2005/08/05 01:54:24 suzhe Exp $
 *
 */

#define Uses_SCIM_DEBUG
#include "scim_private.h"
#include "scim.h"
#include <cstdio>

namespace scim {

struct _DebugMaskName
{
    uint32 mask;
    const char  *name;
};

static _DebugMaskName _debug_mask_names [] =
{
    {SCIM_DEBUG_AllMask,        "all"},
    {SCIM_DEBUG_MainMask,       "main"},
    {SCIM_DEBUG_ConfigMask,     "config"},
    {SCIM_DEBUG_IMEngineMask,   "imengine"},
    {SCIM_DEBUG_BackEndMask,    "backend"},
    {SCIM_DEBUG_FrontEndMask,   "frontend"},
    {SCIM_DEBUG_ModuleMask,     "module"},
    {SCIM_DEBUG_UtilityMask,    "utility"},
    {SCIM_DEBUG_IConvMask,      "iconv"},
    {SCIM_DEBUG_LookupTableMask,"lookuptable"},
    {SCIM_DEBUG_SocketMask,     "socket"},
    {0, 0}
};

uint32         DebugOutput::current_verbose = 0;
uint32         DebugOutput::current_mask = 0;
uint32         DebugOutput::verbose_level = 0;
uint32         DebugOutput::output_mask = ~0;
std::ostream * DebugOutput::output_stream = &std::cerr;

static std::ofstream __debug_output_file;

#if ENABLE_DEBUG
DebugOutput::DebugOutput (uint32 mask, uint32 verbose)
{
    current_mask    = mask;
    current_verbose = verbose;
}
#else
DebugOutput::DebugOutput (uint32 mask, uint32 verbose)
{
}
#endif

void
DebugOutput::set_verbose_level (uint32 verbose)
{
    verbose_level =
        (verbose > SCIM_DEBUG_MAX_VERBOSE) ? SCIM_DEBUG_MAX_VERBOSE : verbose;
}

void
DebugOutput::enable_debug (uint32 debug)
{
    output_mask |= debug;
}

void
DebugOutput::enable_debug_by_name (const String &debug)
{
    _DebugMaskName *p = _debug_mask_names;
    while (p->mask && p->name) {
        if (String (p->name) == debug) {
            output_mask |= p->mask;
            return;
        }
        ++ p;
    }
}

void
DebugOutput::disable_debug (uint32 debug)
{
    output_mask &= (~debug);
}

void
DebugOutput::disable_debug_by_name (const String &debug)
{
    _DebugMaskName *p = _debug_mask_names;
    while (p->mask && p->name) {
        if (String (p->name) == debug) {
            output_mask &= (~(p->mask));
            return;
        }
        ++ p;
    }
}

void
DebugOutput::set_output (const String &file)
{
    DebugOutput::output_stream = &std::cerr;

    if (file.length ()) {
        if (file == String ("stderr") || file == String ("cerr"))
            DebugOutput::output_stream = &std::cerr;
        else if (file == String ("stdout") || file == String ("cout"))
            DebugOutput::output_stream = &std::cout;
        else if (file == String ("none") || file == String ("off"))
            DebugOutput::output_stream = 0;
        else {
            __debug_output_file.open (file.c_str ());
            if (__debug_output_file.is_open ())
                DebugOutput::output_stream = &__debug_output_file;
        }
    }
}

String
DebugOutput::serial_number ()
{
    static unsigned int serial = 0;
    char buf [40];
    snprintf (buf, 40, "<%08u>:", serial ++);
    return String (buf);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
