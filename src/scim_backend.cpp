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
 * $Id: scim_backend.cpp,v 1.38.2.1 2006/09/24 16:00:52 suzhe Exp $
 *
 */

#define Uses_SCIM_FILTER_MANAGER
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_STL_ALGORITHM
#include <set>
#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

namespace scim {

typedef scim_map <String, IMEngineFactoryPointer>                         IMEngineFactoryRepository;

typedef std::vector <IMEngineFactoryPointer>                                        IMEngineFactoryPointerVector;

class LocaleEqual
{
    String m_lhs;
public:
    LocaleEqual (const String &lhs) : m_lhs (lhs) { }

    bool operator () (const String &rhs) const {
        if (m_lhs == rhs) return true;
        if (scim_get_locale_language (m_lhs) == scim_get_locale_language (rhs) &&
            scim_get_locale_encoding (m_lhs) == scim_get_locale_encoding (rhs) &&
            m_lhs.find ('.') != String::npos && rhs.find ('.') != String::npos)
            return true;
        return false;
    }
};

class IMEngineFactoryPointerLess
{
public:
    bool operator () (const IMEngineFactoryPointer &lhs, const IMEngineFactoryPointer &rhs) const {
        return (lhs->get_language () < rhs->get_language ()) ||
               (lhs->get_language () == rhs->get_language () && lhs->get_name () < rhs->get_name ());
    }
};

class BackEndBase::BackEndBaseImpl
{
    IMEngineFactoryRepository    m_factory_repository;
    String                       m_supported_unicode_locales;
    ConfigPointer                m_config;

public:
    BackEndBaseImpl (const ConfigPointer &config)
        : m_config (config)
    {
        String locales;

        // Set the default supported locales.
        locales = scim_global_config_read (SCIM_GLOBAL_CONFIG_SUPPORTED_UNICODE_LOCALES, String ("en_US.UTF-8"));
 
        std::vector <String> locale_list;
        std::vector <String> real_list;
 
        scim_split_string_list (locale_list, locales);
 
        for (std::vector <String>::iterator i = locale_list.begin (); i!= locale_list.end (); ++i) {
            *i = scim_validate_locale (*i);
            if (i->length () && scim_get_locale_encoding (*i) == "UTF-8" &&
                std::find_if (real_list.begin (), real_list.end (), LocaleEqual (*i)) == real_list.end ())
                real_list.push_back (*i);
        }
 
        m_supported_unicode_locales = scim_combine_string_list (real_list);
    }

    void clear ()
    {
        m_factory_repository.clear ();
    }

    String get_all_locales () const
    {
        String locale;
 
        std::vector <String> locale_list;
        std::vector <String> real_list;
 
        IMEngineFactoryRepository::const_iterator it; 
 
        for (it = m_factory_repository.begin (); it != m_factory_repository.end (); ++it) {
            if (locale.length () == 0)
                locale += it->second->get_locales ();
            else
                locale += (String (",") + it->second->get_locales ());
        }
 
        if (m_supported_unicode_locales.length ())
            locale += (String (",") + m_supported_unicode_locales);
 
        scim_split_string_list (locale_list, locale);
 
        for (std::vector <String>::iterator i = locale_list.begin (); i!= locale_list.end (); i++) {
            locale = scim_validate_locale (*i);
            if (locale.length () &&
                std::find_if (real_list.begin (), real_list.end (), LocaleEqual (locale)) == real_list.end ())
                real_list.push_back (locale);
        }
 
        return scim_combine_string_list (real_list);
    }

    IMEngineFactoryPointer get_factory (const String &uuid) const
    {
        IMEngineFactoryRepository::const_iterator it = m_factory_repository.find (uuid);

        if (it != m_factory_repository.end ())
            return it->second;

        return IMEngineFactoryPointer (0);
    }

    uint32 get_factories_for_encoding (std::vector<IMEngineFactoryPointer> &factories,
                                       const String                        &encoding)  const
    {
        IMEngineFactoryRepository::const_iterator it;
 
        factories.clear ();
 
        for (it = m_factory_repository.begin (); it != m_factory_repository.end (); ++it) {
            if ((encoding.length () == 0 || it->second->validate_encoding (encoding)))
                factories.push_back (it->second);
        }
 
        sort_factories (factories);

        return factories.size ();
    }

    uint32 get_factories_for_language (std::vector<IMEngineFactoryPointer> &factories,
                                       const String                        &language)  const
    {
        IMEngineFactoryRepository::const_iterator it;
 
        factories.clear ();
 
        for (it = m_factory_repository.begin (); it != m_factory_repository.end (); ++it) {
            if ((language.length () == 0 || it->second->get_language () == language))
                factories.push_back (it->second);
        }
 
        sort_factories (factories);

        return factories.size ();
    }

    IMEngineFactoryPointer get_default_factory (const String &language, const String &encoding) const
    {
        if (!language.length ()) return IMEngineFactoryPointer ();

        IMEngineFactoryPointerVector factories;

        if (get_factories_for_encoding (factories, encoding) > 0) {
            IMEngineFactoryPointer lang_first;
            IMEngineFactoryPointerVector::iterator it;

            String def_uuid;
            
            def_uuid = m_config->read (String (SCIM_CONFIG_DEFAULT_IMENGINE_FACTORY) +
                                       String ("/") + language,
                                       String (""));

            // Match by Normalized language exactly.
            for (it = factories.begin (); it != factories.end (); ++it) {
                if (scim_get_normalized_language ((*it)->get_language ()) == language && lang_first.null ())
                    lang_first = *it;

                if ((*it)->get_uuid () == def_uuid)
                    return *it; 
            }

            if (!lang_first.null ()) return lang_first;

            // Match by short language name.
            for (it = factories.begin (); it != factories.end (); ++it)
                if ((*it)->get_language () == language.substr (0,2))
                    return *it;

            return factories [0];
        }

        return IMEngineFactoryPointer ();
    }

    void set_default_factory (const String &language, const String &uuid)
    {
        if (!language.length () || !uuid.length ()) return;

        IMEngineFactoryPointerVector factories;
        if (get_factories_for_encoding (factories, "") > 0) {
            IMEngineFactoryPointerVector::iterator it;
            for (it = factories.begin (); it != factories.end (); ++it) {
                if ((*it)->get_uuid () == uuid) {
                    m_config->write (String (SCIM_CONFIG_DEFAULT_IMENGINE_FACTORY) + String ("/") + language, uuid);
                    return;
                }
            }
        }
    }

    IMEngineFactoryPointer get_next_factory (const String &language, const String &encoding, const String &cur_uuid) const
    {
        IMEngineFactoryPointerVector factories;

        if (get_factories_for_encoding (factories, encoding) > 0) {
            IMEngineFactoryPointer lang_first;
            IMEngineFactoryPointerVector::iterator it, itl;

            for (it = factories.begin (); it != factories.end (); ++it) {
                if ((language.length () == 0 || (*it)->get_language () == language) && lang_first.null ())
                    lang_first = *it;

                if ((*it)->get_uuid () == cur_uuid) {
                    for (itl = it + 1; itl != factories.end (); ++itl) {
                        if (language.length () == 0 || (*itl)->get_language () == language)
                            return *itl;
                    }
                    if (!lang_first.null ()) return lang_first;

                    return factories [0];
                }
            }
        }

        return IMEngineFactoryPointer ();
    }

    IMEngineFactoryPointer get_previous_factory (const String &language, const String &encoding, const String &cur_uuid) const
    {
        IMEngineFactoryPointerVector factories;

        if (get_factories_for_encoding (factories, encoding) > 0) {
            IMEngineFactoryPointer lang_last;
            IMEngineFactoryPointerVector::iterator it, itl;

            for (it = factories.begin (); it != factories.end (); ++it) {
                if ((language.length () == 0 || (*it)->get_language () == language))
                    lang_last = *it;
            }

            for (it = factories.begin (); it != factories.end (); ++it) {
                if ((*it)->get_uuid () == cur_uuid) {
                    for (itl = it; itl != factories.begin (); --itl) {
                        if (language.length () == 0 || (*(itl-1))->get_language () == language)
                            return *(itl-1);
                    }

                    if (!lang_last.null ()) return lang_last;

                    return factories [factories.size () - 1];
                }
            }
        }

        return IMEngineFactoryPointer ();
    }

    bool add_factory (const IMEngineFactoryPointer &factory)
    {
        if (!factory.null ()) {
            String uuid = factory->get_uuid ();

            if (uuid.length () && m_factory_repository.find (uuid) == m_factory_repository.end ()) {
                m_factory_repository [uuid] = factory;
                return true;
            }
        }

        return false;
    }

    bool remove_factory (const String &uuid)
    {
        IMEngineFactoryRepository::iterator it = m_factory_repository.find (uuid);

        if (it != m_factory_repository.end ()) {
            m_factory_repository.erase (it);
            return true;
        }

        return false;
    }

private:
    void sort_factories (std::vector<IMEngineFactoryPointer> &factories) const
    {
        std::sort (factories.begin (), factories.end (), IMEngineFactoryPointerLess ());
    }
};

BackEndBase::BackEndBase (const ConfigPointer &config)
    : m_impl (new BackEndBase::BackEndBaseImpl (config))
{
}

BackEndBase::~BackEndBase ()
{
    delete m_impl;
}

String
BackEndBase::get_all_locales () const
{
    return m_impl->get_all_locales ();
}

IMEngineFactoryPointer
BackEndBase::get_factory (const String &uuid) const
{
    return m_impl->get_factory (uuid);
}

uint32
BackEndBase::get_factories_for_encoding (std::vector<IMEngineFactoryPointer> &factories, const String &encoding) const
{
    return m_impl->get_factories_for_encoding (factories, encoding);
}

uint32
BackEndBase::get_factories_for_language (std::vector<IMEngineFactoryPointer> &factories, const String &language) const
{
    return m_impl->get_factories_for_language (factories, language);
}

IMEngineFactoryPointer
BackEndBase::get_default_factory (const String &language, const String &encoding) const
{
    return m_impl->get_default_factory (language, encoding);
}

void
BackEndBase::set_default_factory (const String &language, const String &uuid)
{
    m_impl->set_default_factory (language, uuid);
}

IMEngineFactoryPointer
BackEndBase::get_next_factory (const String &language, const String &encoding, const String &cur_uuid) const
{
    return m_impl->get_next_factory (language, encoding, cur_uuid);
}

IMEngineFactoryPointer
BackEndBase::get_previous_factory (const String &language, const String &encoding, const String &cur_uuid) const
{
    return m_impl->get_previous_factory (language, encoding, cur_uuid);
}

bool
BackEndBase::add_factory (const IMEngineFactoryPointer &factory)
{
    return m_impl->add_factory (factory);
}

bool
BackEndBase::remove_factory (const String &uuid)
{
    return m_impl->remove_factory (uuid);
}

void
BackEndBase::clear ()
{
    m_impl->clear ();
}

// Implementation of CommonBackEnd.
struct CommonBackEnd::CommonBackEndImpl {
    IMEngineModule      *m_engine_modules;
    FilterManager       *m_filter_manager;
    ConfigPointer        m_config;         // kept for on-demand module reload
    std::vector<String>  m_modules;        // module names, indexes m_engine_modules
    // UUIDs served by the socket proxy: the remote side has already applied its
    // own filters, so reload_filters () must leave these alone.
    std::set<String>     m_unfiltered;

    CommonBackEndImpl () : m_engine_modules (0), m_filter_manager (0) { }
};

CommonBackEnd::CommonBackEnd (const ConfigPointer       &config,
                              const std::vector<String> &modules)
    : BackEndBase (config),
      m_impl (new CommonBackEndImpl)
{
    IMEngineFactoryPointer factory;
    std::vector<String>    disabled_factories;
    std::vector<String>    new_modules = modules;

    int all_factories_count = 0;
    int module_factories_count = 0;

    if (config.null ()) return;

    // Get disabled factories list.
    disabled_factories = scim_global_config_read (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES, disabled_factories);

    // Put socket module to the end of list.
    for (std::vector<String>::iterator it = new_modules.begin (); it != new_modules.end (); ++it) {
        if (*it == "socket") {
            new_modules.erase (it);
            new_modules.push_back ("socket");
            break;
        }
    }

    // Try to load all IMEngine modules
    try {
        m_impl->m_engine_modules = new IMEngineModule [new_modules.size ()];
        m_impl->m_filter_manager = new FilterManager (config);
    } catch (const std::exception & err) {
        std::cerr << err.what () << "\n";
        return;
    }

    // Keep what reload_disabled_factories () needs to re-apply the disabled
    // list later: the config (to reload an unloaded module) and the module
    // names (same indexing as m_engine_modules).
    m_impl->m_config  = config;
    m_impl->m_modules = new_modules;

    //load IMEngine modules
    for (size_t i = 0; i < new_modules.size (); ++i) {
        SCIM_DEBUG_BACKEND (1) << "Loading IMEngine module: " << new_modules [i] << " ...\n";

        module_factories_count = 0;

        if (m_impl->m_engine_modules [i].load (new_modules [i], config) &&
            m_impl->m_engine_modules [i].valid ()) {
            for (size_t j=0; j < m_impl->m_engine_modules [i].number_of_factories (); ++j) {

                // Try to load a IMEngine Factory.
                try {
                    factory = m_impl->m_engine_modules [i].create_factory (j);
                } catch (const std::exception & err) {
                    std::cerr << err.what () << "\n";
                    factory.reset ();
                }

                if (!factory.null ()) {
                    // Check if it's disabled.
                    if (std::find (disabled_factories.begin (),
                                   disabled_factories.end (),
                                   factory->get_uuid ()) == disabled_factories.end ()) {

                        // Add it into disabled list to prevent from loading again.
                        disabled_factories.push_back (factory->get_uuid ());

                        // Only load filter for none socket IMEngines.
                        if (new_modules [i] != "socket")
                            factory = m_impl->m_filter_manager->attach_filters_to_factory (factory);
                        else
                            m_impl->m_unfiltered.insert (factory->get_uuid ());

                        add_factory (factory);

                        all_factories_count ++;
                        module_factories_count ++;

                        SCIM_DEBUG_BACKEND (1) << "    Loading IMEngine Factory " << j << " : " << "OK\n";
                    } else {
                        SCIM_DEBUG_BACKEND (1) << "    Loading IMEngine Factory " << j << " : " << "Disabled\n";
                        factory.reset ();
                    }
                } else {
                    SCIM_DEBUG_BACKEND (1) << "    Loading IMEngine Factory " << j << " : " << "Failed\n";
                }
            }
            if (module_factories_count) {
                SCIM_DEBUG_BACKEND (1) << new_modules [i] << " IMEngine module is successfully loaded.\n";
            } else {
                SCIM_DEBUG_BACKEND (1) << "No Factory loaded from " << new_modules [i] << " IMEngine module!\n";
                m_impl->m_engine_modules [i].unload ();
            }
        } else {
            SCIM_DEBUG_BACKEND (1) << "Failed to load " << new_modules [i] << " IMEngine module.\n";
        }
    }

    factory = new ComposeKeyFactory ();

    if (all_factories_count == 0 ||
        std::find (disabled_factories.begin (),
                   disabled_factories.end (),
                   factory->get_uuid ()) == disabled_factories.end ()) {
        factory = m_impl->m_filter_manager->attach_filters_to_factory (factory);
        add_factory (factory);
    }
}

CommonBackEnd::~CommonBackEnd ()
{
    clear ();

    delete [] m_impl->m_engine_modules;
    delete m_impl->m_filter_manager;
    delete m_impl;
}

// Re-apply the filter configuration to factories that are already loaded.
//
// Filters are wrapped around a factory when the factory is created, so a
// changed /Filter/FilteredIMEngines list would otherwise only reach factories
// loaded afterwards. Peel the existing chain off, rebuild it from the current
// configuration and swap the factory in the repository.
//
// As with reload_disabled_factories (), existing IMEngine instances keep the
// chain they were created with; the change applies to instances created later.
void
CommonBackEnd::reload_filters ()
{
    if (!m_impl || m_impl->m_filter_manager == 0)
        return;

    std::vector<IMEngineFactoryPointer> current;
    get_factories_for_encoding (current, String ());

    for (size_t i = 0; i < current.size (); ++i) {
        IMEngineFactoryPointer factory = current [i];

        if (factory.null ())
            continue;

        const String uuid = factory->get_uuid ();

        // The socket proxy is filtered on the far side; wrapping it here would
        // apply every filter twice.
        if (m_impl->m_unfiltered.count (uuid))
            continue;

        // Peel off however many filters are currently wrapped around it.
        IMEngineFactoryPointer base = factory;
        for (;;) {
            IMEngineFactoryBase *raw = base;
            FilterFactoryBase   *ff  = dynamic_cast <FilterFactoryBase *> (raw);

            if (!ff)
                break;

            IMEngineFactoryPointer inner = ff->get_attached_factory ();

            if (inner.null ())
                break;

            base = inner;
        }

        IMEngineFactoryPointer rebuilt =
            m_impl->m_filter_manager->attach_filters_to_factory (base);

        // Unchanged when the engine has no filters configured: attach_filters_
        // to_factory () hands the original straight back, so there is nothing
        // to swap and no churn on engines nobody filters.
        IMEngineFactoryBase *before = factory;
        IMEngineFactoryBase *after  = rebuilt;

        if (before == after)
            continue;

        remove_factory (uuid);
        add_factory (rebuilt);
    }
}

void
CommonBackEnd::reload_disabled_factories ()
{
    if (!m_impl || !m_impl->m_engine_modules)
        return;

    // Pick up changes written by another process (e.g. scim-setup) and fetch
    // the fresh disabled-factory list.
    scim_global_config_reload ();

    std::vector<String> disabled;
    disabled = scim_global_config_read (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES, disabled);

    // Disable phase: drop any currently-loaded factory that is now disabled.
    std::vector<IMEngineFactoryPointer> current;
    get_factories_for_encoding (current, String ());
    for (size_t i = 0; i < current.size (); ++i) {
        String uuid = current [i]->get_uuid ();
        if (std::find (disabled.begin (), disabled.end (), uuid) != disabled.end ())
            remove_factory (uuid);
    }

    // Enable phase: add any installed-but-newly-enabled factory.
    IMEngineFactoryPointer factory;
    for (size_t i = 0; i < m_impl->m_modules.size (); ++i) {
        const String &name = m_impl->m_modules [i];

        // A module whose factories were all disabled at load time was
        // unloaded; reload it on demand so a now-enabled factory can serve.
        bool was_loaded = m_impl->m_engine_modules [i].valid ();
        if (!was_loaded)
            m_impl->m_engine_modules [i].load (name, m_impl->m_config);

        if (!m_impl->m_engine_modules [i].valid ())
            continue;

        int added = 0;
        for (size_t j = 0; j < m_impl->m_engine_modules [i].number_of_factories (); ++j) {
            try {
                factory = m_impl->m_engine_modules [i].create_factory (j);
            } catch (const std::exception & err) {
                std::cerr << err.what () << "\n";
                factory.reset ();
            }

            if (factory.null ())
                continue;

            String uuid = factory->get_uuid ();

            // Skip if disabled, or already loaded (add_factory would no-op but
            // avoid re-wrapping/re-creating an active factory).
            if (std::find (disabled.begin (), disabled.end (), uuid) != disabled.end () ||
                !get_factory (uuid).null ()) {
                factory.reset ();
                continue;
            }

            // Only load filter for none socket IMEngines.
            if (name != "socket")
                factory = m_impl->m_filter_manager->attach_filters_to_factory (factory);
            else
                m_impl->m_unfiltered.insert (uuid);

            if (add_factory (factory))
                ++added;
        }

        // If we freshly loaded this module for the reload but nothing came of
        // it, drop it again (matches constructor behavior). Never unload a
        // module that was already resident: live IMEngine instances may still
        // reference its factories.
        if (!was_loaded && added == 0)
            m_impl->m_engine_modules [i].unload ();
    }

    // Keep the built-in Compose key factory in sync. It is always kept as a
    // fallback when no other factory is enabled (mirrors the constructor).
    {
        IMEngineFactoryPointer compose = new ComposeKeyFactory ();
        String compose_uuid = compose->get_uuid ();

        std::vector<IMEngineFactoryPointer> loaded;
        get_factories_for_encoding (loaded, String ());
        size_t non_compose = 0;
        for (size_t i = 0; i < loaded.size (); ++i)
            if (loaded [i]->get_uuid () != compose_uuid)
                ++non_compose;

        bool compose_disabled =
            std::find (disabled.begin (), disabled.end (), compose_uuid) != disabled.end ();

        if (non_compose == 0 || !compose_disabled) {
            if (get_factory (compose_uuid).null ()) {
                compose = m_impl->m_filter_manager->attach_filters_to_factory (compose);
                add_factory (compose);
            }
        } else {
            remove_factory (compose_uuid);
        }
    }
}


} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
