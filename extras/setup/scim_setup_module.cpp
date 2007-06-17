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
 * $Id: scim_setup_module.cpp,v 1.9 2005/01/10 08:30:45 suzhe Exp $
 *
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_MODULE

#include "scim_private.h"
#include "scim.h"
#include "scim_setup_module.h"

SetupModule::SetupModule ()
    : m_create_ui (0),
      m_get_category (0),
      m_get_name (0),
      m_get_description (0),
      m_load_config (0),
      m_save_config (0)
{
}

SetupModule::SetupModule (const String &name)
    : m_create_ui (0),
      m_get_category (0),
      m_get_name (0),
      m_get_description (0),
      m_load_config (0),
      m_save_config (0)
{
    load (name);
}

bool
SetupModule::load (const String &name)
{
    if (!m_module.load (name, "SetupUI"))
        return false;

    m_create_ui       = (SetupModuleCreateUIFunc) m_module.symbol ("scim_setup_module_create_ui");
    m_get_category    = (SetupModuleGetCategoryFunc) m_module.symbol ("scim_setup_module_get_category");
    m_get_name        = (SetupModuleGetNameFunc) m_module.symbol ("scim_setup_module_get_name");
    m_get_description = (SetupModuleGetDescriptionFunc) m_module.symbol ("scim_setup_module_get_description");
    m_load_config     = (SetupModuleLoadConfigFunc) m_module.symbol ("scim_setup_module_load_config");
    m_save_config     = (SetupModuleSaveConfigFunc) m_module.symbol ("scim_setup_module_save_config");
    m_query_changed   = (SetupModuleQueryChangedFunc) m_module.symbol ("scim_setup_module_query_changed");


    if (!m_create_ui || !m_get_category || !m_get_name ||
        !m_load_config || !m_save_config) {
        m_module.unload ();
        m_create_ui = 0;
        m_get_category = 0;
        m_get_name = 0;
        m_get_description = 0;
        m_load_config = 0;
        m_save_config = 0;
        return false;
    }

    return true;
}

bool
SetupModule::valid () const
{
    return (m_module.valid () &&
            m_create_ui && m_get_category && m_get_name &&
            m_load_config && m_save_config);
}

GtkWidget *
SetupModule::create_ui () const
{
    if (valid ())
        return m_create_ui ();

    return 0;
}

String
SetupModule::get_category () const
{
    if (valid ())
        return m_get_category ();

    return String ();
}

String
SetupModule::get_name () const
{
    if (valid ())
        return m_get_name ();

    return String ();
}

String
SetupModule::get_description () const
{
    if (valid () && m_get_description)
        return m_get_description ();

    return String ();
}

void
SetupModule::load_config (const ConfigPointer &config) const
{
    if (valid ())
        m_load_config (config);
}

void
SetupModule::save_config (const ConfigPointer &config) const
{
    if (valid ())
        m_save_config (config);
}

bool
SetupModule::query_changed () const
{
    if (valid () && m_query_changed)
        return m_query_changed ();
    return false;
}

int scim_get_setup_module_list (std::vector <String>& mod_list)
{
    return scim_get_module_list (mod_list, "SetupUI");
}


/*
vi:ts=4:nowrap:ai:expandtab
*/
