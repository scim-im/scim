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
 * $Id: scim_lookup_table.cpp,v 1.32 2005/05/13 04:21:29 suzhe Exp $
 *
 */

#define Uses_SCIM_LOOKUP_TABLE
#include "scim_private.h"
#include "scim.h"

namespace scim {

struct LookupTable::LookupTableImpl
{
    std::vector<int>        m_page_history;
    int                     m_page_size;
    int                     m_current_page_start;
    int                     m_cursor_pos;
    bool                    m_cursor_visible;
    bool                    m_page_size_fixed;

    std::vector<WideString> m_candidate_labels;

    LookupTableImpl (int page_size)
        : m_page_size (page_size),
          m_current_page_start (0),
          m_cursor_pos (0),
          m_cursor_visible (false),
          m_page_size_fixed (false) {
        if (m_page_size <= 0 || m_page_size > SCIM_LOOKUP_TABLE_MAX_PAGESIZE)
            m_page_size = SCIM_LOOKUP_TABLE_MAX_PAGESIZE;
    }
};


//implementation of class LookupTable
LookupTable::LookupTable (int page_size)
    : m_impl (new LookupTableImpl (page_size))
{
}

LookupTable::~LookupTable ()
{
    delete m_impl;
}

void
LookupTable::set_candidate_labels (const std::vector<WideString> &labels)
{
    if (labels.size ())
        m_impl->m_candidate_labels = labels;
}

void
LookupTable::set_page_size (int page_size)
{
    if (page_size > 0 && page_size <= SCIM_LOOKUP_TABLE_MAX_PAGESIZE) {
        m_impl->m_page_size = page_size;
        if (m_impl->m_cursor_pos >= m_impl->m_current_page_start + get_current_page_size ())
            m_impl->m_cursor_pos = m_impl->m_current_page_start + get_current_page_size () - 1;
        if (m_impl->m_cursor_pos < 0)
            m_impl->m_cursor_pos = 0;
    }
}

int
LookupTable::get_page_size () const
{
    return m_impl->m_page_size;
}

int
LookupTable::get_current_page_start () const
{
    return m_impl->m_current_page_start;
}

int
LookupTable::get_cursor_pos() const
{
    return m_impl->m_cursor_pos;
}

int
LookupTable::get_cursor_pos_in_current_page () const
{
    return m_impl->m_cursor_pos - m_impl->m_current_page_start;
}

int
LookupTable::get_current_page_size () const
{
    return std::min ((uint32) m_impl->m_page_size, (uint32)(number_of_candidates () - m_impl->m_current_page_start));
}

bool
LookupTable::page_up ()
{
    if (m_impl->m_current_page_start <= 0)
        return false;

    if (m_impl->m_page_history.size ()) {
        m_impl->m_page_size = m_impl->m_page_history.back ();
        m_impl->m_page_history.pop_back ();
    }

    if (m_impl->m_current_page_start >= m_impl->m_page_size)
        m_impl->m_current_page_start -= m_impl->m_page_size;
    else
        m_impl->m_current_page_start = 0;

    if (m_impl->m_cursor_pos >= m_impl->m_page_size)
        m_impl->m_cursor_pos -= m_impl->m_page_size;
    else
        m_impl->m_cursor_pos = 0;

    if (m_impl->m_cursor_pos < m_impl->m_current_page_start)
        m_impl->m_cursor_pos = m_impl->m_current_page_start;
    else if (m_impl->m_cursor_pos >= m_impl->m_current_page_start + get_current_page_size ())
        m_impl->m_cursor_pos = m_impl->m_current_page_start + get_current_page_size () - 1;

    return true;
}

bool
LookupTable::page_down ()
{
    if (m_impl->m_current_page_start + m_impl->m_page_size >= number_of_candidates ())
        return false;

    m_impl->m_current_page_start += m_impl->m_page_size;
    m_impl->m_page_history.push_back (m_impl->m_page_size);

    m_impl->m_cursor_pos += m_impl->m_page_size;
    
    if (m_impl->m_cursor_pos < m_impl->m_current_page_start)
        m_impl->m_cursor_pos = m_impl->m_current_page_start;
    else if (m_impl->m_cursor_pos >= m_impl->m_current_page_start + get_current_page_size ())
        m_impl->m_cursor_pos = m_impl->m_current_page_start + get_current_page_size () - 1;

    return true;
}

bool
LookupTable::cursor_up ()
{
    if (m_impl->m_cursor_pos <= 0) return false;

    if (!m_impl->m_cursor_visible)
        m_impl->m_cursor_visible = true;

    m_impl->m_cursor_pos --;
    
    if (m_impl->m_cursor_pos < m_impl->m_current_page_start) {
        page_up ();
        m_impl->m_cursor_pos = m_impl->m_current_page_start + get_current_page_size () - 1;
    }
    return true;
}

bool
LookupTable::cursor_down ()
{
    if (m_impl->m_cursor_pos + 1 >= number_of_candidates ())
        return false;

    if (!m_impl->m_cursor_visible)
        m_impl->m_cursor_visible = true;

    ++m_impl->m_cursor_pos;
    
    if (m_impl->m_cursor_pos >= m_impl->m_current_page_start + get_current_page_size ()) {
        page_down ();
        m_impl->m_cursor_pos = m_impl->m_current_page_start;
    }
    return true;
}

void
LookupTable::show_cursor (bool show)
{
    m_impl->m_cursor_visible = show;
}

bool
LookupTable::is_cursor_visible () const
{
    return m_impl->m_cursor_visible;
}

void
LookupTable::fix_page_size (bool fixed)
{
    m_impl->m_page_size_fixed = fixed;
}

bool
LookupTable::is_page_size_fixed () const
{
    return m_impl->m_page_size_fixed;
}

void
LookupTable::set_cursor_pos (int pos)
{
    if (pos < 0 || pos >= number_of_candidates ()) return;

    if (!m_impl->m_cursor_visible)
        m_impl->m_cursor_visible = true;

    if (pos >= get_current_page_start () && pos < get_current_page_start () + get_current_page_size ()) {
        m_impl->m_cursor_pos = pos;
    } else if (pos < get_cursor_pos ()) {
        while (pos < get_cursor_pos ())
            cursor_up ();
    } else if (pos > get_cursor_pos ()) {
        while (pos > get_cursor_pos ())
            cursor_down ();
    }
}

void
LookupTable::set_cursor_pos_in_current_page (int pos)
{
    if (pos < 0 || pos >= get_current_page_size ()) return;

    if (!m_impl->m_cursor_visible)
        m_impl->m_cursor_visible = true;

    m_impl->m_cursor_pos = pos + get_current_page_start ();
}

WideString
LookupTable::get_candidate_label (int page_index) const
{
    if (page_index >= 0 && page_index < get_current_page_size () && page_index < m_impl->m_candidate_labels.size ())
        return m_impl->m_candidate_labels [page_index];
    return WideString ();
}

WideString
LookupTable::get_candidate_in_current_page (int page_index) const
{
    if (page_index >= 0 && page_index < get_current_page_size ())
        return get_candidate (page_index + m_impl->m_current_page_start);
    return WideString ();
}

AttributeList
LookupTable::get_attributes_in_current_page (int page_index) const
{
    if (page_index >= 0 && page_index < get_current_page_size ())
        return get_attributes (page_index + m_impl->m_current_page_start);
    return AttributeList ();
}

void
LookupTable::clear ()
{
    m_impl->m_current_page_start = 0;
    m_impl->m_cursor_pos = 0;
    std::vector <int> ().swap (m_impl->m_page_history);
}

class CommonLookupTable::CommonLookupTableImpl
{
public:
    std::vector <ucs4_t> m_buffer;
    std::vector <uint32> m_index;

    AttributeList        m_attributes;
    std::vector <uint32> m_attrs_index;
};

//implementation of CommonLookupTable
CommonLookupTable::CommonLookupTable (int page_size)
    : LookupTable (page_size), m_impl (new CommonLookupTableImpl ())
{
    std::vector <WideString> labels;
    char buf [2] = { 0, 0 };
    for (int i = 0; i < 9; ++i) {
        buf [0] = '1' + i;
        labels.push_back (utf8_mbstowcs (buf));
    }

    labels.push_back (utf8_mbstowcs ("0"));

    set_candidate_labels (labels);
}

CommonLookupTable::CommonLookupTable (int                            page_size,
                                      const std::vector<WideString> &labels)
    : LookupTable (page_size), m_impl (new CommonLookupTableImpl ())
{
    set_candidate_labels (labels);
}

CommonLookupTable::~CommonLookupTable ()
{
    delete m_impl;
}

uint32
CommonLookupTable::number_of_candidates () const
{
    return m_impl->m_index.size ();
}

bool
CommonLookupTable::append_candidate (const WideString    &cand,
                                     const AttributeList &attrs)
{
    if (cand.length () == 0)
        return false;

    m_impl->m_index.push_back (m_impl->m_buffer.size ());
    m_impl->m_buffer.insert (m_impl->m_buffer.end (), cand.begin (), cand.end ());

    m_impl->m_attrs_index.push_back (m_impl->m_attributes.size ());

    if (attrs.size ())
        m_impl->m_attributes.insert (m_impl->m_attributes.end (), attrs.begin (), attrs.end ());

    return true;
}

bool
CommonLookupTable::append_candidate (ucs4_t               cand,
                                     const AttributeList &attrs)
{
    if (cand == 0)
        return false;

    m_impl->m_index.push_back (m_impl->m_buffer.size ());
    m_impl->m_buffer.push_back (cand);

    m_impl->m_attrs_index.push_back (m_impl->m_attributes.size ());

    if (attrs.size ())
        m_impl->m_attributes.insert (m_impl->m_attributes.end (), attrs.begin (), attrs.end ());

    return true;
}

WideString
CommonLookupTable::get_candidate (int index) const
{
    if (index < 0 || index >= number_of_candidates ())
        return WideString ();

    std::vector <ucs4_t>::const_iterator start, end;

    start = m_impl->m_buffer.begin () + m_impl->m_index [index];

    if (index < number_of_candidates () - 1)
        end = m_impl->m_buffer.begin () + m_impl->m_index [index+1];
    else
        end = m_impl->m_buffer.end ();

    return WideString (start, end);
}

AttributeList
CommonLookupTable::get_attributes (int index) const
{
    if (index < 0 || index >= number_of_candidates ())
        return AttributeList ();

    AttributeList::const_iterator start, end;

    start = m_impl->m_attributes.begin () + m_impl->m_attrs_index [index];

    if (index < number_of_candidates () - 1)
        end = m_impl->m_attributes.begin () + m_impl->m_attrs_index [index+1];
    else
        end = m_impl->m_attributes.end ();

    if (start < end)
        return AttributeList (start, end);
 
    return AttributeList ();
}

void
CommonLookupTable::clear ()
{
    LookupTable::clear ();

    std::vector <ucs4_t> ().swap (m_impl->m_buffer);
    std::vector <uint32> ().swap (m_impl->m_index);

    AttributeList ().swap (m_impl->m_attributes);
    std::vector <uint32> ().swap (m_impl->m_attrs_index);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
