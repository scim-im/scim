/** @file scim_sctc_filter.h
 * definition of SCTCFilter (Simplified Chinese <-> Traditional Chinese Filter) related classes.
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
 * $Id: scim_sctc_filter.h,v 1.2 2005/05/17 14:56:39 suzhe Exp $
 */

#if !defined (__SCIM_SCTC_FILTER_H)
#define __SCIM_SCTC_FILTER_H

using namespace scim;

enum SCTCWorkMode
{
    SCTC_MODE_OFF = 0,
    SCTC_MODE_SC_TO_TC,
    SCTC_MODE_TC_TO_SC,
    SCTC_MODE_FORCE_OFF,
    SCTC_MODE_FORCE_SC_TO_TC,
    SCTC_MODE_FORCE_TC_TO_SC
};

class SCTCFilterFactory : public FilterFactoryBase
{
    bool   m_sc_ok;
    String m_sc_encoding;

    bool   m_tc_ok;
    String m_tc_encoding;

    friend class SCTCFilterInstance;

public:
    SCTCFilterFactory ();

    virtual void attach_imengine_factory (const IMEngineFactoryPointer &orig);
    virtual WideString  get_name () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_help () const;
    virtual bool        validate_encoding (const String& encoding) const; 
    virtual bool        validate_locale (const String& locale) const;
    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class SCTCFilterInstance : public FilterInstanceBase
{
    SCTCFilterFactory *m_factory;

    bool               m_props_registered;

    SCTCWorkMode       m_work_mode;

public:
    SCTCFilterInstance (SCTCFilterFactory *factory,
                        const SCTCWorkMode &mode,
                        const String &client_encoding,
                        const IMEngineInstancePointer &orig_inst);

    virtual bool set_encoding (const String &encoding);

public:
    virtual void focus_in ();

    virtual void trigger_property (const String &property);

protected:
    virtual void filter_update_preedit_string (const WideString    &str,
                                               const AttributeList &attrs = AttributeList ());
    virtual void filter_update_aux_string (const WideString    &str,
                                           const AttributeList &attrs = AttributeList ());
    virtual void filter_update_lookup_table (const LookupTable &table);
    virtual void filter_commit_string (const WideString &str);
    virtual void filter_register_properties (const PropertyList &properties);
    virtual void filter_update_property (const Property &property);
};

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/

