/**
 * @file scim_helper_module.h
 * @brief Defines scim::HelperModule and it's related types.
 *
 * scim::HelperModule is a class used to load Client Helper modules.
 *
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2004-2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_helper_module.h,v 1.6 2005/01/10 08:30:54 suzhe Exp $
 */

#ifndef __SCIM_HELPER_MODULE_H
#define __SCIM_HELPER_MODULE_H

namespace scim {

/**
 * @addtogroup Helper
 * @{
 */

/**
 * @brief Get the number of Helpers in this module.
 *
 * A helper module can have multiple Helpers in it.
 * But each helper will run in its own process space.
 *
 * There must be a function called "scim_helper_module_number_of_helpers"
 * in each helper module which complies with this prototype.
 * This function name can have a prefix like kbd_LTX_,
 * in which "kbd" is the module's name.
 */
typedef unsigned int  (*HelperModuleNumberOfHelpersFunc) (void);

/**
 * @brief Get the information of a Helper.
 *
 * There must be a function called "scim_helper_module_get_helper_info"
 * in each helper module which complies with this prototype.
 * This function name can have a prefix like kbd_LTX_,
 * in which "kbd" is the module's name.
 *
 * @param idx The index of this helper, must between 0 to (the number of helpers) - 1.
 * @param info The HelperInfo object to store the information.
 * @return true if this Helper is valid and the correct information is stored into info.
 */
typedef bool (*HelperModuleGetHelperInfoFunc)   (unsigned int idx, HelperInfo &info); 

/**
 * @brief Run a specific Helper.
 *
 * This function will be called within an independent process.
 *
 * There must be a function called "scim_helper_module_run_helper"
 * in each helper module which complies with this prototype.
 * This function name can have a prefix like kbd_LTX_,
 * in which "kbd" is the module's name.
 *
 * @param config The Config object should be used to read/write configurations.
 * @param uuid The UUID of the Helper to be run.
 * @param display The display in which this helper should run.
 */
typedef void (*HelperModuleRunHelperFunc)       (const String &uuid, const ConfigPointer &config, const String &display);


/**
 * @brief The class used to load a Helper module and run its Helpers.
 *
 * This class should not be used directly. HelperManager should be used instead.
 */
class HelperModule
{
    Module                          m_module;

    HelperModuleNumberOfHelpersFunc m_number_of_helpers;
    HelperModuleGetHelperInfoFunc   m_get_helper_info;
    HelperModuleRunHelperFunc       m_run_helper;

    HelperModule (const HelperModule &);
    HelperModule & operator= (const HelperModule &);

public:
    /**
     * @brief Constructor.
     *
     * @param name The name of the Helper module to be loaded.
     */
    HelperModule (const String &name = String (""));

    /**
     * @brief Load a Helper module.
     *
     * If a module has already been loaded, then it'll be unloaded first.
     *
     * @param name The name of the Helper module to be loaded.
     * @return true if success.
     */
    bool load (const String &name);

    /**
     * @brief Unload the module.
     *
     * @return true if success.
     */
    bool unload ();

    /**
     * @brief Check if a Helper module has been loaded successfully.
     *
     * @return true if a module has been loaded successfully.
     */
    bool valid () const;

    /**
     * @brief Get the number of helpers supported by this module.
     *
     * @return the number of helpers supported by this module.
     */
    unsigned int number_of_helpers () const;

    /**
     * @brief The the information of a specific helper.
     *
     * @param idx The index of the helper, must between 0 to number_of_helpers () - 1.
     * @param info The HeperInfo object to store the information.
     * @return true if this helper is ok and the information is stored into info successfully.
     */
    bool get_helper_info (unsigned int idx, HelperInfo &info) const;

    /**
     * @brief Run a specific helper.
     *
     * The helper should be run in an independent process, this function will not return
     * until the helper exits.
     *
     * @param config The Config object to be used to read configurations.
     * @param uuid The UUID of the helper, which is returned by get_helper_info ().
     * @param display The display in which this helper should run.
     */
    void run_helper (const String &uuid, const ConfigPointer &config, const String &display) const;
};

/**
 * @brief Get a name list of currently available Helper modules.
 * @param mod_list - the result list will be stored here.
 * @return the number of the modules, equal to mod_list.size ().
 */
int scim_get_helper_module_list (std::vector <String> &mod_list);
/**  @} */

} // namespace scim

#endif //__SCIM_HELPER_MODULE_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

