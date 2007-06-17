/** @file scim_rawcode_imengine.h
 * definition of RawCode related classes.
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
 * $Id: scim_rawcode_imengine.h,v 1.5.4.3 2006/01/12 07:00:36 suzhe Exp $
 */

#if !defined (__SCIM_RAWCODE_IMENGINE_H)
#define __SCIM_RAWCODE_IMENGINE_H

using namespace scim;

class RawCodeFactory : public IMEngineFactoryBase
{
    friend class RawCodeInstance;

public:
    RawCodeFactory ();
    virtual ~RawCodeFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual String      get_language () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);

private:
    int get_maxlen (const String &encoding);
};

class RawCodeInstance : public IMEngineInstanceBase
{
    Pointer <RawCodeFactory>    m_factory;

    CommonLookupTable           m_lookup_table;
    std::vector<WideString>     m_lookup_table_labels;
    WideString                  m_preedit_string;
    String                      m_working_encoding;
    bool                        m_unicode;

    size_t                      m_max_preedit_len;

    IConvert                    m_working_iconv;
    IConvert                    m_client_iconv;

public:
    RawCodeInstance (RawCodeFactory *factory,
                     const String& encoding,
                     int id = -1);
    virtual ~RawCodeInstance ();

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

private:
    int create_lookup_table ();
    void process_preedit_string ();

    void initialize_properties ();
    void refresh_encoding_property ();

    void set_working_encoding (const String &encoding);

    String get_multibyte_string (const WideString& preedit);
    ucs4_t get_unicode_value (const WideString& preedit);
};

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/

