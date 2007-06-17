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
 * $Id: testsocketserver.cpp,v 1.2 2005/01/05 13:41:13 suzhe Exp $
 *
 */

#define Uses_SCIM_SOCKET
#define Uses_SCIM_SOCKET_TRANSACTION
#define Uses_C_STRING

#include "scim_private.h"
#include "scim.h"
#include <ctype.h>

void accept_callback (scim::SocketServer *server, const scim::Socket &client)
{
	std::cerr << "Accept client: " << client.get_id () << "\n";
}

void receive_callback (scim::SocketServer *server, const scim::Socket &client)
{
	char buffer [32768];
	int size;

	if (client.wait_for_data (100) <= 0) {
		std::cerr << "Error! timeout.\n";
		return;
	}

	if ((size = client.read_with_timeout (buffer, 32768, 100)) > 0) {
		buffer [size] = 0;
		std::cerr << "Read " << size << " chars from client: " << client.get_id () << "\n";
		std::cerr << buffer << "\n";
		if (client.write (buffer, size) == size) {
			std::cerr << "Write back ok!\n";
		} else {
			std::cerr << "Write back failed!\n";
		}
	} else {
		std::cerr << "Error! Close the client.\n";
		server->close_connection (client);
		return;
	}

	if (strcmp (buffer, "exit") == 0)
		server->shutdown ();

	std::cerr << "\n";
}

void exception_callback (scim::SocketServer *server, const scim::Socket &client)
{
	std::cerr << "Client: " << client.get_id () << " got an exception\n";
}

int main (int argc, char **argv)
{
	scim::SocketAddress address;
	scim::SocketAddress address2;
	scim::SocketServer server;

	scim::DebugOutput::set_verbose_level (4);

	if (argc > 1)
		address.set_address (argv[1]);
	else
		address.set_address ("inet:localhost:12345");

	address2 = address;

	server.signal_connect_accept (slot (accept_callback));
	server.signal_connect_receive (slot (receive_callback));
	server.signal_connect_exception (slot (exception_callback));

	if (server.create (address2)) {
		if (!server.run ()) {
			std::cerr << "Error occurred when server running: "
				  << server.get_error_message () << "\n";
		} else {
			std::cerr << "Server exit OK.\n";
		}
	} else {
		std::cerr << "Error creating server: "
			<< server.get_error_message () << "\n";
	}

	return 0;
}
/*
vi:ts=4:nowrap:ai
*/

