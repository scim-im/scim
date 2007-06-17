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
 * $Id: scim_config_module.cpp,v 1.15 2005/04/09 15:38:39 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_MODULE
#include "scim_private.h"
#include "scim.h"

namespace scim {

ConfigModule::ConfigModule ()
    : m_config_init (0),
      m_config_create_config (0)
{
}

ConfigModule::ConfigModule (const String &name)
    : m_config_init (0),
      m_config_create_config (0)
{
    load (name);
}

bool
ConfigModule::load (const String &name)
{
    try {
        if (!m_module.load (name, "Config"))
            return false;
 
        m_config_init = (ConfigModuleInitFunc) m_module.symbol ("scim_config_module_init");
        m_config_create_config = (ConfigModuleCreateConfigFunc) m_module.symbol ("scim_config_module_create_config");
 
        if (!m_config_init || !m_config_create_config) {
            m_module.unload ();
            m_config_init = 0;
            m_config_create_config = 0;
            return false;
        }
 
        m_config_init ();
    } catch (...) {
        m_module.unload ();
        m_config_init = 0;
        m_config_create_config = 0;
        return false;
    }

    return true;
}

bool
ConfigModule::valid () const
{
    return (m_module.valid () && m_config_init && m_config_create_config);
}

ConfigPointer
ConfigModule::create_config () const
{
    if (valid ())
        return m_config_create_config ();

    return ConfigPointer ();
}

int scim_get_config_module_list (std::vector <String>& mod_list)
{
    return scim_get_module_list (mod_list, "Config");
}


} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
