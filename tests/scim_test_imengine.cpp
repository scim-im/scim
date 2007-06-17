/** @file scim_test_imengine.cpp
 * implementation of class TestInstance.
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
 * $Id: scim_test_imengine.cpp,v 1.6 2005/05/11 09:52:10 suzhe Exp $
 *
 */

#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_TRANS_COMMANDS
#include "scim_private.h"
#include "scim.h"
#include "scim_test_imengine.h"

#define scim_module_init test_imengine_LTX_scim_module_init
#define scim_module_exit test_imengine_LTX_scim_module_exit
#define scim_imengine_module_init test_imengine_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory test_imengine_LTX_scim_imengine_module_create_factory

#define SCIM_PROP_STATUS                       "/IMEngine/Test/Status"

#define SCIM_TEST_ICON_FILE                 (SCIM_ICONDIR "/keyboard.png")

#define TEST_HELPER                            "5d82379b-ba6e-4adc-9d81-ca49d1791b55"

using namespace scim;

static Pointer <TestFactory> _scim_test_factory;
static ConfigPointer _scim_config;

static const char * _DEFAULT_LANGUAGES = N_(
    "zh_CN,zh_TW,zh_HK,zh_SG,ja_JP,ko_KR");

extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
        _scim_test_factory.reset ();
        _scim_config.reset ();
    }

    unsigned int scim_imengine_module_init (const ConfigPointer &config)
    {
        _scim_config = config;
        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (unsigned int factory)
    {
        String languages;

        if (factory != 0) return NULL;

        if (_scim_test_factory.null ()) {
            _scim_test_factory =
                new TestFactory ();
        }
        return _scim_test_factory;
    }
}

// implementation of Test
TestFactory::TestFactory ()
{
    set_languages (String (_(_DEFAULT_LANGUAGES)));
}

TestFactory::~TestFactory ()
{
}

WideString
TestFactory::get_name () const
{
    return utf8_mbstowcs (_("TEST"));
}

WideString
TestFactory::get_authors () const
{
    return utf8_mbstowcs (String (
                _("(C) 2002-2004 James Su <suzhe@tsinghua.org.cn>")));
}

WideString
TestFactory::get_credits () const
{
    return WideString ();
}

WideString
TestFactory::get_help () const
{
    return WideString ();
}

String
TestFactory::get_uuid () const
{
    return String ("29904635-8afa-4e21-9138-0edb556150e7");
}

String
TestFactory::get_icon_file () const
{
    return String (SCIM_TEST_ICON_FILE);
}

String
TestFactory::get_language () const
{
    return scim_validate_language ("other");
}

IMEngineInstancePointer
TestFactory::create_instance (const String& encoding, int id)
{
    return new TestInstance (this, encoding, id);
}

// implementation of TestInstance
TestInstance::TestInstance (TestFactory *factory,
                            const String& encoding,
                            int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_helper_started (false),
      m_focused (false),
      m_work (false)
{
}

TestInstance::~TestInstance ()
{
    if (m_helper_started) {
        std::cerr << "TestInstance::~TestInstance () stop_helper\n";
        stop_helper (TEST_HELPER);
    }
}

bool
TestInstance::process_key_event (const KeyEvent& key)
{
    if (!m_focused) return false;

    if (key.is_key_release ()) return true;

    KeyEvent newkey = key.map_to_layout (SCIM_KEYBOARD_Default);

    if (newkey.code == SCIM_KEY_F12) {
        m_work = !m_work;
        return true;
    }

    if (m_work) {
        commit_string (utf8_mbstowcs (newkey.get_key_string () + String ("\n")));
    } else {
        m_trans.clear ();
        m_trans.put_command (SCIM_TRANS_CMD_REQUEST);
        m_trans.put_command (SCIM_TRANS_CMD_PROCESS_KEY_EVENT);
        m_trans.put_data (newkey);
 
        WideString text;
        int cursor;
 
        get_surrounding_text (text, cursor);
        m_trans.put_command (SCIM_TRANS_CMD_GET_SURROUNDING_TEXT);
        m_trans.put_data (text);
 
        send_helper_event (TEST_HELPER, m_trans);
        return false;
    }

    return true;
}

void
TestInstance::process_helper_event (const String &helper_uuid, const Transaction &trans)
{
    std::cerr << "TestInstance::process_helper_event ()\n";

    TransactionReader reader (trans);

    if (helper_uuid == TEST_HELPER) {
        int cmd;
        if (reader.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST &&
            reader.get_command (cmd)) {
            if (cmd == SCIM_TRANS_CMD_START_HELPER) {
                m_helper_started = true;
                if (m_focused) {
                    m_trans.clear ();
                    m_trans.put_command (SCIM_TRANS_CMD_REQUEST);
                    m_trans.put_command (SCIM_TRANS_CMD_FOCUS_IN);
                    send_helper_event (TEST_HELPER, m_trans);
                }
            } else if (cmd == SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT) {
                delete_surrounding_text (-1,2);
                m_trans.clear ();
                m_trans.put_command (SCIM_TRANS_CMD_REQUEST);
                WideString text;
                int cursor;
 
                get_surrounding_text (text, cursor);
                m_trans.put_command (SCIM_TRANS_CMD_GET_SURROUNDING_TEXT);
                m_trans.put_data (text);
 
                send_helper_event (TEST_HELPER, m_trans);
            }
        }
    }
}

void
TestInstance::reset ()
{
}

void
TestInstance::focus_in ()
{
    std::cerr << "TestInstance::focus_in\n";
    m_focused = true;

    if (m_helper_started) {
        m_trans.clear ();
        m_trans.put_command (SCIM_TRANS_CMD_REQUEST);
        m_trans.put_command (SCIM_TRANS_CMD_FOCUS_IN);
        std::cerr << "  send_helper_event (" << TEST_HELPER ")\n";
        send_helper_event (TEST_HELPER, m_trans);
    } else {
        std::cerr << "  start_helper (" << TEST_HELPER ")\n";
        start_helper (TEST_HELPER);
    }
}

void
TestInstance::focus_out ()
{
    std::cerr << "TestInstance::focus_out\n";

    m_focused = false;

    if (m_helper_started) {
        m_trans.clear ();
        m_trans.put_command (SCIM_TRANS_CMD_REQUEST);
        m_trans.put_command (SCIM_TRANS_CMD_FOCUS_OUT);
        std::cerr << "  send_helper_event (" << TEST_HELPER ")\n";
        send_helper_event (TEST_HELPER, m_trans);
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
