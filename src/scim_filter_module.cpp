/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_filter_module.cpp,v 1.5 2005/05/24 12:22:51 suzhe Exp $
 *
 */

#define Uses_SCIM_FILTER_MODULE
#include "scim_private.h"
#include "scim.h"

namespace scim {

FilterModule::FilterModule ()
    : m_filter_init (0),
      m_filter_create_filter (0),
      m_filter_get_filter_info (0),
      m_number_of_filters (0)
{
}

FilterModule::FilterModule (const String &name, const ConfigPointer &config)
    : m_filter_init (0),
      m_filter_create_filter (0),
      m_filter_get_filter_info (0),
      m_number_of_filters (0)
{
    load (name, config);
}

bool
FilterModule::load (const String &name, const ConfigPointer &config)
{
    try {
        if (!m_module.load (name, "Filter"))
            return false;

        m_filter_init =
            (FilterModuleInitFunc) m_module.symbol ("scim_filter_module_init");

        m_filter_create_filter =
            (FilterModuleCreateFilterFunc) m_module.symbol ("scim_filter_module_create_filter");

        m_filter_get_filter_info =
            (FilterModuleGetFilterInfoFunc) m_module.symbol ("scim_filter_module_get_filter_info");

        if (!m_filter_init || !m_filter_create_filter || !m_filter_get_filter_info ||
            (m_number_of_filters = m_filter_init (config)) == 0) {
            m_module.unload ();
            m_filter_init = 0;
            m_filter_create_filter = 0;
            m_filter_get_filter_info = 0;
            return false;
        }
    } catch (...) {
        m_module.unload ();
        m_filter_init = 0;
        m_filter_create_filter = 0;
        m_filter_get_filter_info = 0;
        return false;
    }

    return true;
}

bool
FilterModule::unload ()
{
    return m_module.unload ();
}

bool
FilterModule::valid () const
{
    return (m_module.valid () && m_filter_init && m_number_of_filters > 0 &&
            m_filter_create_filter && m_filter_get_filter_info);
}

FilterFactoryPointer
FilterModule::create_filter (unsigned int index) const
{
    if (valid () && index < m_number_of_filters)
        return m_filter_create_filter (index);

    return FilterFactoryPointer (0);
}

bool
FilterModule::get_filter_info (unsigned int index, FilterInfo &info) const
{
    if (valid () && index < m_number_of_filters)
        return m_filter_get_filter_info (index, info);

    return false;
}

unsigned int
FilterModule::number_of_filters () const
{
    return m_number_of_filters;
}


int scim_get_filter_module_list (std::vector <String>& engine_list)
{
    return scim_get_module_list (engine_list, "Filter");
}


} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
