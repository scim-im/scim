/** @file scim_ibus_ctx.h
 * definition of IBUSCTX related classes.
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
 * $Id: scim_ibus_ctx.h,v 1.10 2020/04/29 16:35:12 derekdai Exp $
 */

#if !defined (__SCIM_IBUS_CTX_H)
#define __SCIM_IBUS_CTX_H

#include "scim.h"

using namespace scim;

struct IBusContentType
{
    uint32_t purpose;
    uint32_t hints;

    IBusContentType () : purpose (0), hints (0) { }

    int from_message (sd_bus_message *m)
    {
        int r;
        if ((r = sd_bus_message_enter_container (m, 'r', "uu")) < 0) {
            return r;
        }

        return sd_bus_message_read (m, "uu", &purpose, &hints);
    }

    int to_message (sd_bus_message *m) const
    {
        int r;
        if ((r = sd_bus_message_open_container(m, 'r', "uu")) < 0) {
            return r;
        }

        if ((r = sd_bus_message_append(m, "uu", purpose, hints)) < 0) {
            return r;
        }

        return sd_bus_message_close_container(m);
    }
};

struct IBusRect
{
    int x;
    int y;
    int w;
    int h;

    IBusRect (): x (0), y (0), w (0), h (0) { }

    int from_message (sd_bus_message *m)
    {
        return sd_bus_message_read (m, "iiii", &x, &y, &w, &h);
    }

    const IBusRect & operator +=(const IBusRect &other)
    {
        x += other.x;
        y += other.y;
        w += other.w;
        h += other.h;

        return *this;
    }
};

class IBusCtx
{
    String                      m_ibus_id;
    int                         m_id;
    int                         m_siid;
    uint32_t                    m_caps;
    IBusContentType             m_content_type;
    IBusRect                    m_cursor_location;
    bool                        m_client_commit_preedit;
    bool                        m_on;
    bool                        m_shared_siid;
    String                      m_locale;
    WideString                  m_preedit_text;
    AttributeList               m_preedit_attrs;
    int                         m_preedit_caret;
    KeyboardLayout              m_keyboard_layout;
    sd_bus_slot                *m_inputcontext_slot;
    sd_bus_slot                *m_service_slot;

public:
    IBusCtx (const String &owner, const String &locale, int id, int siid);

    ~IBusCtx();

    int init (sd_bus *bus, const char *path);

    const String owner () const { return m_ibus_id; }

    int id () const { return m_id; }

    uint32_t caps () const { return m_caps; }
    uint32_t scim_caps () const;
    int caps_from_message (sd_bus_message *v);

    const IBusContentType &content_type() const { return m_content_type; }
    int content_type_from_message (sd_bus_message *v);
    int content_type_to_message (sd_bus_message *v) const;

    const IBusRect &cursor_location() const { return m_cursor_location; }
    IBusRect &cursor_location() { return m_cursor_location; }
    int cursor_location_from_message (sd_bus_message *v);
    int cursor_location_relative_from_message (sd_bus_message *v);

    int siid() const { return m_siid; }
    void siid (int siid) { m_siid = siid; }

    bool shared_siid() const { return m_shared_siid; }
    void shared_siid (bool shared) { m_shared_siid = shared; }

    const String &locale() const { return m_locale; }
    String encoding() const { return scim_get_locale_encoding (m_locale); }
    void locale (const String &locale) { m_locale = locale; }

    const WideString &preedit_text () const { return m_preedit_text; }
    void preedit_text (const WideString &preedit_text) {
        m_preedit_text = preedit_text;
    }

    const AttributeList &preedit_attrs () const { return m_preedit_attrs; }
    void preedit_attrs (const AttributeList &attrs) {
        m_preedit_attrs = attrs;
    }

    int preedit_caret () const { return m_preedit_caret; }
    void preedit_caret (int caret) { m_preedit_caret = caret; }

    void preedit_reset () {
        m_preedit_text.clear ();
        m_preedit_attrs.clear ();
        m_preedit_caret = 0;
    }

    bool client_commit_preedit() const { return m_client_commit_preedit; }
    int client_commit_preedit_from_message (sd_bus_message *v);
    int client_commit_preedit_to_message (sd_bus_message *v) const;

    KeyboardLayout keyboard_layout() const { return m_keyboard_layout; }
    void keyboard_layout(KeyboardLayout layout) { m_keyboard_layout = layout; }

    bool is_on() const { return m_on; }
    void on() { m_on = true; }
    void off() { m_on = false; }
};


#endif // _SCIM_IBUS_CTX_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
