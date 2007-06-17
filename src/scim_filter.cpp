/** @file scim_filter.cpp
 *  @brief Implementation of class FilterFactoryBase and FilterInstanceBase.
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
 * $Id: scim_filter.cpp,v 1.4 2005/05/16 01:25:46 suzhe Exp $
 *
 */

#define Uses_SCIM_FILTER

#include "scim_private.h"
#include "scim.h"

namespace scim {

FilterFactoryBase::FilterFactoryBase ()
{
}

FilterFactoryBase::~FilterFactoryBase ()
{
}

void
FilterFactoryBase::attach_imengine_factory (const IMEngineFactoryPointer &orig)
{
    m_orig = orig;

    if (!m_orig.null ())
        set_locales (m_orig->get_locales ());
    else
        set_locales ("");
}

WideString
FilterFactoryBase::get_name () const
{
    return m_orig.null () ? WideString () : m_orig->get_name ();
}

String
FilterFactoryBase::get_uuid () const
{
    return m_orig.null () ? String () : m_orig->get_uuid ();
}

String
FilterFactoryBase::get_icon_file () const
{
    return m_orig.null () ? String () : m_orig->get_icon_file ();
}

WideString
FilterFactoryBase::get_authors () const
{
    return m_orig.null () ? WideString () : m_orig->get_authors ();
}

WideString
FilterFactoryBase::get_credits () const
{
    return m_orig.null () ? WideString () : m_orig->get_credits ();
}

WideString
FilterFactoryBase::get_help () const
{
    return m_orig.null () ? WideString () : m_orig->get_help ();
}

String
FilterFactoryBase::get_language () const
{
    return m_orig.null () ? IMEngineFactoryBase::get_language () : m_orig->get_language ();
}

bool
FilterFactoryBase::validate_encoding (const String &encoding) const
{
    return m_orig.null () ? IMEngineFactoryBase::validate_encoding (encoding) : m_orig->validate_encoding (encoding);
}

bool
FilterFactoryBase::validate_locale (const String &locale) const
{
    return m_orig.null () ? IMEngineFactoryBase::validate_locale (locale) : m_orig->validate_locale (locale);
}

WideString
FilterFactoryBase::inverse_query (const WideString &str)
{
    return m_orig.null () ? WideString () : m_orig->inverse_query (str);
}

IMEngineInstancePointer
FilterFactoryBase::create_instance (const String& encoding, int id)
{
    return m_orig.null () ? IMEngineInstancePointer (0) : m_orig->create_instance (encoding, id);
}

class FilterInstanceBase::FilterInstanceBaseImpl
{
    FilterInstanceBase      *m_parent;
    IMEngineInstancePointer  m_orig;

public:
    FilterInstanceBaseImpl (FilterInstanceBase *parent, const IMEngineInstancePointer &orig)
        : m_parent (parent),
          m_orig (orig)
    {
        if (!m_orig.null ()) {
            m_orig->signal_connect_show_preedit_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_show_preedit_string));
            m_orig->signal_connect_show_aux_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_show_aux_string));
            m_orig->signal_connect_show_lookup_table (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_show_lookup_table));

            m_orig->signal_connect_hide_preedit_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_hide_preedit_string));
            m_orig->signal_connect_hide_aux_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_hide_aux_string));
            m_orig->signal_connect_hide_lookup_table (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_hide_lookup_table));
 
            m_orig->signal_connect_update_preedit_caret (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_update_preedit_caret));
            m_orig->signal_connect_update_preedit_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_update_preedit_string));
            m_orig->signal_connect_update_aux_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_update_aux_string));
            m_orig->signal_connect_update_lookup_table (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_update_lookup_table));
 
            m_orig->signal_connect_commit_string (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_commit_string));
 
            m_orig->signal_connect_forward_key_event (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_forward_key_event));
 
            m_orig->signal_connect_register_properties (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_register_properties));
 
            m_orig->signal_connect_update_property (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_update_property));
 
            m_orig->signal_connect_beep (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_beep));
 
            m_orig->signal_connect_start_helper (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_start_helper));
 
            m_orig->signal_connect_stop_helper (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_stop_helper));
 
            m_orig->signal_connect_send_helper_event (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_send_helper_event));
 
            m_orig->signal_connect_get_surrounding_text (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_get_surrounding_text));
 
            m_orig->signal_connect_delete_surrounding_text (
                slot (this, &FilterInstanceBase::FilterInstanceBaseImpl::slot_delete_surrounding_text));
        }
    }
 
    bool set_encoding (const String &encoding) {
        return m_orig.null () ? false : m_orig->set_encoding (encoding);
    }

    bool process_key_event (const KeyEvent &key) {
        return m_orig.null () ? false : m_orig->process_key_event (key);
    }

    void move_preedit_caret (unsigned int pos) {
        if (!m_orig.null ()) m_orig->move_preedit_caret (pos);
    }

    void select_candidate (unsigned int index) {
        if (!m_orig.null ()) m_orig->select_candidate (index);
    }

    void update_lookup_table_page_size (unsigned int page_size) {
        if (!m_orig.null ()) m_orig->update_lookup_table_page_size (page_size);
    }

    void lookup_table_page_up () {
        if (!m_orig.null ()) m_orig->lookup_table_page_up ();
    }

    void lookup_table_page_down () {
        if (!m_orig.null ()) m_orig->lookup_table_page_down ();
    }

    void reset () {
        if (!m_orig.null ()) m_orig->reset ();
    }

    void focus_in () {
        if (!m_orig.null ()) m_orig->focus_in ();
    }

    void focus_out () {
        if (!m_orig.null ()) m_orig->focus_out ();
    }

    void trigger_property (const String &property) {
        if (!m_orig.null ()) m_orig->trigger_property (property);
    }

    void process_helper_event (const String &helper_uuid, const Transaction &trans) {
        if (!m_orig.null ()) m_orig->process_helper_event (helper_uuid, trans);
    }

    void update_client_capabilities (unsigned int cap) {
        if (!m_orig.null ()) m_orig->update_client_capabilities (cap);
    }

private:
    void slot_show_preedit_string   (IMEngineInstanceBase * si) {
        m_parent->filter_show_preedit_string ();
    }

    void slot_show_aux_string       (IMEngineInstanceBase * si) {
        m_parent->filter_show_aux_string ();
    }

    void slot_show_lookup_table     (IMEngineInstanceBase * si) {
        m_parent->filter_show_lookup_table ();
    }

    void slot_hide_preedit_string   (IMEngineInstanceBase * si) {
        m_parent->filter_hide_preedit_string ();
    }

    void slot_hide_aux_string       (IMEngineInstanceBase * si) {
        m_parent->filter_hide_aux_string ();
    }

    void slot_hide_lookup_table     (IMEngineInstanceBase * si) {
        m_parent->filter_hide_lookup_table ();
    }

    void slot_update_preedit_caret  (IMEngineInstanceBase * si, int caret) {
        m_parent->filter_update_preedit_caret (caret);
    }

    void slot_update_preedit_string (IMEngineInstanceBase * si, const WideString & str, const AttributeList & attrs) {
        m_parent->filter_update_preedit_string (str, attrs);
    }

    void slot_update_aux_string     (IMEngineInstanceBase * si, const WideString & str, const AttributeList & attrs) {
        m_parent->filter_update_aux_string (str, attrs);
    }

    void slot_update_lookup_table   (IMEngineInstanceBase * si, const LookupTable & table) {
        m_parent->filter_update_lookup_table (table);
    }

    void slot_commit_string         (IMEngineInstanceBase * si, const WideString & str) {
        m_parent->filter_commit_string (str);
    }

    void slot_forward_key_event     (IMEngineInstanceBase * si, const KeyEvent & key) {
        m_parent->filter_forward_key_event (key);
    }

    void slot_register_properties   (IMEngineInstanceBase * si, const PropertyList & properties) {
        m_parent->filter_register_properties (properties);
    }

    void slot_update_property       (IMEngineInstanceBase * si, const Property & property) {
        m_parent->filter_update_property (property);
    }

    void slot_beep                  (IMEngineInstanceBase * si) {
        m_parent->filter_beep ();
    }

    void slot_start_helper          (IMEngineInstanceBase * si, const String & helper_uuid) {
        m_parent->filter_start_helper (helper_uuid);
    }

    void slot_stop_helper           (IMEngineInstanceBase * si, const String & helper_uuid) {
        m_parent->filter_stop_helper (helper_uuid);
    }

    void slot_send_helper_event     (IMEngineInstanceBase * si, const String & helper_uuid, const Transaction & trans) {
        m_parent->filter_send_helper_event (helper_uuid, trans);
    }

    bool slot_get_surrounding_text  (IMEngineInstanceBase * si, WideString &text, int &cursor, int maxlen_before, int maxlen_after) {
        return m_parent->filter_get_surrounding_text (text, cursor, maxlen_before, maxlen_after);
    }

    bool slot_delete_surrounding_text(IMEngineInstanceBase * si, int offset, int len) {
        return m_parent->filter_delete_surrounding_text (offset, len);
    }
};

FilterInstanceBase::FilterInstanceBase (FilterFactoryBase     *factory,
                                        const IMEngineInstancePointer &orig_inst)
    : IMEngineInstanceBase (factory, (orig_inst.null () ? "UTF-8" : orig_inst->get_encoding ()), (orig_inst.null () ? -1 : orig_inst->get_id ())),
      m_impl (new FilterInstanceBaseImpl (this, orig_inst))
{
}

FilterInstanceBase::~FilterInstanceBase ()
{
    delete m_impl;
}

bool
FilterInstanceBase::set_encoding (const String &encoding)
{
    return IMEngineInstanceBase::set_encoding (encoding) && m_impl->set_encoding (encoding);
}

bool
FilterInstanceBase::process_key_event (const KeyEvent &key)
{
    return m_impl->process_key_event (key);
}

void
FilterInstanceBase::move_preedit_caret (unsigned int pos)
{
    m_impl->move_preedit_caret (pos);
}

void
FilterInstanceBase::select_candidate (unsigned int index)
{
    m_impl->select_candidate (index);
}

void
FilterInstanceBase::update_lookup_table_page_size (unsigned int page_size)
{
    m_impl->update_lookup_table_page_size (page_size);
}

void
FilterInstanceBase::lookup_table_page_up ()
{
    m_impl->lookup_table_page_up ();
}

void
FilterInstanceBase::lookup_table_page_down ()
{
    m_impl->lookup_table_page_down ();
}

void
FilterInstanceBase::reset ()
{
    m_impl->reset ();
}

void
FilterInstanceBase::focus_in ()
{
    m_impl->focus_in ();
}

void
FilterInstanceBase::focus_out ()
{
    m_impl->focus_out ();
}

void
FilterInstanceBase::trigger_property (const String &property)
{
    m_impl->trigger_property (property);
}

void
FilterInstanceBase::process_helper_event (const String &helper_uuid, const Transaction &trans)
{
    m_impl->process_helper_event (helper_uuid, trans);
}

void
FilterInstanceBase::update_client_capabilities (unsigned int cap)
{
    m_impl->update_client_capabilities (cap);
}

void
FilterInstanceBase::filter_show_preedit_string ()
{
    show_preedit_string ();
}

void
FilterInstanceBase::filter_show_aux_string ()
{
    show_aux_string ();
}

void
FilterInstanceBase::filter_show_lookup_table ()
{
    show_lookup_table ();
}

void
FilterInstanceBase::filter_hide_preedit_string ()
{
    hide_preedit_string ();
}

void
FilterInstanceBase::filter_hide_aux_string ()
{
    hide_aux_string ();
}

void
FilterInstanceBase::filter_hide_lookup_table ()
{
    hide_lookup_table ();
}

void
FilterInstanceBase::filter_update_preedit_caret (int caret)
{
    update_preedit_caret (caret);
}

void
FilterInstanceBase::filter_update_preedit_string (const WideString    &str,
                                                          const AttributeList &attrs)
{
    update_preedit_string (str, attrs);
}

void
FilterInstanceBase::filter_update_aux_string (const WideString    &str,
                                                      const AttributeList &attrs)
{
    update_aux_string (str, attrs);
}

void
FilterInstanceBase::filter_update_lookup_table (const LookupTable &table)
{
    update_lookup_table (table);
}

void
FilterInstanceBase::filter_commit_string (const WideString &str)
{
    commit_string (str);
}

void
FilterInstanceBase::filter_forward_key_event (const KeyEvent &key)
{
    forward_key_event (key);
}

void
FilterInstanceBase::filter_register_properties (const PropertyList &properties)
{
    register_properties (properties);
}

void
FilterInstanceBase::filter_update_property (const Property &property)
{
    update_property (property);
}

void
FilterInstanceBase::filter_beep ()
{
    beep ();
}

void
FilterInstanceBase::filter_start_helper (const String &helper_uuid)
{
    start_helper (helper_uuid);
}

void
FilterInstanceBase::filter_stop_helper (const String &helper_uuid)
{
    stop_helper (helper_uuid);
}

void
FilterInstanceBase::filter_send_helper_event (const String &helper_uuid, const Transaction &trans)
{
    send_helper_event (helper_uuid, trans);
}

bool
FilterInstanceBase::filter_get_surrounding_text (WideString &text, int &cursor, int maxlen_before, int maxlen_after)
{
    return get_surrounding_text (text, cursor, maxlen_before, maxlen_after);
}

bool
FilterInstanceBase::filter_delete_surrounding_text (int offset, int len)
{
    return delete_surrounding_text (offset, len);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
