/** @file scim_config_module.h
 *  @brief Define scim::ConfigModule class for manipulating the config modules.
 *
 *  Class scim::ConfigModule is a wrapper of class scim::Module,
 *  which is for manipulating the configuration modules.
 */

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
 * $Id: scim_config_module.h,v 1.15 2005/04/09 15:38:39 suzhe Exp $
 */

#ifndef __SCIM_CONFIG_MODULE_H
#define __SCIM_CONFIG_MODULE_H

namespace scim {
/**
 * @addtogroup Config
 * @{
 */

/**
 * @brief The prototype of initialization function in config modules.
 *
 * There must be a function called "scim_config_module_init"
 * which complies with this prototype.
 * This function name can have a prefix like simple_LTX_,
 * in which "simple" is the module's name.
 */
typedef void (*ConfigModuleInitFunc) (void);

/**
 * @brief The prototype of configure object creation function in config modules.
 *
 * There must be a function called "scim_config_module_create_config"
 * which complies with this prototype.
 * This function name can have a prefix like simple_LTX_,
 * in which "simple" is the module's name.
 */
typedef ConfigPointer (*ConfigModuleCreateConfigFunc) ();

/**
 * @brief The class to manipulate the config modules.
 *
 * This is a wrapper of scim::Module class, which is specially
 * for manipulating the config modules.
 */
class ConfigModule
{
    Module      m_module;

    ConfigModuleInitFunc m_config_init;
    ConfigModuleCreateConfigFunc m_config_create_config;

    ConfigModule (const ConfigModule &);
    ConfigModule & operator= (const ConfigModule &);

public:
    /**
     * @brief Default constructor.
     */
    ConfigModule ();

    /**
     * @brief Constructor.
     * @param name - the module's name, eg. "simple".
     */
    ConfigModule (const String &name);

    /**
     * @brief Load a module by its name.
     *
     * Load a module into memory.
     * If another module has been loaded into this object,
     * then the old module will be unloaded first.
     * If the old module is resident, false will be returned,
     * and the old module will be untouched.
     * 
     * @param name - the module's name, eg. "simple".
     * @return true if success.
     */
    bool load  (const String &name);

    /**
     * @brief Check if a module is loaded and initialized successfully.
     * @return true if a module is already loaded and initialized successfully.
     */
    bool valid () const;

    /**
     * @brief Create a configuration object from this module.
     *
     * The type of newly created configuration object
     * must be a derived class of scim::ConfigBase.
     *
     * @return a smart pointer points to the configuration object.
     */
    ConfigPointer create_config () const;
};

/**
 * @brief Get a name list of currently available configuration modules.
 * @param mod_list - the result list will be stored here.
 * @return the number of the modules, equal to mod_list.size ().
 */
int scim_get_config_module_list (std::vector <String>& mod_list);

/** @} */

} // namespace scim

#endif //__SCIM_CONFIG_MODULE_H

/*
vi:ts=4:ai:nowrap:expandtab
*/
