/** @file scim_rawcode_imengine.cpp
 * implementation of class RawCodeInstance.
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
 * $Id: scim_rawcode_imengine.cpp,v 1.7.2.7 2007/04/10 07:47:18 suzhe Exp $
 *
 */

#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_C_STRING
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#include "scim_private.h"
#include "scim.h"
#include "scim_rawcode_imengine.h"

#define scim_module_init rawcode_LTX_scim_module_init
#define scim_module_exit rawcode_LTX_scim_module_exit
#define scim_imengine_module_init rawcode_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory rawcode_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_RAWCODE_LOCALES   "/IMEngine/RawCode/Locales"
#define SCIM_PROP_RAWCODE_ENCODING             "/IMEngine/RawCode/Encoding"

#define SCIM_RAWCODE_ICON_FILE                 (SCIM_ICONDIR "/rawcode.png")

using namespace scim;

static Pointer <RawCodeFactory> __rawcode_factory;

static String __rawcode_locales;

static std::vector<String> __rawcode_encodings;

extern "C" {
    void scim_module_init (void)
    {
        __rawcode_locales = String (
            "zh_CN.GB18030,zh_CN.GBK,zh_CN.GB2312,zh_TW,zh_TW.EUC-TW,zh_HK,"
            "ja_JP,ja_JP.sjis,ko_KR,en_US.UTF-8");
    }

    void scim_module_exit (void)
    {
        __rawcode_factory.reset ();
    }

    unsigned int scim_imengine_module_init (const ConfigPointer &config)
    {
        if (!config.null ()) {
            String locales = config->read (String (SCIM_CONFIG_IMENGINE_RAWCODE_LOCALES), String ("default"));
            if (locales != "default")
                __rawcode_locales = locales;
        }

        std::vector <String> locale_list;

        scim_split_string_list (locale_list, __rawcode_locales);

        for (size_t i=0; i<locale_list.size (); ++i) {
            locale_list [i] = scim_validate_locale (locale_list [i]);
            if (locale_list [i].length ())
                __rawcode_encodings.push_back (scim_get_locale_encoding (locale_list [i]));
        }

        std::sort (__rawcode_encodings.begin (), __rawcode_encodings.end ());
        __rawcode_encodings.erase (std::unique (__rawcode_encodings.begin (), __rawcode_encodings.end ()), __rawcode_encodings.end ());

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (unsigned int factory)
    {
        String languages;

        if (factory != 0) return NULL;

        if (__rawcode_factory.null ())
            __rawcode_factory = new RawCodeFactory ();

        return __rawcode_factory;
    }
}

// implementation of RawCode
RawCodeFactory::RawCodeFactory ()
{
    set_locales (__rawcode_locales);
}

RawCodeFactory::~RawCodeFactory ()
{
}

WideString
RawCodeFactory::get_name () const
{
    return utf8_mbstowcs (_("RAW CODE"));
}

WideString
RawCodeFactory::get_authors () const
{
    return utf8_mbstowcs (String (
                _("(C) 2002-2006 James Su <suzhe@tsinghua.org.cn>")));
}

WideString
RawCodeFactory::get_credits () const
{
    return WideString ();
}

WideString
RawCodeFactory::get_help () const
{
    return utf8_mbstowcs (String (_(
                            "Hot Keys:\n\n"
                            "  Control+u:\n"
                            "    switch between Multibyte encoding and Unicode.\n\n"
                            "  Esc:\n"
                            "    reset the input method.\n")));
}

String
RawCodeFactory::get_uuid () const
{
    return String ("6e029d75-ef65-42a8-848e-332e63d70f9c");
}

String
RawCodeFactory::get_icon_file () const
{
    return String (SCIM_RAWCODE_ICON_FILE);
}

String
RawCodeFactory::get_language () const
{
    return scim_validate_language ("other");
}

IMEngineInstancePointer
RawCodeFactory::create_instance (const String& encoding, int id)
{
    return new RawCodeInstance (this, encoding, id);
}

int
RawCodeFactory::get_maxlen (const String &encoding)
{
    if (encoding == "UTF-8") return 4;
    if (encoding == "Unicode") return 0;

    std::vector <String> locales;

    scim_split_string_list (locales, get_locales ());

    for (size_t i=0; i<locales.size (); ++i)
        if (scim_get_locale_encoding (locales [i]) == encoding)
            return scim_get_locale_maxlen (locales [i]);

    return 0;
}

// implementation of RawCodeInstance
RawCodeInstance::RawCodeInstance (RawCodeFactory *factory,
                                  const String& encoding,
                                  int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory)
{
    if (!m_client_iconv.set_encoding (encoding))
        m_client_iconv.set_encoding ("UTF-8");

    set_working_encoding ("Unicode");
}

RawCodeInstance::~RawCodeInstance ()
{
}

bool
RawCodeInstance::process_key_event (const KeyEvent& key)
{
    if (key.is_key_release ()) return true;

    // switch between unicode and native code mode
    if ((key.code == SCIM_KEY_u || key.code == SCIM_KEY_U) && key.is_control_down ()) {
        if (m_unicode)
            set_working_encoding (get_encoding ());
        else
            set_working_encoding ("Unicode");

        reset ();
        return true;
    }

    //reset key
    if (key.code == SCIM_KEY_Escape && key.mask == 0) {
        reset ();
        return true;
    }

    //delete key
    if (key.code == SCIM_KEY_BackSpace && key.mask == 0 &&
        m_preedit_string.size () != 0) {
        m_preedit_string.erase (m_preedit_string.length () - 1, 1);
        update_preedit_string (m_preedit_string);
        update_preedit_caret (m_preedit_string.length ());
        process_preedit_string ();
        return true;
    }

    // valid keys
    if (((key.code >= SCIM_KEY_0 && key.code <= SCIM_KEY_9) ||
         (key.code >= SCIM_KEY_A && key.code <= SCIM_KEY_F) ||
         (key.code >= SCIM_KEY_a && key.code <= SCIM_KEY_f)) &&
        (key.mask == 0 || key.is_shift_down ()) &&
        m_preedit_string.length () < m_max_preedit_len) {

        if (m_preedit_string.length () == 0)
            show_preedit_string ();

        ucs4_t ascii = (ucs4_t) tolower (key.get_ascii_code ());
        m_preedit_string.push_back (ascii);
        update_preedit_string (m_preedit_string);
        update_preedit_caret (m_preedit_string.length ());
        process_preedit_string ();
        return true;
    }

    // commit key
    if (key.code == SCIM_KEY_space && key.mask == 0 && m_preedit_string.length ()
        && m_lookup_table.number_of_candidates ()) {
        WideString str = m_lookup_table.get_candidate_label (0);
        // It's already a complete char, commit it.
        if (str.length () && str [0] == 0x20) {
            commit_string (m_lookup_table.get_candidate_in_current_page (0));
            reset ();
            return true;
        }
    }

    //page up key.
    if ((key.code == SCIM_KEY_comma || key.code == SCIM_KEY_minus || key.code == SCIM_KEY_bracketleft
         || key.code == SCIM_KEY_Page_Up) && key.mask == 0)
        lookup_table_page_up ();

    //page down key.
    if ((key.code == SCIM_KEY_period || key.code == SCIM_KEY_equal || key.code == SCIM_KEY_bracketright
         || key.code == SCIM_KEY_Page_Down) && key.mask == 0)
        lookup_table_page_down ();

    //other keys is not allowed when preediting
    if (m_preedit_string.length ())
        return true;

    return false;
}

void
RawCodeInstance::select_candidate (unsigned int item)
{
    WideString label = m_lookup_table.get_candidate_label (item);
    KeyEvent key ((int) label [0], 0);
    process_key_event (key);
}

void
RawCodeInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0)
        m_lookup_table.set_page_size (page_size);
}

void
RawCodeInstance::lookup_table_page_up ()
{
    if (m_preedit_string.length () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.page_up ();
        m_lookup_table.set_page_size (m_lookup_table.number_of_candidates ());

        m_lookup_table.set_candidate_labels (
            std::vector <WideString> (
                m_lookup_table_labels.begin () + m_lookup_table.get_current_page_start (),
                m_lookup_table_labels.end ()));

        update_lookup_table (m_lookup_table);
    }
}

void
RawCodeInstance::lookup_table_page_down ()
{
    if (m_preedit_string.length () && m_lookup_table.number_of_candidates ()) {
        m_lookup_table.page_down ();
        m_lookup_table.set_page_size (m_lookup_table.number_of_candidates ());

        m_lookup_table.set_candidate_labels (
            std::vector <WideString> (
                m_lookup_table_labels.begin () + m_lookup_table.get_current_page_start (),
                m_lookup_table_labels.end ()));

        update_lookup_table (m_lookup_table);
    }
}

void
RawCodeInstance::move_preedit_caret (unsigned int /*pos*/)
{
}

void
RawCodeInstance::reset ()
{
    if (!m_client_iconv.set_encoding (get_encoding ()))
        m_client_iconv.set_encoding ("UTF-8");

    m_preedit_string = WideString ();

    m_lookup_table.clear ();

    hide_lookup_table ();
    hide_preedit_string ();
}

void
RawCodeInstance::set_working_encoding (const String &encoding)
{
    unsigned int maxlen = m_factory->get_maxlen (encoding);

    if (maxlen && encoding != "Unicode" && m_working_iconv.set_encoding (encoding)){
        m_unicode = false;
        m_max_preedit_len = maxlen * 2;
        m_working_encoding = encoding;
    } else {
        m_unicode = true;
        m_working_encoding = "Unicode";
        m_max_preedit_len = 6;
    }

    refresh_encoding_property ();
}

void
RawCodeInstance::focus_in ()
{
    initialize_properties ();

    if (m_preedit_string.length ()) {
        update_preedit_string (m_preedit_string);
        update_preedit_caret (m_preedit_string.length ());
        show_preedit_string ();
        if (m_lookup_table.number_of_candidates ()) {
            update_lookup_table (m_lookup_table);
            show_lookup_table ();
        }
    }
}

void
RawCodeInstance::focus_out ()
{
    reset ();
}

void
RawCodeInstance::trigger_property (const String &property)
{
    if (property.substr (0, strlen (SCIM_PROP_RAWCODE_ENCODING)) == SCIM_PROP_RAWCODE_ENCODING) {
        set_working_encoding (property.substr (strlen (SCIM_PROP_RAWCODE_ENCODING) + 1));
        reset ();
    }
}

void
RawCodeInstance::initialize_properties ()
{
    PropertyList proplist;

    proplist.push_back (Property (SCIM_PROP_RAWCODE_ENCODING,
                                  _(m_working_encoding.c_str ()),
                                  "",
                                  _("The status of the current input method. Click to change it.")));

    proplist.push_back (Property (String (SCIM_PROP_RAWCODE_ENCODING) + String ("/Unicode"), _("Unicode")));

    for (size_t i = 0; i < __rawcode_encodings.size (); ++i)
        proplist.push_back (Property (String (SCIM_PROP_RAWCODE_ENCODING) + String ("/") + __rawcode_encodings [i], _(__rawcode_encodings [i].c_str ())));

    register_properties (proplist);
}

void
RawCodeInstance::refresh_encoding_property ()
{
    update_property (Property (SCIM_PROP_RAWCODE_ENCODING,
                               _(m_working_encoding.c_str ()),
                               "",
                               _("The status of the current input method. Click to change it.")));
}

void
RawCodeInstance::process_preedit_string ()
{
    if (m_preedit_string.length () == 0) {
        hide_preedit_string ();
        hide_lookup_table ();
        return;
    }

    if (m_unicode) {
        size_t maxlen = 6;

        if (m_preedit_string.length () > 0) {
            if (m_preedit_string [0] == '0')
                maxlen = 4;
            else if (m_preedit_string [0] == '1')
                maxlen = 6;
            else
                maxlen = 5;
        }

        if (m_preedit_string.length () >= 3 && m_preedit_string.length () < maxlen && create_lookup_table () > 0) {
            update_lookup_table (m_lookup_table);
        } else if (m_preedit_string.length () == maxlen) {
            WideString str;
            ucs4_t code = get_unicode_value (m_preedit_string);

            m_preedit_string = WideString ();
            m_lookup_table.clear ();
            hide_preedit_string ();

            // If code is valid under current encoding,
            // then commit it.
            if (m_client_iconv.test_convert (&code, 1) && code > 0 && code < 0x10FFFF) {
                str.push_back (code);
                commit_string (str);
            }
        } else if (m_lookup_table.number_of_candidates ()){
            m_lookup_table.clear ();
        }
    } else {
        String code = get_multibyte_string (m_preedit_string);
        WideString wstr;

        // convert ok, then commit.
        if (m_working_iconv.convert (wstr, code) && wstr.length () > 0 && wstr [0] >= 128 && m_client_iconv.test_convert (wstr)) {
            m_preedit_string = WideString ();
            m_lookup_table.clear ();

            hide_preedit_string ();
            commit_string (wstr);
        } else if (create_lookup_table () > 0) {
            update_lookup_table (m_lookup_table);
        }
    }

    if (m_lookup_table.number_of_candidates ())
        show_lookup_table ();
    else
        hide_lookup_table ();
}

inline static int ascii_to_hex (int ascii)
{
    if (ascii >= '0' && ascii <= '9')
        return ascii - '0';
    else if (ascii >= 'a' && ascii <= 'f')
        return ascii - 'a' + 10;
    else if (ascii >= 'A' && ascii <= 'F')
        return ascii - 'A' + 10;
    return 0;
}
inline static int hex_to_ascii (int hex)
{
    hex %= 16;

    if (hex >= 0 && hex <= 9)
        return hex + '0';

    return hex - 10 + 'a';
}

String
RawCodeInstance::get_multibyte_string (const WideString & preedit)
{
    String str;
    char ch = 0;

    if (preedit.length () == 0)
        return str;

    for (size_t i=0; i<preedit.length (); ++i) {
        if (i % 2 == 0)
            ch = (char) ascii_to_hex ((int) preedit [i]) & 0x0f;
        else {
            ch = (ch << 4) | ((char) ascii_to_hex ((int) preedit [i]) & 0x0f);
            str.push_back (ch);
            ch = 0;
        }
    }

    if (ch != 0)
        str.push_back (ch);

    return str;
}

ucs4_t
RawCodeInstance::get_unicode_value (const WideString &preedit)
{
    ucs4_t code = 0;
    for (size_t i=0; i<preedit.length (); ++i) {
        code = (code << 4) | (ascii_to_hex ((int) preedit [i]) & 0x0f);
    }
    return code;
}

int
RawCodeInstance::create_lookup_table ()
{
    String mbs_code;
    ucs4_t ucs_code;
    WideString trail;
    WideString wstr;

    m_lookup_table.clear ();
    m_lookup_table_labels.clear ();

    trail.push_back (0x20);

    if (m_unicode) {
        ucs_code = get_unicode_value (m_preedit_string);
        if (m_client_iconv.test_convert (&ucs_code, 1) && ucs_code > 0 && ucs_code < 0x10FFFF) {
            m_lookup_table_labels.push_back (trail);
            m_lookup_table.append_candidate (ucs_code);
        }
    }

    for (int i=0; i<16; ++i) {
        trail [0] = (ucs4_t) hex_to_ascii (i);
        if (m_unicode) {
            ucs_code = get_unicode_value (m_preedit_string + trail);
            if (m_client_iconv.test_convert (&ucs_code, 1) && ucs_code > 0 && ucs_code < 0x10FFFF) {
                m_lookup_table_labels.push_back (trail);
                m_lookup_table.append_candidate (ucs_code);
            }
        } else {
            mbs_code = get_multibyte_string (m_preedit_string + trail);
            if (m_working_iconv.convert (wstr, mbs_code) && wstr.length () > 0 && wstr [0] >= 128 && m_client_iconv.test_convert (wstr)) {
                m_lookup_table_labels.push_back (trail);
                m_lookup_table.append_candidate (wstr);
            }
        }
    }

    m_lookup_table.set_page_size (m_lookup_table_labels.size ());
    m_lookup_table.set_candidate_labels (m_lookup_table_labels);

    return m_lookup_table_labels.size ();
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
