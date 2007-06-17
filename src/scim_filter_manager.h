/**
 * @file scim_filter_manager.h
 * @brief Defines scim::FilterManager.
 *
 * scim::FilterManager is a class used to manage all Filter modules.
 *
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
 * $Id: scim_filter_manager.h,v 1.3 2005/05/28 13:54:59 suzhe Exp $
 */

#ifndef __SCIM_FILTER_MANAGER_H
#define __SCIM_FILTER_MANAGER_H

namespace scim {

/**
 * @addtogroup IMEngine
 * @{
 */

class FilterManager
{
    class FilterManagerImpl;
    FilterManagerImpl *m_impl;

    FilterManager (const FilterManager &);
    const FilterManager & operator = (const FilterManager &);

public:
    FilterManager (const ConfigPointer &config);

    ~FilterManager ();

    /**
     * @brief Get the total number of Filters supported by all filter modules.
     */
    unsigned int number_of_filters () const;

    /**
     * @brief Get the information of a specific filter by its index.
     *
     * @param idx The index of the filter, must between 0 to number_of_filters () - 1.
     * @param info The FilterInfo object to store the information.
     * @return true if this filter is ok and the information is stored correctly.
     */
    bool get_filter_info (unsigned int idx, FilterInfo &info) const;

    /**
     * @brief Get the information of a specific filter by its uuid.
     *
     * @param uuid The uuid of the filter.
     * @param info The FilterInfo object to store the information.
     * @return true if this filter is ok and the information is stored correctly.
     */
    bool get_filter_info (const String &uuid, FilterInfo &info) const;

    /**
     * @brief Clear all Filter settings for IMEngines.
     */
    void   clear_all_filter_settings () const;

    /**
     * @brief Get a list of Filters binded to an IMEngine.
     *
     * @param uuid The uuid of the IMEngine to be queried.
     * @param filters The list of Filters' UUIDs binded to the IMEngine will be stored here.
     *
     * @return How many filters binded to this IMEngine.
     */
    size_t get_filters_for_imengine (const String &uuid, std::vector <String> &filters) const;

    /**
     * @brief Bind one or more Filters to an IMEngine. 
     *
     * @param uuid The uuid of the IMEngine to be binded.
     * @param filters The list of Filters' UUIDs to be binded to the IMEngine.
     */
    void   set_filters_for_imengine (const String &uuid, const std::vector <String> &filters) const;

    /**
     * @brief Get a list of imengines which have one or more filters attached.
     *
     * @param imengines The UUIDs of filtered imengines will be stored here.
     * @return How many imengines are being filtered.
     */
    size_t get_filtered_imengines (std::vector <String> &imengines) const;

    /**
     * @brief Create a FilterFactory according to its index.
     *
     * @param idx The index of the filter to be created, must be less than number_of_filters() - 1.
     * @return The pointer of the FilterFactory object.
     */
    FilterFactoryPointer create_filter (unsigned int idx) const;

    /**
     * @brief Create a FilterFactory according to its UUID.
     *
     * @param uuid The UUID of the filter to be created.
     * @return The pointer of the FilterFactory object.
     */
    FilterFactoryPointer create_filter (const String &uuid) const;

    /**
     * @brief Attach all binded Filters to an IMEngineFactory object.
     *
     * @param factory The pointer to an IMEngineFactory object which would be filtered.
     *
     * @return New pointer of IMEngineFactory object which has Filters binded.
     */
    IMEngineFactoryPointer attach_filters_to_factory (const IMEngineFactoryPointer &factory) const;
};

/** @} */

} // namespace scim

#endif //__SCIM_FILTER_MANAGER_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

