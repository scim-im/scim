/** @file scim_sctc_filter.cpp
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
 * $Id: scim_sctc_filter.cpp,v 1.7.2.1 2006/01/09 13:37:25 suzhe Exp $
 *
 */

#define Uses_SCIM_FILTER
#define Uses_SCIM_FILTER_MODULE
#define Uses_SCIM_CONFIG_BASE
#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"
#include "scim_sctc_filter.h"
#include "scim_sctc_filter_data.h"

#define scim_module_init sctc_LTX_scim_module_init
#define scim_module_exit sctc_LTX_scim_module_exit
#define scim_filter_module_init sctc_LTX_scim_filter_module_init
#define scim_filter_module_create_filter sctc_LTX_scim_filter_module_create_filter
#define scim_filter_module_get_filter_info sctc_LTX_scim_filter_module_get_filter_info

using namespace scim;

// Private datatype definition.
#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <unsigned short, unsigned short, __gnu_cxx::hash <unsigned short> > UUMap;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <unsigned short, unsigned short, std::hash <unsigned short> >             UUMap;
#else
typedef std::map <unsigned short, unsigned short>                                               UUMap;
#endif

// Private data definition.
static FilterInfo   __filter_info (String ("adb861a9-76da-454c-941b-1957e644a94e"),
                                   String (_("Simplified-Traditional Chinese Conversion")),
                                   String ("zh_CN,zh_TW,zh_SG,zh_HK"),
                                   String (SCIM_ICONDIR "/sctc.png"),
                                   String (_("Convert between Simplified Chinese and Traditional Chinese")));

static std::vector <String> __sc_encodings;
static std::vector <String> __tc_encodings;

static UUMap        __sc_to_tc_map;
static UUMap        __tc_to_sc_map;
static bool         __sc_to_tc_initialized = false;
static bool         __tc_to_sc_initialized = false;

static Property     __prop_root     (String ("/Filter/SCTC"),
                                     String (_("SC-TC")),
                                     String (SCIM_ICONDIR "/sctc.png"),
                                     String (_("Simplified-Traditional Chinese conversion")));

static Property     __prop_off      (String ("/Filter/SCTC/Off"),
                                     String (_("No Conversion")),
                                     String (SCIM_ICONDIR "/sctc.png"),
                                     String (_("Simplified-Traditional Chinese conversion")));

static Property     __prop_sc_to_tc (String ("/Filter/SCTC/SC-TC"),
                                     String (_("Simplified to Traditional")),
                                     String (SCIM_ICONDIR "/sctc-sc-to-tc.png"),
                                     String (_("Convert Simplified Chinese to Traditional Chinese")));

static Property     __prop_tc_to_sc (String ("/Filter/SCTC/TC-SC"),
                                     String (_("Traditional to Simplified")),
                                     String (SCIM_ICONDIR "/sctc-tc-to-sc.png"),
                                     String (_("Convert Traditional Chinese to Simplified Chinese")));

//Private functions definition.
static void       __init_sc_to_tc ();
static void       __init_tc_to_sc ();

static bool       __is_sc_encoding (const String &encoding);
static bool       __is_tc_encoding (const String &encoding);

static WideString __sc_to_tc (const WideString &sc);
static WideString __tc_to_sc (const WideString &tc);


//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        //Initialize encoding information.
        __sc_encodings.push_back ("GB2312");
        __sc_encodings.push_back ("GBK");
        __sc_encodings.push_back ("GB18030");
        __sc_encodings.push_back ("EUC-CN");
        __tc_encodings.push_back ("BIG5");
        __tc_encodings.push_back ("BIG5-HKSCS");
        __tc_encodings.push_back ("EUC-TW");
    }

    void scim_module_exit (void)
    {
    }

    unsigned int scim_filter_module_init (const ConfigPointer &config)
    {
        return 1;
    }

    FilterFactoryPointer scim_filter_module_create_filter (unsigned int index)
    {
        if (index == 0)
            return new SCTCFilterFactory ();

        return FilterFactoryPointer (0);
    }

    bool scim_filter_module_get_filter_info (unsigned int index, FilterInfo &info)
    {
        if (index == 0) {
            info = __filter_info;
            return true;
        }
        return false;
    }
}

//Implementation of private functions
static void
__init_sc_to_tc ()
{
    if (__sc_to_tc_initialized) return;

    __sc_to_tc_map.clear ();

    for (size_t i = 0; __sc_to_tc_table [i].first; ++i)
        __sc_to_tc_map [__sc_to_tc_table [i].first] = __sc_to_tc_table [i].second;

    __sc_to_tc_initialized = true;
}

static void
__init_tc_to_sc ()
{
    if (__tc_to_sc_initialized) return;

    __tc_to_sc_map.clear ();

    for (size_t i = 0; __tc_to_sc_table [i].first; ++i)
        __tc_to_sc_map [__tc_to_sc_table [i].first] = __tc_to_sc_table [i].second;

    __tc_to_sc_initialized = true;
}

static WideString __sc_to_tc (const WideString &sc)
{
    WideString tc;
    if (!__sc_to_tc_initialized) __init_sc_to_tc ();

    UUMap::const_iterator mapit;
    WideString::const_iterator sit;

    for (sit = sc.begin (); sit != sc.end (); ++sit) {
        if (*sit <= 0xFFFF) {
            mapit = __sc_to_tc_map.find ((unsigned short) (*sit));
            if (mapit != __sc_to_tc_map.end ()) {
                tc.push_back (static_cast<ucs4_t> (mapit->second));
            } else {
                tc.push_back (*sit);
            }
        } else {
            tc.push_back (*sit);
        }
    }
    return tc;
}

static WideString __tc_to_sc (const WideString &tc)
{
    WideString sc;
    if (!__tc_to_sc_initialized) __init_tc_to_sc ();

    UUMap::const_iterator mapit;
    WideString::const_iterator sit;

    for (sit = tc.begin (); sit != tc.end (); ++sit) {
        if (*sit <= 0xFFFF) {
            mapit = __tc_to_sc_map.find ((unsigned short) (*sit));
            if (mapit != __tc_to_sc_map.end ()) {
                sc.push_back (static_cast<ucs4_t> (mapit->second));
            } else {
                sc.push_back (*sit);
            }
        } else {
            sc.push_back (*sit);
        }
    }
    return sc;
}

static bool
__is_sc_encoding (const String &encoding)
{
    return std::find (__sc_encodings.begin (), __sc_encodings.end (), encoding) != __sc_encodings.end ();
}

static bool
__is_tc_encoding (const String &encoding)
{
    return std::find (__tc_encodings.begin (), __tc_encodings.end (), encoding) != __tc_encodings.end ();
}

//Implementation of SCTCFilterFactory.
SCTCFilterFactory::SCTCFilterFactory ()
    : m_sc_ok(false),
      m_tc_ok(false)
{
}

void
SCTCFilterFactory::attach_imengine_factory (const IMEngineFactoryPointer &orig)
{
    size_t i;

    FilterFactoryBase::attach_imengine_factory (orig);

    for (i = 0; i < __sc_encodings.size (); ++i) {
        if (orig->validate_encoding (__sc_encodings [i])) {
            m_sc_ok = true;
            if (orig->validate_encoding ("GB18030"))
                m_sc_encoding = "GB18030";
            else
                m_sc_encoding = __sc_encodings [i];

            break;
        }
    }

    for (i = 0; i < __tc_encodings.size (); ++i) {
        if (orig->validate_encoding (__tc_encodings [i])) {
            m_tc_ok = true;
            if (orig->validate_encoding ("BIG5"))
                m_tc_encoding = "BIG5";
            else
                m_tc_encoding = __tc_encodings [i];

            break;
        }
    }

    if (m_sc_ok || m_tc_ok) {
        String locales = orig->get_locales ();
        locales = locales + String (",") + scim_get_language_locales ("zh_CN");
        locales = locales + String (",") + scim_get_language_locales ("zh_TW");
        locales = locales + String (",") + scim_get_language_locales ("zh_SG");
        locales = locales + String (",") + scim_get_language_locales ("zh_HK");
        set_locales (locales);
    }
}

WideString
SCTCFilterFactory::get_name () const
{
     WideString name = FilterFactoryBase::get_name ();
     return name.length () ? name : utf8_mbstowcs (__filter_info.name);
}

String
SCTCFilterFactory::get_uuid () const
{
    String uuid = FilterFactoryBase::get_uuid ();
    return uuid.length () ? uuid : __filter_info.uuid;
}

String
SCTCFilterFactory::get_icon_file () const
{
    String icon = FilterFactoryBase::get_icon_file ();
    return icon.length () ? icon : __filter_info.icon;
}

WideString
SCTCFilterFactory::get_authors () const
{
    WideString authors = FilterFactoryBase::get_authors ();
    return authors.length () ? authors : utf8_mbstowcs (_("James Su <suzhe@tsinghua.org.cn>"));
}

WideString
SCTCFilterFactory::get_help () const
{
    // No help yet.
    WideString help = FilterFactoryBase::get_help ();
    return help;
}

bool
SCTCFilterFactory::validate_encoding (const String& encoding) const
{
    // Bypass the original IMEngineFactory.
    return IMEngineFactoryBase::validate_encoding (encoding);
}

bool
SCTCFilterFactory::validate_locale (const String& locale) const
{
    // Bypass the original IMEngineFactory.
    return IMEngineFactoryBase::validate_locale (locale);
}

IMEngineInstancePointer
SCTCFilterFactory::create_instance (const String& encoding, int id)
{
    if (m_sc_ok || m_tc_ok) {
        SCTCWorkMode mode = SCTC_MODE_OFF;

        String orig_encoding = encoding;
 
        // If the original IMEngineFactory doesn't support this encoding,
        // then we must use another encoding to create the original IMEngineInstance.
        // It means we must use a conversion mode.
        if (!FilterFactoryBase::validate_encoding (encoding)) {
            // The client encoding is Simplified Chinese encoding, but is not supported by the IMEngine.
            // So use Traditional Chinese encoding instead.
            if (__is_sc_encoding (encoding)) {
                if (FilterFactoryBase::validate_encoding (m_sc_encoding)) {
                    orig_encoding = m_sc_encoding;
                } else {
                    orig_encoding = m_tc_encoding;
                    mode = SCTC_MODE_FORCE_TC_TO_SC;
                }
            } else if (__is_tc_encoding (encoding)) {
                if (FilterFactoryBase::validate_encoding (m_tc_encoding)) {
                    orig_encoding = m_tc_encoding;
                } else {
                    orig_encoding = m_sc_encoding;
                    mode = SCTC_MODE_FORCE_SC_TO_TC;
                }
            }
        } else if ((__is_sc_encoding (encoding) && !FilterFactoryBase::validate_encoding (m_tc_encoding)) ||
                   (__is_tc_encoding (encoding) && !FilterFactoryBase::validate_encoding (m_sc_encoding))) {
            mode = SCTC_MODE_FORCE_OFF;
        }

        return new SCTCFilterInstance (this, mode, encoding, FilterFactoryBase::create_instance (orig_encoding, id));
    }

    return FilterFactoryBase::create_instance (encoding, id);
}

//Implementation of SCTCFilterInstance
SCTCFilterInstance::SCTCFilterInstance (SCTCFilterFactory *factory, const SCTCWorkMode &mode, const String &client_encoding, const IMEngineInstancePointer &orig_inst)
    : FilterInstanceBase (factory, orig_inst),
      m_factory (factory),
      m_props_registered (false),
      m_work_mode (mode)
{
    IMEngineInstanceBase::set_encoding (client_encoding);
}

bool
SCTCFilterInstance::set_encoding (const String &encoding)
{
    if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC) {
        if (__is_tc_encoding (encoding))
            FilterInstanceBase::set_encoding (m_factory->m_sc_encoding);
    } else if (m_work_mode == SCTC_MODE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC) {
        if (__is_sc_encoding (encoding))
            FilterInstanceBase::set_encoding (m_factory->m_tc_encoding);
    } else {
        FilterInstanceBase::set_encoding (encoding);
    }
    reset ();
    return IMEngineInstanceBase::set_encoding (encoding);
}

void
SCTCFilterInstance::focus_in ()
{
    m_props_registered = false;

    FilterInstanceBase::focus_in ();

    if (!m_props_registered) {
        PropertyList props;
        filter_register_properties (props); 
    }
}

void
SCTCFilterInstance::trigger_property (const String &property)
{
    if (property != __prop_off.get_key () && property != __prop_sc_to_tc.get_key () && property != __prop_tc_to_sc.get_key ()) {
        FilterInstanceBase::trigger_property (property);
        return;
    }

    if (m_work_mode == SCTC_MODE_FORCE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_OFF)
        return;

    Property prop = __prop_root;
    bool changed = false;

    if (property == __prop_off.get_key () && (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_TC_TO_SC)) {
        m_work_mode = SCTC_MODE_OFF;
        changed = true;
    } else if (property == __prop_sc_to_tc.get_key () && m_factory->m_sc_ok && !__is_sc_encoding (get_encoding ()) &&
               (m_work_mode == SCTC_MODE_OFF || m_work_mode == SCTC_MODE_TC_TO_SC)) {
        m_work_mode = SCTC_MODE_SC_TO_TC;
        prop.set_icon (__prop_sc_to_tc.get_icon ());
        prop.set_label (_("SC->TC"));
        changed = true;
    } else if (property == __prop_tc_to_sc.get_key () && m_factory->m_tc_ok && !__is_tc_encoding (get_encoding ()) &&
               (m_work_mode == SCTC_MODE_OFF || m_work_mode == SCTC_MODE_SC_TO_TC)) {
        m_work_mode = SCTC_MODE_TC_TO_SC;
        prop.set_icon (__prop_tc_to_sc.get_icon ());
        prop.set_label (_("TC->SC"));
        changed = true;
    }

    if (changed) {
        set_encoding (get_encoding ());
        update_property (prop);
    }
}

void
SCTCFilterInstance::filter_update_preedit_string (const WideString    &str,
                                                  const AttributeList &attrs)
{
    WideString nstr = str;

    if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC)
        nstr = __sc_to_tc (str);
    if (m_work_mode == SCTC_MODE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC)
        nstr = __tc_to_sc (str);

    update_preedit_string (nstr, attrs);
}

void
SCTCFilterInstance::filter_update_aux_string (const WideString    &str,
                                              const AttributeList &attrs)
{
    WideString nstr = str;

    if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC)
        nstr = __sc_to_tc (str);
    if (m_work_mode == SCTC_MODE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC)
        nstr = __tc_to_sc (str);

    update_aux_string (nstr, attrs);
}

void
SCTCFilterInstance::filter_update_lookup_table (const LookupTable &table)
{
    if (m_work_mode == SCTC_MODE_OFF) {
        update_lookup_table (table);
    } else {
        CommonLookupTable ntable;
        std::vector<WideString> labels;
        size_t i;

        // Can be paged up.
        if (table.get_current_page_start ())
            ntable.append_candidate (0x3400);

        if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC) {
            for (i = 0; i < table.get_current_page_size (); ++i) {
                ntable.append_candidate (__sc_to_tc (table.get_candidate_in_current_page (i)), table.get_attributes_in_current_page (i));
                labels.push_back (__sc_to_tc (table.get_candidate_label (i)));
            }
        } else {
            for (i = 0; i < table.get_current_page_size (); ++i) {
                ntable.append_candidate (__tc_to_sc (table.get_candidate_in_current_page (i)), table.get_attributes_in_current_page (i));
                labels.push_back (__tc_to_sc (table.get_candidate_label (i)));
            }
        }

        if (table.get_current_page_start () + table.get_current_page_size () < table.number_of_candidates ())
            ntable.append_candidate (0x3400);

        if (table.get_current_page_start ()) {
            ntable.set_page_size (1);
            ntable.page_down ();
        }

        ntable.set_page_size (table.get_current_page_size ());
        ntable.set_cursor_pos_in_current_page (table.get_cursor_pos_in_current_page ());
        ntable.show_cursor (table.is_cursor_visible ());
        ntable.fix_page_size (table.is_page_size_fixed ());
        ntable.set_candidate_labels (labels);

        update_lookup_table (ntable);
    }
}

void
SCTCFilterInstance::filter_commit_string (const WideString &str)
{
    WideString nstr = str;

    if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC)
        nstr = __sc_to_tc (str);
    if (m_work_mode == SCTC_MODE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC)
        nstr = __tc_to_sc (str);

    commit_string (nstr);
}

void
SCTCFilterInstance::filter_register_properties (const PropertyList &properties)
{
    PropertyList props;

    if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC) {
        for (size_t i = 0; i < properties.size (); ++i) {
            Property prop = properties [i];
            prop.set_label (utf8_wcstombs (__sc_to_tc (utf8_mbstowcs (prop.get_label ()))));
            prop.set_tip   (utf8_wcstombs (__sc_to_tc (utf8_mbstowcs (prop.get_tip ()))));
            props.push_back (prop);
        }
    } else if (m_work_mode == SCTC_MODE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC) {
        for (size_t i = 0; i < properties.size (); ++i) {
            Property prop = properties [i];
            prop.set_label (utf8_wcstombs (__tc_to_sc (utf8_mbstowcs (prop.get_label ()))));
            prop.set_tip   (utf8_wcstombs (__tc_to_sc (utf8_mbstowcs (prop.get_tip ()))));
            props.push_back (prop);
        }
    } else {
        props = properties;
    }

    if (m_work_mode == SCTC_MODE_OFF || m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_TC_TO_SC) {
        Property root = __prop_root;

        if (m_work_mode == SCTC_MODE_SC_TO_TC) {
            root.set_icon (__prop_sc_to_tc.get_icon ());
            root.set_tip  (__prop_sc_to_tc.get_tip ());
            root.set_label (_("SC->TC"));
        } else if (m_work_mode == SCTC_MODE_TC_TO_SC) {
            root.set_icon (__prop_tc_to_sc.get_icon ());
            root.set_tip  (__prop_tc_to_sc.get_tip ());
            root.set_label (_("TC->SC"));
        }

        props.push_back (root);
        props.push_back (__prop_off);

        if (!__is_sc_encoding (get_encoding ()) && m_factory->m_sc_ok)
            props.push_back (__prop_sc_to_tc);
        if (!__is_tc_encoding (get_encoding ()) && m_factory->m_tc_ok)
            props.push_back (__prop_tc_to_sc);
    } else if (m_work_mode == SCTC_MODE_FORCE_SC_TO_TC) {
        Property root = __prop_sc_to_tc;
        root.set_label (_("SC->TC"));
        props.push_back (root);
    } else if (m_work_mode == SCTC_MODE_FORCE_TC_TO_SC) {
        Property root = __prop_tc_to_sc;
        root.set_label (_("TC->SC"));
        props.push_back (root);
    }

    register_properties (props);

    m_props_registered = true;
}

void
SCTCFilterInstance::filter_update_property (const Property &property)
{
    Property prop = property;

    if (m_work_mode == SCTC_MODE_SC_TO_TC || m_work_mode == SCTC_MODE_FORCE_SC_TO_TC) {
        prop.set_label (utf8_wcstombs (__sc_to_tc (utf8_mbstowcs (prop.get_label ()))));
        prop.set_tip   (utf8_wcstombs (__sc_to_tc (utf8_mbstowcs (prop.get_tip ()))));
    } else if (m_work_mode == SCTC_MODE_TC_TO_SC || m_work_mode == SCTC_MODE_FORCE_TC_TO_SC) {
        prop.set_label (utf8_wcstombs (__tc_to_sc (utf8_mbstowcs (prop.get_label ()))));
        prop.set_tip   (utf8_wcstombs (__tc_to_sc (utf8_mbstowcs (prop.get_tip ()))));
    }

    update_property (prop);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
