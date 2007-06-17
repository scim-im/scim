/**
 * @file scim_helper_manager.h
 * @brief Defines scim::HelperManager.
 *
 * scim::HelperManager is a class used to manage all Client Helper modules.
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
 * $Id: scim_helper_manager.h,v 1.3 2005/05/17 06:45:15 suzhe Exp $
 */

#ifndef __SCIM_HELPER_MANAGER_H
#define __SCIM_HELPER_MANAGER_H

namespace scim {

/**
 * @addtogroup Helper
 * @{
 */

/**
 * @brief This class is used to manage all helper objects.
 *
 */
class HelperManager
{
    class HelperManagerImpl;
    HelperManagerImpl *m_impl;

    HelperManager (const HelperManager &);
    const HelperManager & operator = (const HelperManager &);

public:
    HelperManager ();
    ~HelperManager ();

    /**
     * @brief Get the total number of helpers supported by all helper modules.
     */
    unsigned int number_of_helpers () const;

    /**
     * @brief Get the information of a specific helper by its index.
     *
     * @param idx The index of the helper, must between 0 to number_of_helpers () - 1.
     * @param info The HelperInfo object to store the information.
     * @return true if this helper is ok and the information is stored correctly.
     */
    bool get_helper_info (unsigned int idx, HelperInfo &info) const;

    /**
     * @brief Run a specific helper.
     *
     * The helper will run in a newly forked process, so this function will return as soon
     * as the new process is launched.
     * 
     * @param config_name The name of the ConfigModule to be used to read configurations.
     * @param uuid The UUID of the helper to be run.
     * @param display The display in which the helper will be run.
     */
    void run_helper (const String &uuid, const String &config_name, const String &display) const;
};

/** @} */

} // namespace scim

#endif //__SCIM_HELPER_MANAGER_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

