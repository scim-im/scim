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
 * $Id: scim_imengine_module.cpp,v 1.3 2005/01/10 08:30:54 suzhe Exp $
 *
 */

#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_MODULE
#include "scim_private.h"
#include "scim.h"

namespace scim {

IMEngineModule::IMEngineModule ()
    : m_imengine_init (0),
      m_imengine_create_factory (0),
      m_number_of_factories (0)
{
}

IMEngineModule::IMEngineModule (const String &name, const ConfigPointer &config)
    : m_imengine_init (0),
      m_imengine_create_factory (0),
      m_number_of_factories (0)
{
    load (name, config);
}

bool
IMEngineModule::load (const String &name, const ConfigPointer &config)
{
    try {
        if (!m_module.load (name, "IMEngine"))
            return false;

        m_imengine_init =
            (IMEngineModuleInitFunc) m_module.symbol ("scim_imengine_module_init");

        m_imengine_create_factory =
            (IMEngineModuleCreateFactoryFunc) m_module.symbol ("scim_imengine_module_create_factory");

        if (!m_imengine_init || !m_imengine_create_factory) {
            m_module.unload ();
            m_imengine_init = 0;
            m_imengine_create_factory = 0;
            m_number_of_factories = 0;
            return false;
        }

        m_number_of_factories = m_imengine_init (config);
    } catch (...) {
        m_module.unload ();
        m_imengine_init = 0;
        m_imengine_create_factory = 0;
        m_number_of_factories = 0;
        return false;
    }

    return true;
}

bool
IMEngineModule::unload ()
{
    return m_module.unload ();
}

bool
IMEngineModule::valid () const
{
    return (m_module.valid () && m_imengine_init &&
            m_imengine_create_factory && m_number_of_factories > 0);
}

IMEngineFactoryPointer
IMEngineModule::create_factory (unsigned int engine) const
{
    if (valid () && engine < m_number_of_factories )
        return m_imengine_create_factory (engine);

    return IMEngineFactoryPointer (0);
}

unsigned int
IMEngineModule::number_of_factories () const
{
    return m_number_of_factories ;
}

int scim_get_imengine_module_list (std::vector <String>& engine_list)
{
    return scim_get_module_list (engine_list, "IMEngine");
}


} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
