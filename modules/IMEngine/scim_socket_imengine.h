/** @file scim_socket_imengine.h
 * definition of SocketFactory related classes.
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
 * $Id: scim_socket_imengine.h,v 1.13 2005/07/06 03:57:05 suzhe Exp $
 */

#if !defined (__SCIM_SOCKET_IMENGINE_H)
#define __SCIM_SOCKET_IMENGINE_H

namespace scim {

class SocketFactory;

class SocketFactory : public IMEngineFactoryBase
{
    WideString m_name;

    String     m_language;

    String     m_peer_uuid;

    String     m_icon_file;

    bool       m_ok;

    friend class SocketInstance;

public:
    SocketFactory (const String &peer_uuid);

    bool valid () const { return m_ok; }

    virtual ~SocketFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual String      get_language () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

private:
    int  create_peer_instance (const String &encoding);
};

class SocketInstance : public IMEngineInstanceBase
{
    SocketFactory *m_factory;
    int            m_peer_id;
    Connection     m_signal_reconnect_connection;

public:
    SocketInstance (SocketFactory *factory, const String& encoding, int id, int peer_id);
    virtual ~SocketInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    virtual void trigger_property (const String &property);
    virtual void process_helper_event (const String &helper_uuid, const Transaction &trans);
    virtual void update_client_capabilities (unsigned int cap);

private:
    bool commit_transaction (Transaction &trans);
    bool do_transaction (Transaction &trans, bool &ret);
    void reconnect_callback (void);
};

// Forward declaration
class SocketIMEngineGlobal;

} // namespace scim

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/

