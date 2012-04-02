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
 * $Id: testsocketclient.cpp,v 1.3 2005/01/05 13:41:13 suzhe Exp $
 *
 */

#define Uses_SCIM_SOCKET
#define Uses_SCIM_SOCKET_TRANSACTION

#include <cstring>

#include "scim.h"
#include <ctype.h>
#include <unistd.h>

static const char * test_string [] = 
{
	"This is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very very long test string.",
	"This is a very very very very very very very very very long test string.",
	"This is a very very very very very very very very long test string.",
	"This is a very very very very very very very long test string.",
	"This is a very very very very very very long test string.",
	"This is a very very very very very long test string.",
	"This is a very very very very long test string.",
	"This is a very very very long test string.",
	"This is a very very long test string.",
	"This is a very long test string.",
	"This is a long test string.",
	"This is a test string.",
	"This is a string.",
	"Is a string.",
	"A string.",
	"String.",
	"A",
	"exit",
	NULL
};

int main (int argc, char *argv[])
{
	scim::SocketAddress address;
	scim::SocketClient client;
	scim::DebugOutput::set_verbose_level (4);
	int size;
	const char **ptr = test_string;

	if (argc > 1)
		address.set_address (argv [1]);
	else
		address.set_address ("inet:localhost:12345");

	if (client.connect (address)) {
		while (*ptr) {
			char tmp [4096];

			if ((size = client.write (*ptr, std::strlen (*ptr))) > 0) {
				std::cout << "Write " << size << " bytes to socket server ok!\n";
				if (client.read_with_timeout (tmp, size, 1000) == size &&
					std::strncmp (*ptr, tmp, size) == 0) {
					std::cout << "Read back ok!\n";
				} else {
					std::cerr << "Read back failed!\n";
				}
			} else {
				std::cerr << "Write failed!\n";
				break;
			}
			++ ptr;
			sleep (1);
		}
	}

	return 0;
}
/*
vi:ts=4:nowrap:ai
*/

