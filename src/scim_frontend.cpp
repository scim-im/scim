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
 * $Id: scim_frontend.cpp,v 1.44 2005/06/26 16:35:33 suzhe Exp $
 *
 */

#define Uses_SCIM_FRONTEND
#include "scim_private.h"
#include "scim.h"

namespace scim {

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <int, IMEngineInstancePointer, __gnu_cxx::hash <int> >  IMEngineInstanceRepository;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <int, IMEngineInstancePointer, std::::hash <int> >            IMEngineInstanceRepository;
#else
typedef std::map <int, IMEngineInstancePointer>                                     IMEngineInstanceRepository;
#endif

class FrontEndBase::FrontEndBaseImpl
{
public:
    FrontEndBase               *m_frontend;
    BackEndPointer              m_backend;
    IMEngineInstanceRepository  m_instance_repository;

    int                         m_instance_count;
public:
    FrontEndBaseImpl (FrontEndBase *fe, const BackEndPointer &backend)
        :m_frontend (fe),
         m_backend (backend),
         m_instance_count (0)
    { }

    IMEngineInstancePointer find_instance (int id) const {
        IMEngineInstanceRepository::const_iterator it = m_instance_repository.find (id);
        if (it != m_instance_repository.end ())
            return it->second;
        return IMEngineInstancePointer (0);
    }

    void slot_show_preedit_string   (IMEngineInstanceBase * si) {
        m_frontend->show_preedit_string (si->get_id ());
    }

    void slot_show_aux_string       (IMEngineInstanceBase * si) {
        m_frontend->show_aux_string (si->get_id ());
    }

    void slot_show_lookup_table     (IMEngineInstanceBase * si) {
        m_frontend->show_lookup_table (si->get_id ());
    }

    void slot_hide_preedit_string   (IMEngineInstanceBase * si) {
        m_frontend->hide_preedit_string (si->get_id ());
    }

    void slot_hide_aux_string       (IMEngineInstanceBase * si) {
        m_frontend->hide_aux_string (si->get_id ());
    }

    void slot_hide_lookup_table     (IMEngineInstanceBase * si) {
        m_frontend->hide_lookup_table (si->get_id ());
    }

    void slot_update_preedit_caret  (IMEngineInstanceBase * si, int caret) {
        m_frontend->update_preedit_caret (si->get_id (), caret);
    }

    void slot_update_preedit_string (IMEngineInstanceBase * si, const WideString & str, const AttributeList & attrs) {
        m_frontend->update_preedit_string (si->get_id (), str, attrs);
    }

    void slot_update_aux_string     (IMEngineInstanceBase * si, const WideString & str, const AttributeList & attrs) {
        m_frontend->update_aux_string (si->get_id (), str, attrs);
    }

    void slot_update_lookup_table   (IMEngineInstanceBase * si, const LookupTable & table) {
        m_frontend->update_lookup_table (si->get_id (), table);
    }

    void slot_commit_string         (IMEngineInstanceBase * si, const WideString & str) {
        m_frontend->commit_string (si->get_id (), str);
    }

    void slot_forward_key_event     (IMEngineInstanceBase * si, const KeyEvent & key) {
        m_frontend->forward_key_event (si->get_id (), key);
    }

    void slot_register_properties   (IMEngineInstanceBase * si, const PropertyList & properties) {
        m_frontend->register_properties (si->get_id (), properties);
    }

    void slot_update_property       (IMEngineInstanceBase * si, const Property & property) {
        m_frontend->update_property (si->get_id (), property);
    }

    void slot_beep                  (IMEngineInstanceBase * si) {
        m_frontend->beep (si->get_id ());
    }

    void slot_start_helper          (IMEngineInstanceBase * si, const String & helper_uuid) {
        m_frontend->start_helper (si->get_id (), helper_uuid);
    }

    void slot_stop_helper           (IMEngineInstanceBase * si, const String & helper_uuid) {
        m_frontend->stop_helper (si->get_id (), helper_uuid);
    }

    void slot_send_helper_event     (IMEngineInstanceBase * si, const String & helper_uuid, const Transaction & trans) {
        m_frontend->send_helper_event (si->get_id (), helper_uuid, trans);
    }

    bool slot_get_surrounding_text  (IMEngineInstanceBase * si, WideString &text, int &cursor, int maxlen_before, int maxlen_after) {
        return m_frontend->get_surrounding_text (si->get_id (), text, cursor, maxlen_before, maxlen_after);
    }

    bool slot_delete_surrounding_text(IMEngineInstanceBase * si, int offset, int len) {
        return m_frontend->delete_surrounding_text (si->get_id (), offset, len);
    }

    void attach_instance (const IMEngineInstancePointer &si)
    {
        si->signal_connect_show_preedit_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_show_preedit_string));
        si->signal_connect_show_aux_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_show_aux_string));
        si->signal_connect_show_lookup_table (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_show_lookup_table));
 
        si->signal_connect_hide_preedit_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_hide_preedit_string));
        si->signal_connect_hide_aux_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_hide_aux_string));
        si->signal_connect_hide_lookup_table (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_hide_lookup_table));
 
        si->signal_connect_update_preedit_caret (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_update_preedit_caret));
        si->signal_connect_update_preedit_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_update_preedit_string));
        si->signal_connect_update_aux_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_update_aux_string));
        si->signal_connect_update_lookup_table (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_update_lookup_table));
 
        si->signal_connect_commit_string (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_commit_string));
 
        si->signal_connect_forward_key_event (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_forward_key_event));
 
        si->signal_connect_register_properties (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_register_properties));
 
        si->signal_connect_update_property (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_update_property));
 
        si->signal_connect_beep (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_beep));
 
        si->signal_connect_start_helper (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_start_helper));
 
        si->signal_connect_stop_helper (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_stop_helper));
 
        si->signal_connect_send_helper_event (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_send_helper_event));
 
        si->signal_connect_get_surrounding_text (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_get_surrounding_text));
 
        si->signal_connect_delete_surrounding_text (
            slot (this, &FrontEndBase::FrontEndBaseImpl::slot_delete_surrounding_text));
    }
};

FrontEndBase::FrontEndBase (const BackEndPointer &backend)
    : m_impl (new FrontEndBaseImpl (this, backend))
{
}

FrontEndBase::~FrontEndBase ()
{
    delete m_impl;
}

uint32
FrontEndBase::get_factory_list_for_encoding (std::vector<String>& uuids, const String &encoding) const
{
    std::vector<IMEngineFactoryPointer> factories;

    m_impl->m_backend->get_factories_for_encoding (factories, encoding);

    uuids.clear ();

    for (std::vector<IMEngineFactoryPointer>::iterator it = factories.begin (); it != factories.end (); ++it)
        uuids.push_back ((*it)->get_uuid ());

    return uuids.size ();
}

uint32
FrontEndBase::get_factory_list_for_language (std::vector<String>& uuids, const String &language) const
{
    std::vector<IMEngineFactoryPointer> factories;

    m_impl->m_backend->get_factories_for_language (factories, language);

    uuids.clear ();

    for (std::vector<IMEngineFactoryPointer>::iterator it = factories.begin (); it != factories.end (); ++it)
        uuids.push_back ((*it)->get_uuid ());

    return uuids.size ();
}

String
FrontEndBase::get_default_factory (const String &language, const String &encoding) const
{
    IMEngineFactoryPointer def = m_impl->m_backend->get_default_factory (language, encoding);
    if (!def.null ()) return def->get_uuid ();
    return String ();
}

void
FrontEndBase::set_default_factory (const String &language, const String &uuid)
{
    m_impl->m_backend->set_default_factory (language, uuid);
}

String
FrontEndBase::get_next_factory (const String &language, const String &encoding, const String &cur_uuid) const
{
    IMEngineFactoryPointer next = m_impl->m_backend->get_next_factory (language, encoding, cur_uuid);
    if (!next.null ()) return next->get_uuid ();
    return String ();
}

String
FrontEndBase::get_previous_factory (const String &language, const String &encoding, const String &cur_uuid) const
{
    IMEngineFactoryPointer prev = m_impl->m_backend->get_previous_factory (language, encoding, cur_uuid);
    if (!prev.null ()) return prev->get_uuid ();
    return String ();
}

WideString
FrontEndBase::get_factory_name (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_name ();
    return WideString ();
}

WideString
FrontEndBase::get_factory_authors (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_authors ();
    return WideString ();
}

WideString
FrontEndBase::get_factory_credits (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_credits ();
    return WideString ();
}

WideString
FrontEndBase::get_factory_help (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_help ();
    return WideString ();
}

String
FrontEndBase::get_factory_locales (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_locales ();
    return String ();
}

String
FrontEndBase::get_factory_language (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_language ();
    return String ();
}

String
FrontEndBase::get_factory_icon_file (const String &uuid) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    if (!factory.null ()) return factory->get_icon_file ();
    return String ();
}

bool
FrontEndBase::validate_factory (const String &uuid, const String &encoding) const
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (uuid);
    return !factory.null () && (encoding.length () == 0 || factory->validate_encoding (encoding));
}

String
FrontEndBase::get_all_locales () const
{
    return m_impl->m_backend->get_all_locales ();
}

int
FrontEndBase::new_instance (const String &sf_uuid, const String& encoding)
{
    IMEngineFactoryPointer factory = m_impl->m_backend->get_factory (sf_uuid);

    if (factory.null () || !factory->validate_encoding (encoding)) {
        SCIM_DEBUG_FRONTEND (1) << "IMEngineFactory " << sf_uuid
            << " does not support encoding " << encoding << "\n";
        return -1;
    }

    IMEngineInstancePointer si =
        factory->create_instance (encoding, m_impl->m_instance_count);

    if (si.null ()) {
        SCIM_DEBUG_BACKEND(1) << "IMEngineFactory " << sf_uuid
            << " failed to create new instance!\n";
        return -1;
    }

    ++ m_impl->m_instance_count;

    if (m_impl->m_instance_count < 0)
        m_impl->m_instance_count = 0;

    m_impl->m_instance_repository [si->get_id ()] = si;

    m_impl->attach_instance (si);

    return si->get_id ();
}

bool
FrontEndBase::replace_instance (int si_id, const String &sf_uuid)
{
    IMEngineFactoryPointer sf = m_impl->m_backend->get_factory (sf_uuid);

    if (sf.null ()) return false;

    IMEngineInstanceRepository::iterator it = m_impl->m_instance_repository.find (si_id);

    if (it != m_impl->m_instance_repository.end ()) {
        if (it->second->get_factory_uuid () == sf_uuid)
            return true;

        String encoding = it->second->get_encoding ();
        if (sf->validate_encoding (encoding)) {
            IMEngineInstancePointer si = sf->create_instance (encoding, si_id);
            if (!si.null ()) {
                it->second = si;
                m_impl->attach_instance (it->second);
                return true;
            }
        }
    }

    SCIM_DEBUG_BACKEND(1) << "Cannot find IMEngine Instance " << si_id << " to replace.\n";

    return false;
}

bool
FrontEndBase::delete_instance (int id)
{
    IMEngineInstanceRepository::iterator it = m_impl->m_instance_repository.find (id);

    if (it != m_impl->m_instance_repository.end ()) {
        m_impl->m_instance_repository.erase (it);
        return true;
    }

    return false;
}

void
FrontEndBase::delete_all_instances ()
{
    m_impl->m_instance_repository.clear ();
}

String
FrontEndBase::get_instance_uuid (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return si->get_factory_uuid ();

    return String ();
}

String
FrontEndBase::get_instance_encoding (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return si->get_encoding ();

    return String ();
}

WideString
FrontEndBase::get_instance_name (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return get_factory_name (si->get_factory_uuid ());

    return WideString ();
}

WideString
FrontEndBase::get_instance_authors (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return get_factory_authors (si->get_factory_uuid ());

    return WideString ();
}

WideString
FrontEndBase::get_instance_credits (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return get_factory_credits (si->get_factory_uuid ());

    return WideString ();
}

WideString
FrontEndBase::get_instance_help (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return get_factory_help (si->get_factory_uuid ());

    return WideString ();
}

String
FrontEndBase::get_instance_icon_file (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return get_factory_icon_file (si->get_factory_uuid ());

    return String ();
}

void
FrontEndBase::focus_in (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->focus_in ();
}

void
FrontEndBase::focus_out (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->focus_out ();
}

void
FrontEndBase::reset (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->reset ();
}

bool
FrontEndBase::process_key_event (int id, const KeyEvent& key) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) return si->process_key_event (key);

    return false;
}

void
FrontEndBase::move_preedit_caret (int id, unsigned int pos) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->move_preedit_caret (pos);
}

void
FrontEndBase::select_candidate (int id, unsigned int index) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->select_candidate (index);
}

void
FrontEndBase::update_lookup_table_page_size (int id, unsigned int page_size) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->update_lookup_table_page_size (page_size);
}

void
FrontEndBase::lookup_table_page_up (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->lookup_table_page_up ();
}

void
FrontEndBase::lookup_table_page_down (int id) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->lookup_table_page_down ();
}

void
FrontEndBase::trigger_property (int id, const String & property) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->trigger_property (property);
}

void
FrontEndBase::process_helper_event (int id, const String & helper_uuid, const Transaction & trans) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->process_helper_event (helper_uuid, trans);
}

void
FrontEndBase::update_client_capabilities (int id, unsigned int cap) const
{
    IMEngineInstancePointer si = m_impl->find_instance (id);

    if (!si.null ()) si->update_client_capabilities (cap);
}

void
FrontEndBase::show_preedit_string (int id)
{
}
void
FrontEndBase::show_aux_string     (int id)
{
}
void
FrontEndBase::show_lookup_table   (int id)
{
}
void
FrontEndBase::hide_preedit_string (int id)
{
}
void
FrontEndBase::hide_aux_string     (int id)
{
}
void
FrontEndBase::hide_lookup_table   (int id)
{
}
void
FrontEndBase::update_preedit_caret  (int id, int caret)
{
}
void
FrontEndBase::update_preedit_string (int id, const WideString & str, const AttributeList & attrs)
{
}
void
FrontEndBase::update_aux_string     (int id, const WideString & str, const AttributeList & attrs)
{
}
void
FrontEndBase::update_lookup_table   (int id, const LookupTable & table)
{
}
void
FrontEndBase::commit_string         (int id, const WideString & str)
{
}
void
FrontEndBase::forward_key_event      (int id, const KeyEvent & key)
{
}
void
FrontEndBase::register_properties   (int id, const PropertyList & properties)
{
}
void
FrontEndBase::update_property       (int id, const Property & property)
{
}
void
FrontEndBase::beep                  (int id)
{
}
void
FrontEndBase::start_helper          (int id, const String &helper_uuid)
{
}
void
FrontEndBase::stop_helper           (int id, const String &helper_uuid)
{
}
void
FrontEndBase::send_helper_event     (int id, const String &helper_uuid, const Transaction &trans)
{
}
bool
FrontEndBase::get_surrounding_text  (int id, WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    return false;
}
bool
FrontEndBase::delete_surrounding_text  (int id, int offset, int len)
{
    return false;
}

} // namespace scim

/*
vi:ts=4:ai:nowrap:expandtab
*/
