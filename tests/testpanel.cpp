/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: testpanel.cpp,v 1.15 2005/01/13 14:54:26 suzhe Exp $
 *
 */

#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HELPER
#define Uses_STL_IOSTREAM

#include "scim_private.h"
#include "scim.h"
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>

using namespace scim;

const char * str1 = "SCIM shurufamianbanceshichengxu \n\n";
const char * str2 = "It's a forwarded string.\n\n";
const char * str3 = "直接提交给客户程序的字符串.\n\n";

Property prop1 ("/TestPanel/Menu1", "Menu1", "", "This is menu1.");
Property prop2 ("/TestPanel/Menu1/Item1", "Item 1", "", "This is item 1 of menu1.");
Property prop3 ("/TestPanel/Menu1/Item2", "Item 2", "", "This is item 2 of menu1.");
Property prop4 ("/TestPanel/Menu1/Item3", "Item 3", "", "This is item 3 of menu1.");
Property prop5 ("/TestPanel/Menu1/Submenu1", "Submenu 1", "", "This is a submenu of menu1.");
Property prop6 ("/TestPanel/Menu1/Submenu1/Item4", "Item 4", "", "This is item 4 of menu1.");
Property prop7 ("/TestPanel/Menu1/Submenu1/Item5", "Item 5", "", "This is item 5 of menu1.");
Property prop8 ("/TestPanel/Menu1/Submenu1/TestInput", "TestInput", "", "Test input emulation.");
Property prop9 ("/TestPanel/Menu1/Item6", "Item 6", "", "This is item 6 of menu1.");
Property prop10 ("/TestPanel/Button1", "Button1", "", "This is button1.");

void slot_exit (const HelperAgent *agent, int ic, const String &uuid)
{
	std::cout << "slot_exit (" << ic << ", " << uuid << ")\n";
	exit (0);
}

void slot_update_screen (const HelperAgent *agent, int ic, const String &uuid, int screen)
{
	std::cout << "slot_update_screen (" << ic << ", " << uuid << ", " << screen << ")\n";
}

void slot_update_spot_location (const HelperAgent *agent, int ic, const String &uuid, int x, int y)
{
	std::cout << "slot_update_spot_location (" << ic << ", " << uuid << ", " << x << ", " << y << ")\n";
}

void slot_trigger_property (const HelperAgent *agent, int ic, const String &uuid, const String &property)
{
	std::cout << "slot_trigger_property (" << ic << ", " << uuid << ", " << property << ")\n";

 	if (property == prop10) {
		prop1.show (! prop1.visible ());
		agent->update_property (prop1);
	} else if (property == prop2) {
		prop8.set_active (!prop8.active ());
		agent->update_property (prop8);
	} else if (property == prop8) {
		sleep (1);

		for (const char *p = str1; *p; ++p) {
			KeyEvent key;
	
			key.code = *p;
	
			if (isupper (*p)) key.mask = SCIM_KEY_ShiftMask;
			if (*p == '\n') key.code = SCIM_KEY_Return;

			agent->send_key_event (ic, uuid, key);

			usleep (100000);
		}
	
		sleep (3);
	
		for (const char *p = str2; *p; ++p) {
			KeyEvent key;

			key.code = *p;

			if (isupper (*p)) key.mask = SCIM_KEY_ShiftMask;
			if (*p == '\n') key.code = SCIM_KEY_Return;

			agent->forward_key_event (ic, uuid, key);

			usleep (100000);
		}
	
		sleep (3);

		agent->commit_string (ic, uuid, utf8_mbstowcs (str3));
	}
}

int main (int argc, char *argv[])
{
	HelperInfo   info ("e135e0ee-5588-423e-a027-f07d769c12a3", "Test", "", "It's a test helper.", SCIM_HELPER_STAND_ALONE | SCIM_HELPER_NEED_SCREEN_INFO | SCIM_HELPER_NEED_SPOT_LOCATION_INFO);
	HelperAgent  agent;

	String       display (getenv ("DISPLAY"));
	int          id;

	fd_set       fds;

	id = agent.open_connection (info, display);

	if (id < 0) {
		std::cerr << "Unable to open the connection to Panel.\n";
		return -1;
	}

	agent.signal_connect_exit (slot (slot_exit));
	agent.signal_connect_update_screen (slot (slot_update_screen));
	agent.signal_connect_update_spot_location (slot (slot_update_spot_location));
	agent.signal_connect_trigger_property (slot (slot_trigger_property));

	PropertyList properties;

	properties.push_back (prop1);
	properties.push_back (prop2);
	properties.push_back (prop3);
	properties.push_back (prop4);
	properties.push_back (prop5);
	properties.push_back (prop6);
	properties.push_back (prop7);
	properties.push_back (prop8);
	properties.push_back (prop9);
	properties.push_back (prop10);

	agent.register_properties (properties);

	while (1) {
		FD_ZERO (&fds);
        FD_SET (id, &fds);
		if (select (id + 1, &fds, 0, 0, 0) < 0) break;

		std::cout << "Got event.\n";

		agent.filter_event ();
	}

	return 0;
}
/*
vi:ts=4:nowrap:ai
*/

