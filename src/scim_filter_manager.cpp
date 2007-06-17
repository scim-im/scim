/** @file scim_filter_manager.cpp
 *  @brief Implementation of class FilterManager.
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
 * $Id: scim_filter_manager.cpp,v 1.4 2005/12/16 11:12:27 suzhe Exp $
 */

#define Uses_SCIM_FILTER_MODULE
#define Uses_SCIM_FILTER_MANAGER
#define Uses_SCIM_CONFIG_PATH
#include "scim_private.h"
#include "scim.h"

namespace scim {

struct FilterModuleIndex
{
    FilterModule *module;
    unsigned int  index;

    FilterModuleIndex () : module (0), index (0) { }
};

static bool                     __filter_initialized = false;
static unsigned int             __number_of_modules = 0;
static FilterModule            *__filter_modules    = 0;

static std::vector <std::pair <FilterModuleIndex, FilterInfo> > __filter_infos;

static void
__initialize_modules (const ConfigPointer &config)
{
    if (__filter_initialized) return;

    __filter_initialized = true;

    std::vector <String> mod_list;

    __number_of_modules = scim_get_filter_module_list (mod_list);

    if (!__number_of_modules) return;

    __filter_modules = new FilterModule [__number_of_modules];

    unsigned int i, j;

    for (i = 0; i < __number_of_modules; ++i) {
        if (__filter_modules [i].load (mod_list [i], config)) {
            for (j = 0; j < __filter_modules [i].number_of_filters (); ++j) {
                FilterModuleIndex index;
                FilterInfo info;
                if (__filter_modules [i].get_filter_info (j, info)) {
                    index.module = & __filter_modules [i];
                    index.index = j;
                    __filter_infos.push_back (std::make_pair (index, info));
                }
            }
        }
    }
}

class FilterManager::FilterManagerImpl
{
public:
    ConfigPointer        m_config;

    FilterManagerImpl (const ConfigPointer &config) : m_config (config) { }
};

FilterManager::FilterManager (const ConfigPointer &config)
    : m_impl (new FilterManagerImpl (config))
{
}

FilterManager::~FilterManager ()
{
    delete m_impl;
}

unsigned int
FilterManager::number_of_filters () const
{
    if (!__filter_initialized) __initialize_modules (m_impl->m_config);

    return __filter_infos.size ();
}

bool
FilterManager::get_filter_info (unsigned int idx, FilterInfo &info) const
{
    if (!__filter_initialized) __initialize_modules (m_impl->m_config);

    if (idx < number_of_filters ()) {
        info = __filter_infos [idx].second;
        return true;
    }
    return false;
}

bool
FilterManager::get_filter_info (const String &uuid, FilterInfo &info) const
{
    if (!__filter_initialized) __initialize_modules (m_impl->m_config);

    for (size_t i = 0; i < __filter_infos.size (); ++i) {
        if (__filter_infos [i].second.uuid == uuid) {
            info = __filter_infos [i].second;
            return true;
        }
    }

    return false;
}

FilterFactoryPointer
FilterManager::create_filter (unsigned int idx) const
{
    if (!__filter_initialized) __initialize_modules (m_impl->m_config);

    if (idx < __filter_infos.size () && __filter_infos [idx].first.module && __filter_infos [idx].first.module->valid ()) {
        return __filter_infos [idx].first.module->create_filter (__filter_infos [idx].first.index);
    }

    return FilterFactoryPointer (0);
}

FilterFactoryPointer
FilterManager::create_filter (const String &uuid) const
{
    if (!__filter_initialized) __initialize_modules (m_impl->m_config);

    for (size_t i = 0; i < __filter_infos.size (); ++i) {
        if (__filter_infos [i].second.uuid == uuid && __filter_infos [i].first.module && __filter_infos [i].first.module->valid ())
            return __filter_infos [i].first.module->create_filter (__filter_infos [i].first.index);
    }

    return FilterFactoryPointer (0);
}

void
FilterManager::clear_all_filter_settings () const
{
    if (!m_impl->m_config.null () && m_impl->m_config->valid ()) {
        std::vector <String> tmp;
        scim_split_string_list (tmp, m_impl->m_config->read (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES_LIST), String ("")));

        for (size_t i = 0; i < tmp.size (); ++i)
            m_impl->m_config->erase (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES) + String ("/") + tmp [i]);

        m_impl->m_config->erase (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES_LIST));
    }
}

size_t
FilterManager::get_filters_for_imengine (const String &uuid, std::vector <String> &filters) const
{
    filters.clear ();

    if (!m_impl->m_config.null () && m_impl->m_config->valid ()) {
        std::vector <String> tmp;
        scim_split_string_list (tmp, m_impl->m_config->read (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES_LIST), String ("")));

        if (std::find (tmp.begin (), tmp.end (), uuid) != tmp.end ()) {
            FilterInfo info;

            scim_split_string_list (tmp, m_impl->m_config->read (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES) + String ("/") + uuid, String ("")));

            for (size_t i = 0; i < tmp.size (); ++i) {
                if (std::find (filters.begin (), filters.end (), tmp [i]) == filters.end () && get_filter_info (tmp [i], info))
                    filters.push_back (tmp [i]);
            }
        }
    }

    return filters.size ();
}

void
FilterManager::set_filters_for_imengine (const String &uuid, const std::vector <String> &filters) const
{
    if (!m_impl->m_config.null () && m_impl->m_config->valid ()) {
        std::vector <String> valid_filters;

        FilterInfo info;

        for (size_t i = 0; i < filters.size (); ++i) {
            if (std::find (valid_filters.begin (), valid_filters.end (), filters [i]) == valid_filters.end () && get_filter_info (filters [i], info))
                valid_filters.push_back (filters [i]);
        }

        std::vector <String> filtered_ims;
        scim_split_string_list (filtered_ims, m_impl->m_config->read (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES_LIST), String ("")));

        if (valid_filters.size ()) {
            if (std::find (filtered_ims.begin (), filtered_ims.end (), uuid) == filtered_ims.end ())
                filtered_ims.push_back (uuid);
            m_impl->m_config->write (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES) + String ("/") + uuid, scim_combine_string_list (valid_filters));
        } else {
            std::vector <String>::iterator it = std::find (filtered_ims.begin (), filtered_ims.end (), uuid);
            if (it != filtered_ims.end ())
                filtered_ims.erase (it);
            m_impl->m_config->erase (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES) + String ("/") + uuid);
        }

        m_impl->m_config->write (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES_LIST), scim_combine_string_list (filtered_ims));
    }
}

size_t
FilterManager::get_filtered_imengines (std::vector <String> &imengines) const
{
    scim_split_string_list (imengines, m_impl->m_config->read (String (SCIM_CONFIG_FILTER_FILTERED_IMENGINES_LIST), String ("")));
    return imengines.size ();
}

IMEngineFactoryPointer
FilterManager::attach_filters_to_factory (const IMEngineFactoryPointer &factory) const
{
    IMEngineFactoryPointer root = factory;

    std::vector <String> filters;

    if (!factory.null () && get_filters_for_imengine (factory->get_uuid (), filters)) {
        for (size_t i = 0; i < filters.size (); ++i) {
            FilterFactoryPointer filter = create_filter (filters [i]);
            if (!filter.null ()) {
                filter->attach_imengine_factory (root);
                root = filter;
            }
        }
    }

    return root;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/

