/** @file scim_imengine_module.h
 *  @brief definition of IMEngineModule related classes.
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
 * $Id: scim_imengine_module.h,v 1.4 2005/01/10 08:30:54 suzhe Exp $
 */

#ifndef __SCIM_IMENGINE_MODULE_H
#define __SCIM_IMENGINE_MODULE_H

namespace scim {
/**
 * @addtogroup IMEngine
 * @{
 */

/**
 * @brief Initialize a IMEngine Module.
 *
 * There must be a function called "scim_imengine_module_init"
 * in each imengine module which complies with this prototype.
 * This function name can have a prefix like table_LTX_,
 * in which "table" is the module's name.
 *
 * @param config - a ConfigBase instance to maintain the configuration.
 * @return the number of factories supported by this IMEngine Module.
 */
typedef unsigned int (*IMEngineModuleInitFunc) (const ConfigPointer &config);

/**
 * @brief Create a factory instance for an engine,
 *
 * There must be a function called "scim_imengine_module_create_factory"
 * which complies with this prototype.
 * This function name can have a prefix like table_LTX_,
 * in which "table" is the module's name.
 *
 * @param engine - the index of the engine for which a factory object will be created.
 * @return the pointer of the factory object.
 */
typedef IMEngineFactoryPointer (*IMEngineModuleCreateFactoryFunc) (unsigned int engine);

/**
 * @brief The class to manipulate the IMEngine modules.
 *
 * This is a wrapper of scim::Module class, which is specially
 * for manipulating the IMEngine modules.
 */
class IMEngineModule
{
    Module m_module;

    IMEngineModuleInitFunc m_imengine_init;
    IMEngineModuleCreateFactoryFunc m_imengine_create_factory;

    unsigned int m_number_of_factories;

    IMEngineModule (const IMEngineModule &);
    IMEngineModule & operator= (const IMEngineModule &);

public:
    /**
     * @brief Default constructor.
     */
    IMEngineModule ();

    /**
     * @brief Constructor.
     * @param name - the module's name, eg. "rawcode".
     * @param config - a smart pointer points to a ConfigBase instance.
     */
    IMEngineModule (const String &name, const ConfigPointer &config);

    /**
     * @brief Load a IMEngine Module by its name.
     *
     * Load a module into memory.
     * If another module has been loaded into this object,
     * then the old module will be unloaded first.
     * If the old module is resident, false will be returned,
     * and the old module will be untouched.
     *
     * @param name - the name of the IMEngine Module.
     * @param config - the ConfigBase instance to be used for storing/loading configs.
     * @return true if success.
     */
    bool load  (const String &name, const ConfigPointer &config);

    /**
     * @brief Unload the IMEngine Module.
     * @return true if sucessfully unloaded.
     */
    bool unload ();

    /**
     * @brief Check if a module is loaded and initialized successfully.
     * @return true if a module is already loaded and initialized successfully.
     */
    bool valid () const;

    /**
     * @brief Get how many IMEngine factories supported by this module.
     *
     * @return the number of IMEngine factories.
     */
    unsigned int number_of_factories () const;

    /**
     * @brief Create an object for an IMEngine factory.
     *
     * @param engine - the index of this IMEngine factory,
     *                 must be less than the result of number_of_factories method
     *                 and greater than or equal to zero.
     * @return A smart pointer to the factory object, NULL if failed.
     */
    IMEngineFactoryPointer create_factory (unsigned int engine) const;
};

/**
 * @brief Get a name list of currently available IMEngine modules.
 * @param mod_list - the result list will be stored here.
 * @return the number of the modules, equal to mod_list.size ().
 */
int scim_get_imengine_module_list (std::vector <String> &mod_list);

/** @} */

} // namespace scim

#endif //__SCIM_IMENGINE_MODULE_H

/*
vi:ts=4:ai:nowrap:expandtab
*/
