/** @file scim_filter_module.h
 *  @brief definition of FilterModule related classes.
 */

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
 * $Id: scim_filter_module.h,v 1.7 2005/10/06 18:02:06 liuspider Exp $
 */

#ifndef __SCIM_FILTER_MODULE_H
#define __SCIM_FILTER_MODULE_H

namespace scim {
/**
 * @addtogroup IMEngine
 * @{
 */

/**
 * @brief Initialize a Filter Module.
 *
 * There must be a function called "scim_filter_module_init"
 * in each filter module which complies with this prototype.
 * This function name can have a prefix like sctc_LTX_,
 * in which "sctc" is the module's name.
 *
 * If a filter needs services from other IMEngineFactory objects, it should
 * obtain the pointer of these IMEngineFactory objects from the given BacnEnd
 * pointer. But please note that, those IMEngineFactory objects may not be loaded yet,
 * when initializing this Filter module.
 * So the pointers of available IMEngineFactory objects should only be queried from
 * the BackEnd at the first time to used by this Filter.
 *
 * @param config  a ConfigBase instance to maintain the configuration.
 *
 * @return the number of filters provided by this module.
 */
typedef unsigned int (*FilterModuleInitFunc) (const ConfigPointer &config);

/**
 * @brief Create an object of this FilterFactory class
 *
 * There must be a function called "scim_filter_module_create_filter"
 * which complies with this prototype.
 * This function name can have a prefix like sctc_LTX_,
 * in which "sctc" is the module's name.
 *
 * A new FilterFactory object should be returned for each call. Because
 * multiple objects would be used at the same time.
 *
 * @param index the index of the FilterFactory to be used to create the object.
 *              Must between 0 and (number_of_filters - 1).
 * @param backend  the BackEnd instance which holds all real IMEngineFactory objects.
 *                  Some filter may want to use other IMEngineFactory object
 *                  to do some job, eg. inverse convert.
 *
 * @return the pointer of the FilterFactory object.
 */
typedef FilterFactoryPointer (*FilterModuleCreateFilterFunc) (unsigned int index);

/**
 * @brief Get basic information of the FilterFactory class provided by this module.
 *
 * There must be a function called "scim_filter_module_get_filter_info"
 * which complies with this prototype.
 * This function name can have a prefix like sctc_LTX_,
 * in which "sctc" is the module's name.
 *
 *
 * @param index The index of the FilterFactory to be queried.
 *
 * @param info The object to hold the information.
 * @param index the index of the Filter to be queried.
 *
 * @return The uuid of this FilterFactory class.
 */
typedef bool (*FilterModuleGetFilterInfoFunc) (unsigned int index, FilterInfo &info);

/**
 * @brief The class to manipulate the Filter modules.
 *
 * This is a wrapper of scim::Module class, which is specially
 * for manipulating the Filter modules.
 */
class FilterModule
{
    Module m_module;

    FilterModuleInitFunc          m_filter_init;
    FilterModuleCreateFilterFunc  m_filter_create_filter;
    FilterModuleGetFilterInfoFunc m_filter_get_filter_info;

    unsigned int m_number_of_filters;

    FilterModule (const FilterModule &);
    FilterModule & operator= (const FilterModule &);

public:
    /**
     * @brief Default constructor.
     */
    FilterModule ();

    /**
     * @brief Constructor.
     * @param name  the module's name, eg. "sctc".
     * @param config  a ConfigBase instance to maintain the configuration.
     */
    FilterModule (const String &name, const ConfigPointer &config);

    /**
     * @brief Load a Filter Module by its name.
     *
     * Load a module into memory.
     * If another module has been loaded into this object,
     * then the old module will be unloaded first.
     * If the old module is resident, false will be returned,
     * and the old module will be untouched.
     *
     * @param name - the name of the Filter Module.
     * @param config  a ConfigBase instance to maintain the configuration.
     *
     * @return true if success.
     */
    bool load  (const String &name, const ConfigPointer &config);

    /**
     * @brief Unload the Filter Module.
     * @return true if sucessfully unloaded.
     */
    bool unload ();

    /**
     * @brief Check if a module is loaded and initialized successfully.
     * @return true if a module is already loaded and initialized successfully.
     */
    bool valid () const;

    /**
     * @brief Get how many Filter factories supported by this module.
     *
     * @return the number of Filter factories.
     */
    unsigned int number_of_filters () const;

    /**
     * @brief Create an object for a Filter factory.
     *
     * @param index The index of the Filter factory,
     *                 must be less than the result of number_of_factories method
     *                 and greater than or equal to zero.
     * @return A smart pointer to the factory object, NULL if failed.
     */
    FilterFactoryPointer create_filter (unsigned int index) const;

    /**
     * @brief Get basic information of the FilterFactory class provided by this module.
     *
     * @param index The index of the Filter factory to be queried. 
     * @param info The result will be stored in this parameter.
     *
     * @return whether the info is successfully retrieved
     */
    bool get_filter_info (unsigned int index, FilterInfo &info) const;
};

/**
 * @brief Get a name list of currently available Filter modules.
 * @param mod_list - the result list will be stored here.
 * @return the number of the modules, equal to mod_list.size ().
 */
int scim_get_filter_module_list (std::vector <String> &mod_list);

/** @} */

} // namespace scim

#endif //__SCIM_FILTER_MODULE_H

/*
vi:ts=4:ai:nowrap:expandtab
*/

