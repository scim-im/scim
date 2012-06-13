/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <cstdio>

#include "scim-bridge-agent.h"

#include "scim-bridge-debug.h"
#include "scim-bridge-output.h"
#include "scim-bridge-string.h"

using namespace std;

/* Static variables */
static scim_bridge_debug_level_t debug_level;

/* Implementations */
scim_bridge_debug_level_t scim_bridge_debug_get_level ()
{
    return debug_level;
}


int main (int argc, char *argv[])
{
    bool standalone_enabled = false;
    bool noexit_enabled = false;

    debug_level = 0;

    const struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"verbose", 0, NULL, 'v'},
        {"quiet", 0, NULL, 'q'},
        {"debuglevel", 1, NULL, 'l'},
        {"noexit", 0, NULL, 'n'},
        {"standalone", 0, NULL, 's'},
        {0, 0, NULL, 0}
    };

    char short_options[] = "vhqdls:b:";

    unsigned int tmp_uint;

    int option = 0;
    while (option != EOF) {
        option = getopt_long (argc, argv, short_options, long_options, NULL);
        switch (option) {
            case 'v':
                debug_level = 6;
                break;
            case 'q':
                debug_level = 0;
                break;
            case 'l':
                if (scim_bridge_string_to_uint (&tmp_uint, optarg)) {
                    cerr << "Invalid debug level: " << optarg << endl;
                    exit (-1);
                } else {
                    debug_level = tmp_uint;
                }
                break;
            case 'n':
                noexit_enabled = true;
                break;
            case 's':
                standalone_enabled = true;
                break;
            case 'h':
                cout << "Usage: scim-bridge-agent [options]" << endl;
                cout << " Options" << endl << endl;
                cout << " -h, --help\tGive this help list" << endl;
                cout << " -v, --verbose\tVerbosely print out the debug message into standard output.This option equals to '--debuglevel=6'" << endl;
                cout << " -q, --quiet\tMake it print no debug message at all.This option equals to '--debuglevel=0" << endl;
                cout << " -l, --debuglevel\tSet how verbosely should it print debug output.'--debuglevel=0' equals to '--queit', and '--debuglevel=9' equals to '--verbose'" << endl;
                cout << " -s, --standalone\tGiven this, scim-brige-agent won't daemonize itself." << endl;
                cout << " -n, --noexit\tGiven this, scim-brige-agent won't exit when there is no client." << endl;
                exit (0);
                break;
            case ':':
            case '?':
                exit (-1);
            default:
                break;
        }
    }

    ScimBridgeAgent *agent = ScimBridgeAgent::alloc ();
    if (agent == NULL) {
        scim_bridge_perrorln ("Failed to allocate the agent. Exitting...");
        exit (-1);
    }

    agent->set_noexit_enabled (noexit_enabled);
    agent->set_standalone_enabled (standalone_enabled);

    const retval_t retval = agent->launch ();
    delete agent;

    if (retval == RETVAL_SUCCEEDED) {
        scim_bridge_println ("Cleanup, done. Exitting...");
        exit (0);
    } else {
        scim_bridge_println ("An error occurred. Exitting...");
        exit (-1);
    }
}
