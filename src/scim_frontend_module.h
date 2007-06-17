/** @file scim_frontend_module.h
 * @brief definition of FrontEndModule related classes.
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
 * $Id: scim_frontend_module.h,v 1.16 2005/01/10 08:30:54 suzhe Exp $
 */

#ifndef __SCIM_FRONTEND_MODULE_H
#define __SCIM_FRONTEND_MODULE_H

namespace scim {

/**
 * @addtogroup FrontEnd
 * @{
 */

/**
 * @brief Initialize a FrontEnd Module.
 *
 * There must be a function called "scim_frontend_module_init"
 * in each frontend module which complies with this prototype.
 * This function name can have a prefix like x11_LTX_,
 * in which "x11" is the module's name.
 *
 * @param backend - a BackEnd instance which hold all IMEngineFactory instances.
 * @param config - a ConfigBase instance to maintain the configuration.
 */
typedef void (*FrontEndModuleInitFunc) (const BackEndPointer &backend,
                                        const ConfigPointer &config,
                                        int argc,
                                        char **argv);

/**
 * @brief Run a FrontEnd Module.
 *
 * There must be a function called "scim_frontend_module_run"
 * in each frontend module which complies with this prototype.
 * This function name can have a prefix like x11_LTX_,
 * in which "x11" is the module's name.
 */
typedef void (*FrontEndModuleRunFunc)  (void);

/**
 * @brief The class to manipulate the frontend modules.
 *
 * This is a wrapper of scim::Module class, which is specially
 * for manipulating the frontend modules.
 */
class FrontEndModule 
{
    Module       m_module;

    FrontEndModuleInitFunc m_frontend_init;
    FrontEndModuleRunFunc m_frontend_run;

    FrontEndModule (const FrontEndModule &);
    FrontEndModule & operator= (const FrontEndModule &);

public:
    /**
     * @brief Default constructor.
     */
    FrontEndModule ();

    /**
     * @brief Constructor.
     * @param name - the module's name, eg. "rawcode".
     * @param backend - a BackEnd instance which holds all IMEngineFactory instances.
     * @param config - a smart pointer points to a ConfigBase instance.
     * @param argc - the number of (fake) command line arguments
     * @param argv - the (fake) command line argument vector
     */
    FrontEndModule (const String          &name,
                    const BackEndPointer  &backend,
                    const ConfigPointer   &config,
                    int                    argc,
                    char                 **argv);

    /**
     * @brief Load a FrontEnd module by its name.
     *
     * Load a module into memory.
     * If another module has been loaded into this object,
     * then the old module will be unloaded first.
     * If the old module is resident, false will be returned,
     * and the old module will be untouched.
     *
     * @param name - the module's name, eg. "rawcode".
     * @param backend - a BackEnd instance which holds all IMEngineFactory instances.
     * @param config - a smart pointer points to a ConfigBase instance.
     * @param argc - the number of (fake) command line arguments
     * @param argv - the (fake) command line argument vector
     */
    bool load  (const String          &name,
                const BackEndPointer  &backend,
                const ConfigPointer   &config,
                int                    argc,
                char                 **argv);

    /**
     * @brief Check if a module is loaded and initialized successfully.
     * @return true if a module is already loaded and initialized successfully.
     */
    bool valid () const;

    /**
     * @brief run this FrontEnd module.
     */
    void run () const;
};

/**
 * @brief Get a name list of currently available frontend modules.
 * @param mod_list - the result list will be stored here.
 * @return the number of the modules, equal to mod_list.size ().
 */
int scim_get_frontend_module_list (std::vector <String>& mod_list);

/** @} */

} // namespace scim

#endif //__SCIM_FRONTEND_MODULE_H

/*
vi:ts=4:ai:nowrap:expandtab
*/
