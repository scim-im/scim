/** @file scim_hanconv_filter.cpp
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
#include "scim_hanconv_filter.h"
#include "scim_hanconv_sctc_data.h"
#include "scim_hanconv_jp_data.h"

#define scim_module_init hanconv_LTX_scim_module_init
#define scim_module_exit hanconv_LTX_scim_module_exit
#define scim_filter_module_init hanconv_LTX_scim_filter_module_init
#define scim_filter_module_create_filter hanconv_LTX_scim_filter_module_create_filter
#define scim_filter_module_get_filter_info hanconv_LTX_scim_filter_module_get_filter_info

using namespace scim;

// Private datatype definition.
typedef scim_map <unsigned short, unsigned short>                                     UUMap;

// Private data definition.
// The UUID is the one this filter shipped with as "sctc". Keeping it means a
// user who already attached the filter keeps it, and simply gains the new
// directions, instead of finding it silently detached.
static FilterInfo   __filter_info (String ("adb861a9-76da-454c-941b-1957e644a94e"),
                                   String (_("Han Script Variant Conversion")),
                                   String ("zh_CN,zh_TW,zh_SG,zh_HK,ja_JP"),
                                   String (SCIM_ICONDIR "/hanconv.png"),
                                   String (_("Convert between Simplified Chinese, Traditional Chinese and Japanese shinjitai")));

static std::vector <String> __sc_encodings;
static std::vector <String> __tc_encodings;

#define SCIM_CONFIG_HANCONV_WORK_MODE  "/Filter/HanConv/WorkMode"

// Shared across every instance: see scim_filter_module_init ().
//
// Held through a bare pointer that is deliberately never destroyed at exit.
// This module is dlopen-ed, so a static Pointer<ConfigBase> runs its destructor
// during exit() and unrefs a ConfigBase that has already been torn down -- which
// segfaulted scim-setup on close. scim_module_exit () releases it at the proper
// time when the module is unloaded; if the process simply exits instead, one
// Pointer is leaked, which costs nothing at that point.
static ConfigPointer *__config = 0;
static HanConvWorkMode  __default_work_mode = HANCONV_MODE_OFF;


static Property     __prop_root     (String ("/Filter/HanConv"),
                                     String (_("hanconv")),
                                     String (SCIM_ICONDIR "/hanconv.png"),
                                     String (_("Han script variant conversion")));

// One entry per selectable state. The order here is the order they appear in
// the menu; the key suffix is what trigger_property () matches on.
struct HanConvChoice
{
    HanConvWorkMode  mode;
    const char      *key;
    const char      *label;      // shown in the submenu
    const char      *mode_label; // shown after "hanconv: " on the root
    const char      *icon;
    const char      *tip;
};

static const HanConvChoice __choices [] = {
    { HANCONV_MODE_OFF,       "/Filter/HanConv/Off",
      N_("No Conversion"),            N_("off"),
      SCIM_ICONDIR "/hanconv.png",
      N_("Pass text through unchanged") },
    { HANCONV_MODE_SC_TO_TC,  "/Filter/HanConv/SC-TC",
      N_("Simplified to Traditional"), N_("SC\xe2\x86\x92TC"),
      SCIM_ICONDIR "/hanconv-sc-tc.png",
      N_("Convert Simplified Chinese to Traditional Chinese") },
    { HANCONV_MODE_TC_TO_SC,  "/Filter/HanConv/TC-SC",
      N_("Traditional to Simplified"), N_("TC\xe2\x86\x92SC"),
      SCIM_ICONDIR "/hanconv-tc-sc.png",
      N_("Convert Traditional Chinese to Simplified Chinese") },
    { HANCONV_MODE_TC_TO_JP,  "/Filter/HanConv/TC-JP",
      N_("Traditional to Japanese"),   N_("TC\xe2\x86\x92JP"),
      SCIM_ICONDIR "/hanconv-tc-jp.png",
      N_("Convert Traditional Chinese to Japanese shinjitai") },
    { HANCONV_MODE_JP_TO_TC,  "/Filter/HanConv/JP-TC",
      N_("Japanese to Traditional"),   N_("JP\xe2\x86\x92TC"),
      SCIM_ICONDIR "/hanconv-jp-tc.png",
      N_("Convert Japanese shinjitai to Traditional Chinese") },
    { HANCONV_MODE_SC_TO_JP,  "/Filter/HanConv/SC-JP",
      N_("Simplified to Japanese"),    N_("SC\xe2\x86\x92JP"),
      SCIM_ICONDIR "/hanconv-sc-jp.png",
      N_("Convert Simplified Chinese to Japanese shinjitai") },
    { HANCONV_MODE_JP_TO_SC,  "/Filter/HanConv/JP-SC",
      N_("Japanese to Simplified"),    N_("JP\xe2\x86\x92SC"),
      SCIM_ICONDIR "/hanconv-jp-sc.png",
      N_("Convert Japanese shinjitai to Simplified Chinese") }
};

static const size_t __choice_count = sizeof (__choices) / sizeof (__choices [0]);

//Private functions definition.

static bool       __is_sc_encoding (const String &encoding);
static bool       __is_tc_encoding (const String &encoding);

enum ConvDirection
{
    CONV_NONE = -1,
    CONV_SC_TC = 0,
    CONV_TC_SC,
    CONV_TC_JP,
    CONV_JP_TC,
    CONV_SC_JP,
    CONV_JP_SC,
    CONV_COUNT
};

static ConvDirection __direction_for_mode (HanConvWorkMode mode);
static WideString    __convert (const WideString &src, ConvDirection dir);

static Property   __mark_active (const Property &p, bool active);
static Property   __root_for_mode (HanConvWorkMode mode);


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
        // Drop the configuration reference while the ConfigBase is still alive.
        // Reached when the module is unloaded properly; see __config.
        delete __config;
        __config = 0;
    }

    unsigned int scim_filter_module_init (const ConfigPointer &config)
    {
        // The conversion direction is a user preference, not per-context state:
        // picking it from one window's menu has to hold for the next window too,
        // and survive a restart. Remember the config so trigger_property () can
        // write it back, and seed the shared default from it.
        if (!__config)
            __config = new ConfigPointer (config);
        else
            *__config = config;

        if (!__config->null ())
            __default_work_mode = (HanConvWorkMode) (*__config)->read (
                String (SCIM_CONFIG_HANCONV_WORK_MODE), (int) HANCONV_MODE_OFF);

        return 1;
    }

    FilterFactoryPointer scim_filter_module_create_filter (unsigned int index)
    {
        if (index == 0)
            return new HanConvFilterFactory ();

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

// One lazily-built map per direction. The tables are static data; a map is
// built the first time its direction is actually used, so a filter that is
// attached but left off costs nothing.
struct ConvTable
{
    const UShortPair *pairs;
    UUMap             map;
    bool              ready;
};

static ConvTable __conv [CONV_COUNT] = {
    { __sc_to_tc_table, UUMap (), false },
    { __tc_to_sc_table, UUMap (), false },
    { __tc_to_jp_table, UUMap (), false },
    { __jp_to_tc_table, UUMap (), false },
    { __sc_to_jp_table, UUMap (), false },
    { __jp_to_sc_table, UUMap (), false }
};

// Which table a work mode converts through. The FORCE_* modes exist only to
// satisfy a client encoding and convert exactly like their plain counterparts.
static ConvDirection
__direction_for_mode (HanConvWorkMode mode)
{
    switch (mode) {
        case HANCONV_MODE_SC_TO_TC:
        case HANCONV_MODE_FORCE_SC_TO_TC: return CONV_SC_TC;
        case HANCONV_MODE_TC_TO_SC:
        case HANCONV_MODE_FORCE_TC_TO_SC: return CONV_TC_SC;
        case HANCONV_MODE_TC_TO_JP:       return CONV_TC_JP;
        case HANCONV_MODE_JP_TO_TC:       return CONV_JP_TC;
        case HANCONV_MODE_SC_TO_JP:       return CONV_SC_JP;
        case HANCONV_MODE_JP_TO_SC:       return CONV_JP_SC;
        default:                          return CONV_NONE;
    }
}

static WideString
__convert (const WideString &src, ConvDirection dir)
{
    if (dir == CONV_NONE)
        return src;

    ConvTable &t = __conv [dir];

    if (!t.ready) {
        for (size_t i = 0; t.pairs [i].first; ++i)
            t.map [t.pairs [i].first] = t.pairs [i].second;
        t.ready = true;
    }

    WideString out;
    UUMap::const_iterator mapit;

    for (WideString::const_iterator sit = src.begin (); sit != src.end (); ++sit) {
        // The tables are BMP-only; anything above stays as it is.
        if (*sit <= 0xFFFF) {
            mapit = t.map.find ((unsigned short) (*sit));
            if (mapit != t.map.end ()) {
                out.push_back (static_cast<ucs4_t> (mapit->second));
                continue;
            }
        }
        out.push_back (*sit);
    }

    return out;
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

//Implementation of HanConvFilterFactory.
HanConvFilterFactory::HanConvFilterFactory ()
    : m_sc_ok(false),
      m_tc_ok(false)
{
}

void
HanConvFilterFactory::attach_imengine_factory (const IMEngineFactoryPointer &orig)
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

    // The probes above ask whether the engine can emit a Chinese codepage,
    // which used to be the only way to ask what script it produces. Engines are
    // UTF-8 now and declare no codepages -- validate_encoding () answers true
    // for UTF-8 and consults a list that scim-tables never fills -- so the
    // probes always fail and this filter quietly disables itself by handing
    // back an unwrapped instance from create_instance ().
    //
    // Nothing about the conversion needs an encoding: the tables are Unicode
    // (see __sc_to_tc_table) and the filter only ever sees WideString. And the
    // user attached this filter to this engine on purpose, so which scripts the
    // engine is expected to produce is not ours to second-guess -- a raw-code
    // engine that emits a Han character should convert like any other. Enable
    // both directions for any UTF-8 engine and leave it to the work mode, which
    // starts OFF, to decide whether anything is actually converted.
    const bool declares_chinese_codepage = (m_sc_ok || m_tc_ok);

    if (!m_sc_ok && !m_tc_ok && orig->validate_encoding (String ("UTF-8"))) {
        m_sc_ok = true;
        m_tc_ok = true;
    }

    // Only an engine that actually declared a Chinese codepage gets the Chinese
    // locales grafted on. Doing it for every UTF-8 engine would have a filtered
    // Thai or Greek engine claiming to support zh_TW, which is a lie the engine
    // lists and language menus would then repeat.
    if (declares_chinese_codepage) {
        String locales = orig->get_locales ();
        locales = locales + String (",") + scim_get_language_locales ("zh_CN");
        locales = locales + String (",") + scim_get_language_locales ("zh_TW");
        locales = locales + String (",") + scim_get_language_locales ("zh_SG");
        locales = locales + String (",") + scim_get_language_locales ("zh_HK");
        set_locales (locales);
    }
}

WideString
HanConvFilterFactory::get_name () const
{
     WideString name = FilterFactoryBase::get_name ();
     return name.length () ? name : utf8_mbstowcs (__filter_info.name);
}

String
HanConvFilterFactory::get_uuid () const
{
    String uuid = FilterFactoryBase::get_uuid ();
    return uuid.length () ? uuid : __filter_info.uuid;
}

String
HanConvFilterFactory::get_icon_file () const
{
    String icon = FilterFactoryBase::get_icon_file ();
    return icon.length () ? icon : __filter_info.icon;
}

WideString
HanConvFilterFactory::get_authors () const
{
    WideString authors = FilterFactoryBase::get_authors ();
    return authors.length () ? authors : utf8_mbstowcs (_("James Su <suzhe@tsinghua.org.cn>"));
}

WideString
HanConvFilterFactory::get_help () const
{
    // No help yet.
    WideString help = FilterFactoryBase::get_help ();
    return help;
}

bool
HanConvFilterFactory::validate_encoding (const String& encoding) const
{
    // Bypass the original IMEngineFactory.
    return IMEngineFactoryBase::validate_encoding (encoding);
}

bool
HanConvFilterFactory::validate_locale (const String& locale) const
{
    // Bypass the original IMEngineFactory.
    return IMEngineFactoryBase::validate_locale (locale);
}

IMEngineInstancePointer
HanConvFilterFactory::create_instance (const String& encoding, int id)
{
    if (m_sc_ok || m_tc_ok) {
        // Start from the user's chosen direction, not from off; the FORCE_*
        // cases below still override it when the encodings demand it.
        HanConvWorkMode mode = __default_work_mode;

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
                    mode = HANCONV_MODE_FORCE_TC_TO_SC;
                }
            } else if (__is_tc_encoding (encoding)) {
                if (FilterFactoryBase::validate_encoding (m_tc_encoding)) {
                    orig_encoding = m_tc_encoding;
                } else {
                    orig_encoding = m_sc_encoding;
                    mode = HANCONV_MODE_FORCE_SC_TO_TC;
                }
            }
        } else if ((__is_sc_encoding (encoding) && !FilterFactoryBase::validate_encoding (m_tc_encoding)) ||
                   (__is_tc_encoding (encoding) && !FilterFactoryBase::validate_encoding (m_sc_encoding))) {
            mode = HANCONV_MODE_FORCE_OFF;
        }

        return new HanConvFilterInstance (this, mode, encoding, FilterFactoryBase::create_instance (orig_encoding, id));
    }

    return FilterFactoryBase::create_instance (encoding, id);
}

//Implementation of HanConvFilterInstance
HanConvFilterInstance::HanConvFilterInstance (HanConvFilterFactory *factory, const HanConvWorkMode &mode, const String &client_encoding, const IMEngineInstancePointer &orig_inst)
    : FilterInstanceBase (factory, orig_inst),
      m_factory (factory),
      m_props_registered (false),
      m_work_mode (mode)
{
    IMEngineInstanceBase::set_encoding (client_encoding);
}

bool
HanConvFilterInstance::set_encoding (const String &encoding)
{
    if (m_work_mode == HANCONV_MODE_SC_TO_TC || m_work_mode == HANCONV_MODE_FORCE_SC_TO_TC) {
        if (__is_tc_encoding (encoding))
            FilterInstanceBase::set_encoding (m_factory->m_sc_encoding);
    } else if (m_work_mode == HANCONV_MODE_TC_TO_SC || m_work_mode == HANCONV_MODE_FORCE_TC_TO_SC) {
        if (__is_sc_encoding (encoding))
            FilterInstanceBase::set_encoding (m_factory->m_tc_encoding);
    } else {
        FilterInstanceBase::set_encoding (encoding);
    }
    reset ();
    return IMEngineInstanceBase::set_encoding (encoding);
}

void
HanConvFilterInstance::focus_in ()
{
    m_props_registered = false;

    FilterInstanceBase::focus_in ();

    if (!m_props_registered) {
        PropertyList props;
        filter_register_properties (props); 
    }
}

void
HanConvFilterInstance::trigger_property (const String &property)
{
    const HanConvChoice *chosen = 0;

    for (size_t i = 0; i < __choice_count; ++i)
        if (property == String (__choices [i].key))
            chosen = &__choices [i];

    if (!chosen) {
        FilterInstanceBase::trigger_property (property);
        return;
    }

    // A forced mode was imposed by the client's encoding, not chosen.
    if (m_work_mode == HANCONV_MODE_FORCE_SC_TO_TC ||
        m_work_mode == HANCONV_MODE_FORCE_TC_TO_SC ||
        m_work_mode == HANCONV_MODE_FORCE_OFF)
        return;

    if (chosen->mode == m_work_mode)
        return;

    m_work_mode = chosen->mode;

    // Share the choice with every other context and remember it for next time:
    // it is a user preference, not state belonging to this one input context,
    // and picking it in one window has to hold in the next.
    __default_work_mode = m_work_mode;

    if (__config && !__config->null ()) {
        (*__config)->write (String (SCIM_CONFIG_HANCONV_WORK_MODE), (int) m_work_mode);
        (*__config)->flush ();
    }

    set_encoding (get_encoding ());

    // Refresh each entry in place. A panel treats register_properties () as the
    // initial build of the menu and ignores a repeat while that menu already
    // exists -- which left the check mark on the old entry until the engine was
    // re-selected -- whereas update_property () is the path built for exactly
    // this, changing one entry that is already on screen.
    update_property (__root_for_mode (m_work_mode));

    for (size_t i = 0; i < __choice_count; ++i) {
        Property prop (String (__choices [i].key),
                       String (_(__choices [i].label)),
                       String (__choices [i].icon),
                       String (_(__choices [i].tip)));
        update_property (__mark_active (prop, m_work_mode == __choices [i].mode));
    }
}

void
HanConvFilterInstance::filter_update_preedit_string (const WideString    &str,
                                                  const AttributeList &attrs)
{
    WideString nstr = __convert (str, __direction_for_mode (m_work_mode));

    update_preedit_string (nstr, attrs);
}

void
HanConvFilterInstance::filter_update_aux_string (const WideString    &str,
                                              const AttributeList &attrs)
{
    WideString nstr = __convert (str, __direction_for_mode (m_work_mode));

    update_aux_string (nstr, attrs);
}

void
HanConvFilterInstance::filter_update_lookup_table (const LookupTable &table)
{
    if (m_work_mode == HANCONV_MODE_OFF) {
        update_lookup_table (table);
    } else {
        CommonLookupTable ntable;
        std::vector<WideString> labels;
        size_t i;

        // Can be paged up.
        if (table.get_current_page_start ())
            ntable.append_candidate (0x3400);

        const ConvDirection dir = __direction_for_mode (m_work_mode);

        for (i = 0; i < table.get_current_page_size (); ++i) {
            ntable.append_candidate (__convert (table.get_candidate_in_current_page (i), dir),
                                     table.get_attributes_in_current_page (i));
            labels.push_back (__convert (table.get_candidate_label (i), dir));
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
HanConvFilterInstance::filter_commit_string (const WideString &str)
{
    WideString nstr = __convert (str, __direction_for_mode (m_work_mode));

    commit_string (nstr);
}

// The root doubles as an at-a-glance indicator for panels that show only the
// submenu title. Built in one place so the register and update paths cannot
// drift apart.
static const HanConvChoice *
__choice_for_mode (HanConvWorkMode mode)
{
    for (size_t i = 0; i < __choice_count; ++i)
        if (__choices [i].mode == mode)
            return &__choices [i];

    return 0;
}

// The root reads "<filter>: <mode>", so the current state is legible without
// opening the submenu. A forced mode has no entry of its own; show the
// direction it was forced into.
static Property
__root_for_mode (HanConvWorkMode mode)
{
    Property root = __prop_root;

    HanConvWorkMode shown = mode;

    if (mode == HANCONV_MODE_FORCE_OFF)             shown = HANCONV_MODE_OFF;
    else if (mode == HANCONV_MODE_FORCE_SC_TO_TC)   shown = HANCONV_MODE_SC_TO_TC;
    else if (mode == HANCONV_MODE_FORCE_TC_TO_SC)   shown = HANCONV_MODE_TC_TO_SC;

    const HanConvChoice *c = __choice_for_mode (shown);

    if (c) {
        root.set_label (__prop_root.get_label () + String (": ") + String (_(c->mode_label)));
        root.set_icon  (String (c->icon));
        root.set_tip   (String (_(c->tip)));
    }

    return root;
}

// scim properties carry no checked state -- the frontends have nothing to draw
// a radio mark from -- so the only way to show which direction is active is in
// the label itself.
static Property
__mark_active (const Property &p, bool active)
{
    Property prop = p;

    if (active)
        prop.set_label (String ("\xE2\x9C\x93 ") + p.get_label ());

    return prop;
}

void
HanConvFilterInstance::filter_register_properties (const PropertyList &properties)
{
    PropertyList props;

    // The engine's own labels are converted too, so a menu shown next to
    // converted text reads in the same script.
    const ConvDirection dir = __direction_for_mode (m_work_mode);

    for (size_t i = 0; i < properties.size (); ++i) {
        Property prop = properties [i];
        prop.set_label (utf8_wcstombs (__convert (utf8_mbstowcs (prop.get_label ()), dir)));
        prop.set_tip   (utf8_wcstombs (__convert (utf8_mbstowcs (prop.get_tip ()), dir)));
        props.push_back (prop);
    }

    props.push_back (__root_for_mode (m_work_mode));

    // A forced mode is not the user's to change, so offer no choices at all
    // rather than entries that would silently refuse.
    if (m_work_mode != HANCONV_MODE_FORCE_OFF &&
        m_work_mode != HANCONV_MODE_FORCE_SC_TO_TC &&
        m_work_mode != HANCONV_MODE_FORCE_TC_TO_SC) {
        for (size_t i = 0; i < __choice_count; ++i) {
            Property prop (String (__choices [i].key),
                           String (_(__choices [i].label)),
                           String (__choices [i].icon),
                           String (_(__choices [i].tip)));
            props.push_back (__mark_active (prop, m_work_mode == __choices [i].mode));
        }
    }

    register_properties (props);

    m_props_registered = true;
}

void
HanConvFilterInstance::filter_update_property (const Property &property)
{
    Property prop = property;

    const ConvDirection dir = __direction_for_mode (m_work_mode);

    prop.set_label (utf8_wcstombs (__convert (utf8_mbstowcs (prop.get_label ()), dir)));
    prop.set_tip   (utf8_wcstombs (__convert (utf8_mbstowcs (prop.get_tip ()), dir)));

    update_property (prop);
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
