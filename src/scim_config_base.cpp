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
 * $Id: scim_config_base.cpp,v 1.19 2005/04/09 15:38:39 suzhe Exp $
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_CONFIG_MODULE
#include "scim_private.h"
#include "scim.h"

namespace scim {

ConfigPointer _scim_default_config (0);
ConfigModule _scim_default_config_module;

static ConfigPointer
_create_config (const String &default_module)
{
    if (!_scim_default_config_module.valid ()) {

        String module;

        if (!default_module.length ()) 
            module = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        else
            module = default_module;

        _scim_default_config_module.load (module);
    }

    if (_scim_default_config_module.valid ()) {
        _scim_default_config = _scim_default_config_module.create_config ();
        return _scim_default_config;
    }

    return ConfigPointer (0);
}

ConfigBase::ConfigBase ()
{
}

ConfigBase::~ConfigBase ()
{
}

bool
ConfigBase::valid () const
{
    return true;
}

String
ConfigBase::read (const String& key, const String& defVal) const
{
    String tmp;
    if (!read (key, &tmp)) {
        SCIM_DEBUG_CONFIG(1) << "Warning : No default scim::String value for key \"" << key << "\", "
                             << "using default value.\n";
        return defVal;
    }
    return tmp;
}

int
ConfigBase::read (const String& key, int defVal) const
{
    int tmp = 0;
    if (!read (key, &tmp)) {
        SCIM_DEBUG_CONFIG(1) << "Warning : No default int value for key \"" << key << "\", "
                             << "using default value.\n";
        return defVal;
    }
    return tmp;
}

double
ConfigBase::read (const String& key, double defVal) const
{
    double tmp = 0;
    if (!read (key, &tmp)) {
        SCIM_DEBUG_CONFIG(1) << "Warning : No default float value for key \"" << key << "\", "
                             << "using default value.\n";
        return defVal;
    }
    return tmp;
}

bool
ConfigBase::read (const String& key, bool defVal) const
{
    bool tmp = false;
    if (!read (key, &tmp)) {
        SCIM_DEBUG_CONFIG(1) << "Warning : No default bool value for key \"" << key << "\", "
                             << "using default value.\n";
        return defVal;
    }
    return tmp;
}

std::vector <String>
ConfigBase::read (const String& key, const std::vector <String>& defVal) const
{
    std::vector <String> tmp;
    if (!read (key, &tmp)) {
        SCIM_DEBUG_CONFIG(1) << "Warning : No default scim::String list value for key \"" << key << "\", "
                             << "using default value.\n";
        return defVal;
    }
    return tmp;
}

std::vector <int>
ConfigBase::read (const String& key, const std::vector <int>& defVal) const
{
    std::vector <int> tmp;
    if (!read (key, &tmp)) {
        SCIM_DEBUG_CONFIG(1) << "Warning : No default int list value for key \"" << key << "\", "
                             << "using default value.\n";
        return defVal;
    }
    return tmp;
}

bool
ConfigBase::reload ()
{
    if (!ConfigBase::valid ()) return false;

    m_signal_reload.emit (this);

    return true;
}

Connection
ConfigBase::signal_connect_reload (ConfigSlotVoid *slot)
{
    return m_signal_reload.connect (slot);
}

ConfigPointer
ConfigBase::set (const ConfigPointer &p_config)
{
    ConfigPointer old = _scim_default_config;

    _scim_default_config = p_config;

    return old;
}

ConfigPointer
ConfigBase::get (bool create_on_demand, const String &default_module)
{
    if (create_on_demand && _scim_default_config.null ())
        _create_config (default_module);
    return _scim_default_config;
}

/*
 * Implementation of DummyConfig
 */

DummyConfig::DummyConfig ()
{
}

DummyConfig::~DummyConfig ()
{
}

bool
DummyConfig::valid () const
{
    return ConfigBase::valid();
}

String
DummyConfig::get_name () const
{
    return "dummy";
}

// String
bool
DummyConfig::read (const String& key, String *pStr) const
{
    return false;
}

// int
bool
DummyConfig::read (const String& key, int *pl) const
{
    return false;
}

// double
bool
DummyConfig::read (const String& key, double* val) const
{
    return false;
}

// bool
bool
DummyConfig::read (const String& key, bool* val) const
{
    return false;
}

//String list
bool
DummyConfig::read (const String& key, std::vector <String>* val) const
{
    return false;
}

//int list
bool
DummyConfig::read (const String& key, std::vector <int>* val) const
{
    return false;
}

// write the value (return true on success)
bool
DummyConfig::write (const String& key, const String& value)
{
    return true;
}

bool
DummyConfig::write (const String& key, int value)
{
    return true;
}

bool
DummyConfig::write (const String& key, double value)
{
    return true;
}

bool
DummyConfig::write (const String& key, bool value)
{
    return true;
}

bool
DummyConfig::write (const String& key, const std::vector <String>& value)
{
    return true;
}

bool
DummyConfig::write (const String& key, const std::vector <int>& value)
{
    return true;
}


// permanently writes all changes
bool
DummyConfig::flush()
{
    return true;
}

// delete entries
bool
DummyConfig::erase (const String& key)
{
    return true;
}

// reload the configurations. 
bool
DummyConfig::reload ()
{
    return ConfigBase::reload ();
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
