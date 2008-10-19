/** @file scim_helper_manager.cpp
 *  @brief Implementation of class HelperManager.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004-2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_helper_manager.cpp,v 1.2 2005/01/05 13:41:10 suzhe Exp $
 *
 */

#define Uses_SCIM_HELPER_MANAGER
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMAND
#define Uses_C_STDLIB

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h> // for strerror
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "scim_private.h"
#include "scim.h"

namespace scim {

#ifndef SCIM_HELPER_MANAGER_PROGRAM
  #define SCIM_HELPER_MANAGER_PROGRAM  (SCIM_LIBEXECDIR "/scim-helper-manager")
#endif

class HelperManager::HelperManagerImpl
{
    std::vector <HelperInfo> m_helpers;

    uint32                   m_socket_key;
    SocketClient             m_socket_client;
    int                      m_socket_timeout;

public:
    HelperManagerImpl ()
        : m_socket_key (0),
          m_socket_timeout (scim_get_default_socket_timeout ())
    {
        if (open_connection ()) {
            get_helper_list ();
        }
    }

    ~HelperManagerImpl ()
    {
        m_socket_client.close ();
    }

    unsigned int number_of_helpers () const
    {
        return m_helpers.size ();
    }

    bool get_helper_info (unsigned int idx, HelperInfo &info) const
    {
        if (idx < m_helpers.size ()) {
            info = m_helpers [idx];
            return true;
        }
        return false;
    }

    void run_helper (const String &uuid, const String &config_name, const String &display)
    {
        if (!m_socket_client.is_connected () || !uuid.length () || !m_helpers.size ()) return;

        Transaction trans;

        for (int i = 0; i < 3; ++i) {
            trans.clear ();
            trans.put_command (SCIM_TRANS_CMD_REQUEST);
            trans.put_data (m_socket_key);
            trans.put_command (SCIM_TRANS_CMD_HELPER_MANAGER_RUN_HELPER);
            trans.put_data (uuid);
            trans.put_data (config_name);
            trans.put_data (display);

            if (trans.write_to_socket (m_socket_client))
                return;

            m_socket_client.close ();

            if (!open_connection ())
                return;

            get_helper_list ();
        }
    }

private:
    bool open_connection ()
    {
        if (m_socket_client.is_connected ()) return true;

        SocketAddress address (scim_get_default_helper_manager_socket_address ());

        if (address.valid ()) {
            if (!m_socket_client.connect (address)) {
                if (launch_helper_manager () == 0) {
                    for (int i = 0; i < 200; ++i) {
                        if (m_socket_client.connect (address))
                            break;
                        scim_usleep (100000);
                    }
                } else {
                    // bail out: can't continue without the helper
                    std::cerr << _("Failed to launch HelperManager: exiting...") << std::endl;
                    exit(-1);
                }
            }
        }

        if (m_socket_client.is_connected () &&
            scim_socket_open_connection (m_socket_key,
                                         "HelperManager",
                                         "HelperLauncher",
                                         m_socket_client,
                                         m_socket_timeout)) {
            return true;
        }

        m_socket_client.close ();
        return false;
    }

    int launch_helper_manager () const
    {
        char *argv [] = { (char*)SCIM_HELPER_MANAGER_PROGRAM, 0 };

        pid_t child_pid;
 
        child_pid = fork ();
        // Error fork.
        if (child_pid == -1) {
            std::cerr << _("Error launching HelperManager") << " (" << SCIM_HELPER_MANAGER_PROGRAM << "): fork " << _("failed") << ": " << strerror(errno) << std::endl;
            return -1;
        }
 
        // In child process, start scim-helper-manager.
        if (child_pid == 0) {
            return execv (SCIM_HELPER_MANAGER_PROGRAM, argv);
        }
 
        // In parent process, wait the child exit.
 
        int status;
        pid_t ret_pid;
 
        ret_pid = waitpid (child_pid, &status, 0);
        if (ret_pid == -1) {
            std::cerr << _("Error launching HelperManager") << " (" << SCIM_HELPER_MANAGER_PROGRAM << "): waitpid " << _("failed") << ": " << strerror(errno) << std::endl;
        }
        assert(ret_pid==child_pid);
 
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                std::cerr << _("Error launching HelperManager") << " (" << SCIM_HELPER_MANAGER_PROGRAM << "): " << _("abnormal process termination") << std::endl;
            }
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            std::cerr << _("Error launching HelperManager") << " (" << SCIM_HELPER_MANAGER_PROGRAM << "): "
                      << _("it exited with signal") << " " << WTERMSIG(status) << std::endl;
            return -1;
        }

        std::cerr << _("Error launching HelperManager") << " (" << SCIM_HELPER_MANAGER_PROGRAM << "): " << _("unknown process launch error") << std::endl;
 
        return -1;
    }

    void get_helper_list ()
    {
        Transaction trans;
        uint32 num;
        int cmd;
        HelperInfo info;

        m_helpers.clear ();

        trans.put_command (SCIM_TRANS_CMD_REQUEST);
        trans.put_data (m_socket_key);
        trans.put_command (SCIM_TRANS_CMD_HELPER_MANAGER_GET_HELPER_LIST);

        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {

            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (num) && num > 0) {

                for (uint32 j = 0; j < num; ++j) {
                    if (trans.get_data (info.uuid) &&
                        trans.get_data (info.name) &&
                        trans.get_data (info.icon) &&
                        trans.get_data (info.description) &&
                        trans.get_data (info.option)) {
                        m_helpers.push_back (info);
                    }
                }
            }
        }
    }
};

HelperManager::HelperManager ()
    : m_impl (new HelperManagerImpl ())
{
}

HelperManager::~HelperManager ()
{
    delete m_impl;
}

unsigned int
HelperManager::number_of_helpers () const
{
    return m_impl->number_of_helpers ();
}

bool
HelperManager::get_helper_info (unsigned int idx, HelperInfo &info) const
{
    return m_impl->get_helper_info (idx, info);
}

void
HelperManager::run_helper (const String &uuid, const String &config_name, const String &display) const
{
    return m_impl->run_helper (uuid, config_name, display);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/

