/** @file scim_test_helper.cpp
 * implementation of Test Helper module.
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
 * $Id: scim_test_helper.cpp,v 1.3 2005/01/15 11:34:03 suzhe Exp $
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_MODULE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HELPER
#define Uses_STL_MAP
#include "scim_private.h"
#include "scim.h"

#include <gtk/gtk.h>

#define scim_module_init test_helper_LTX_scim_module_init
#define scim_module_exit test_helper_LTX_scim_module_exit
#define scim_helper_module_number_of_helpers test_helper_LTX_scim_helper_module_number_of_helpers
#define scim_helper_module_get_helper_info test_helper_LTX_scim_helper_module_get_helper_info
#define scim_helper_module_run_helper test_helper_LTX_scim_helper_module_run_helper

using namespace scim;

static HelperInfo __helper_info (String ("5d82379b-ba6e-4adc-9d81-ca49d1791b55"),
                                 String (_("Test")),
                                 String (""),
                                 String (_("A test helper.")),
                                 SCIM_HELPER_AUTO_RESTART|SCIM_HELPER_NEED_SCREEN_INFO|SCIM_HELPER_NEED_SPOT_LOCATION_INFO);

static void run (const String &display);

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
    }

    unsigned int scim_helper_module_number_of_helpers (void)
    {
        return 1;
    }

    bool scim_helper_module_get_helper_info (unsigned int idx, HelperInfo &info)
    {
        if (idx == 0) {
            info = __helper_info; 
            return true;
        }
        return false;
    }

    void scim_helper_module_run_helper (const String &uuid, const ConfigPointer &config, const String &display)
    {
        SCIM_DEBUG_MAIN(1) << "test_LTX_scim_helper_module_run_helper ()\n";

        if (uuid == "5d82379b-ba6e-4adc-9d81-ca49d1791b55") {
            run (display);
        }

        SCIM_DEBUG_MAIN(1) << "exit test_LTX_scim_helper_module_run_helper ()\n";
    }
}

static GtkWidget *main_window;
static GtkWidget *keystring;
static GtkWidget *surrounding;

static int        cur_ic = -1;
static String     cur_uuid;

static void slot_exit (const HelperAgent *, int ic, const String &uuid)
{
	std::cout << "slot_exit (" << ic << ", " << uuid << ")\n";
	gtk_main_quit ();
}

static void slot_attach_input_context (const HelperAgent *agent, int ic, const String &uuid)
{
	std::cout << "slot_attach_input_context (" << ic << ", " << uuid << ")\n";

    // Inform the focused IMEngineInstance (which starts this helper) that this helper
    // is started successfully.
    Transaction trans;
    trans.put_command (SCIM_TRANS_CMD_REQUEST);
    trans.put_command (SCIM_TRANS_CMD_START_HELPER);
    agent->send_imengine_event (ic, uuid, trans);
}

static void slot_detach_input_context (const HelperAgent *agent, int ic, const String &uuid)
{
	std::cout << "slot_detach_input_context (" << ic << ", " << uuid << ")\n";
}

static void slot_update_screen (const HelperAgent *, int ic, const String &uuid, int screen)
{
	std::cout << "slot_update_screen (" << ic << ", " << uuid << ", " << screen << ")\n";

    if (gdk_display_get_n_screens (gdk_display_get_default ()) > screen) {
        GdkScreen *scr = gdk_display_get_screen (gdk_display_get_default (), screen);
        if (scr)
            gtk_window_set_screen (GTK_WINDOW (main_window), scr);
    }
}

static void slot_update_spot_location (const HelperAgent *, int ic, const String &uuid, int x, int y)
{
	std::cout << "slot_update_spot_location (" << ic << ", " << uuid << ", " << x << ", " << y << ")\n";

    gtk_window_move (GTK_WINDOW (main_window), x + 16, y + 16);
}

static void slot_process_imengine_event (const HelperAgent *, int ic, const String &uuid, const Transaction &trans)
{
	std::cout << "slot_process_imengine_event (" << ic << ", " << uuid << ")\n";

    if (uuid == "29904635-8afa-4e21-9138-0edb556150e7") {
        TransactionReader reader (trans);
        int cmd;
        KeyEvent key;
        WideString wstr;

        if (reader.get_command (cmd) && cmd == SCIM_TRANS_CMD_REQUEST) {
            while (reader.get_command (cmd)) {
                switch (cmd) {
                    case SCIM_TRANS_CMD_PROCESS_KEY_EVENT:
                    {
                        if (reader.get_data (key))
                            gtk_label_set_text (GTK_LABEL (keystring), key.get_key_string ().c_str ());
                        break;
                    }
                    case SCIM_TRANS_CMD_FOCUS_IN:
                    {
                        cur_ic = ic;
                        cur_uuid = uuid;
                        gtk_widget_show (main_window);
                        break;
                    }
                    case SCIM_TRANS_CMD_FOCUS_OUT:
                    {
                        cur_ic = -1;
                        cur_uuid = String ("");
                        gtk_widget_hide (main_window);
                        break;
                    }
                    case SCIM_TRANS_CMD_GET_SURROUNDING_TEXT:
                    {
                        if (reader.get_data (wstr))
                            gtk_label_set_text (GTK_LABEL(surrounding), utf8_wcstombs (wstr).c_str ());
                        break;
                    }
                }
            }
        }
    }
}

static void
delete_button_clicked_callback (GtkButton *button, gpointer user_data)
{
    HelperAgent *agent = static_cast<HelperAgent *> (user_data);

    if (agent && cur_ic >= 0) {
        Transaction trans;
        trans.put_command (SCIM_TRANS_CMD_REQUEST);
        trans.put_command (SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT);
        agent->send_imengine_event (cur_ic, cur_uuid, trans);
    }
}

static gboolean
agent_input_handler (GIOChannel *source, GIOCondition condition, gpointer user_data)
{
    if (condition == G_IO_IN) {
        std::cerr << "agent_input_handler G_IO_IN\n";
        HelperAgent *agent = static_cast<HelperAgent *> (user_data);
        if (agent && agent->has_pending_event ())
            agent->filter_event ();
    } else if (condition == G_IO_ERR || condition == G_IO_HUP) {
        std::cerr << "agent_input_handler G_IO_ERR|G_IO_HUP\n";
        gtk_main_quit ();
    }
    return TRUE;
}

static void run (const String &display)
{
    char **argv = new char * [4];
    int    argc = 1;
    argv [0] = const_cast<char*>("test-helper");
    argv [1] = 0;

	HelperAgent  agent;
    GtkWidget *vbox;
    GtkWidget *button;

    setenv ("DISPLAY", display.c_str (), 1);

    gtk_init (&argc, &argv);

    main_window = gtk_window_new (GTK_WINDOW_POPUP);
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_window_set_resizable (GTK_WINDOW (main_window), FALSE);
#else
    gtk_window_set_policy (GTK_WINDOW (main_window), FALSE, FALSE, TRUE);
#endif
    gtk_widget_hide (main_window);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);
    gtk_widget_show (vbox);

    keystring = gtk_label_new (0);
    gtk_box_pack_start (GTK_BOX (vbox), keystring, FALSE, FALSE, 0);
    gtk_widget_show (keystring);

    surrounding = gtk_label_new (0);
    gtk_box_pack_start (GTK_BOX (vbox), surrounding, FALSE, FALSE, 0);
    gtk_widget_show (surrounding);

    button = gtk_button_new_with_label ("Delete Surrounding");
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_widget_show (button);

    g_signal_connect (G_OBJECT(button), "clicked",
                      G_CALLBACK (delete_button_clicked_callback),
                      (gpointer) &agent);

	agent.signal_connect_exit (slot (slot_exit));
	agent.signal_connect_attach_input_context (slot (slot_attach_input_context));
	agent.signal_connect_detach_input_context (slot (slot_detach_input_context));
	agent.signal_connect_update_screen (slot (slot_update_screen));
	agent.signal_connect_update_spot_location (slot (slot_update_spot_location));
	agent.signal_connect_process_imengine_event (slot (slot_process_imengine_event));

    int fd = agent.open_connection (__helper_info, display);
    GIOChannel *ch = g_io_channel_unix_new (fd);

    if (fd < 0 || !ch) {
		std::cerr << "Unable to open the connection to Panel.\n";
		exit (-1);
    }

    g_io_add_watch (ch, G_IO_IN, agent_input_handler, (gpointer) &agent);
    g_io_add_watch (ch, G_IO_ERR, agent_input_handler, (gpointer) &agent);
    g_io_add_watch (ch, G_IO_HUP, agent_input_handler, (gpointer) &agent);

    gtk_main ();
}

/*
vi:ts=4:nowrap:ai:expandtab
*/

