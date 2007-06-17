/**
 * @file scim_compose_key.h
 * @brief Defines scim::ComposeKeyFactory and scim::ComposeKeyInstance.
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
 * $Id: scim_compose_key.h,v 1.5 2005/08/16 07:26:54 suzhe Exp $
 */

#ifndef __SCIM_COMPOSE_KEY_H
#define __SCIM_COMPOSE_KEY_H

namespace scim {
/**
 * @addtogroup IMEngine
 * @{
 */

#define SCIM_COMPOSE_KEY_FACTORY_UUID       "c6bebc27-6324-4b77-8ad4-6d41dcaf2e08"

/**
 * @brief A simple IMEngine to deal with the Compose keys.
 */
class ComposeKeyFactory : public IMEngineFactoryBase
{
public:
    ComposeKeyFactory ();
    virtual ~ComposeKeyFactory ();

    virtual WideString  get_name () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;

    virtual bool validate_encoding (const String& encoding) const;
    virtual bool validate_locale (const String& locale) const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class ComposeKeyInstance : public IMEngineInstanceBase
{
    uint32 m_compose_buffer [8];

public:
    ComposeKeyInstance (ComposeKeyFactory *factory,
                        const String      &encoding,
                        int                id = -1);

    virtual ~ComposeKeyInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int index);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    virtual void trigger_property (const String& property);
};

/**  @} */

} // namespace scim

#endif //__SCIM_COMPOSE_KEY_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

