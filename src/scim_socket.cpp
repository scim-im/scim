/** @file scim_socket.cpp
 *  @brief Implementation of Socket related classes.
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
 * $Id: scim_socket.cpp,v 1.44 2005/12/01 08:27:36 suzhe Exp $
 *
 */

#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_CONFIG_PATH
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include "scim_private.h"
#include "scim.h"

#define SCIM_SOCKET_SERVER_MAX_CLIENTS  256

namespace scim {

static struct in_addr
__gethostname (const char *host)
{
    struct in_addr addr = { 0 };

#if HAVE_GETHOSTBYNAME_R
    struct hostent hostbuf, *hp;
    size_t hstbuflen;
    char *tmphstbuf;
    int res;
    int herr;

    hstbuflen = 1024;
    /* Allocate buffer, remember to free it to avoid memory leakage.  */
    tmphstbuf = (char*) malloc (hstbuflen);

    while ((res = gethostbyname_r (host, &hostbuf, tmphstbuf, hstbuflen, &hp, &herr)) == ERANGE) {
        /* Enlarge the buffer.  */
        hstbuflen *= 2;
        tmphstbuf = (char*) realloc (tmphstbuf, hstbuflen);
    }

    /* Found the host */
    if (!res && hp) {
        addr = * ((struct in_addr *)hp->h_addr_list [0]);
    }

    free (tmphstbuf);
#else
    struct hostent *hostinfo;

    hostinfo = gethostbyname (host);

    if (hostinfo) 
        addr = * ((struct in_addr *) hostinfo->h_addr_list [0]);
#endif
    return addr;
}

class SocketAddress::SocketAddressImpl
{
    struct sockaddr *m_data;
    SocketFamily     m_family;
    String           m_address;

public:
    SocketAddressImpl (const String &addr = String ())
        : m_data (0), m_family (SCIM_SOCKET_UNKNOWN) {
        if (addr.length ()) set_address (addr);
    }

    SocketAddressImpl (const SocketAddressImpl &other)
        : m_data (0), m_family (other.m_family), m_address (other.m_address) {
        if (other.m_data) {
            size_t len = 0;
            switch (m_family) {
                case SCIM_SOCKET_LOCAL:
                    m_data = (struct sockaddr*) new struct sockaddr_un;
                    len = sizeof (sockaddr_un);
                    break;
                case SCIM_SOCKET_INET:
                    m_data = (struct sockaddr*) new struct sockaddr_in;
                    len = sizeof (sockaddr_in);
                    break;
            }

            if (len && m_data) memcpy (m_data, other.m_data, len);
        }
    }

    ~SocketAddressImpl () {
        if (m_data) delete m_data;
    }

    void swap (SocketAddressImpl &other) {
        std::swap (m_data, other.m_data);
        std::swap (m_family, other.m_family);
        std::swap (m_address, other.m_address);
    }

    bool valid () const {
        if (m_address.length () && m_data &&
            (m_family == SCIM_SOCKET_LOCAL || m_family == SCIM_SOCKET_INET))
            return true;
        return false;
    }

    SocketFamily get_family () const {
        return m_family;
    }

    bool set_address (const String &addr);

    const String & get_address () const {
        return m_address;
    }

    const void * get_data () const {
        return (void *)m_data;
    }

    int get_data_length () const {
        if (m_data) {
            if (m_family == SCIM_SOCKET_LOCAL)
                return SUN_LEN ((struct sockaddr_un*)(m_data));
            else if (m_family == SCIM_SOCKET_INET)
                return sizeof (struct sockaddr_in);
        }
        return 0;
    }
};

bool
SocketAddress::SocketAddressImpl::set_address (const String &addr)
{
    std::vector <String> varlist;

    struct sockaddr *new_data   = 0;
    SocketFamily     new_family = SCIM_SOCKET_UNKNOWN;

    scim_split_string_list (varlist, addr, ':');

    if (varlist.size () < 2)
        return false;

    if (varlist [0] == "local" || varlist [0] == "unix" || varlist [0] == "file") {
        String real_addr = addr.substr (varlist [0].length ()+1) +
                           String ("-") +
                           scim_get_user_name ();

        struct sockaddr_un *un = new struct sockaddr_un;

        un->sun_family = AF_UNIX;

        memset (un->sun_path, 0, sizeof (un->sun_path));

        strncpy (un->sun_path, real_addr.c_str (), sizeof (un->sun_path));

        un->sun_path[sizeof (un->sun_path) - 1] = '\0';

        SCIM_DEBUG_SOCKET(3) << "  local:" << un->sun_path << "\n";

        new_family = SCIM_SOCKET_LOCAL;
        new_data = (struct sockaddr *) un;

    } else if ((varlist [0] == "tcp" || varlist [0] == "inet") &&
                varlist.size () == 3) {

        struct sockaddr_in *in = new struct sockaddr_in;

        in->sin_addr = __gethostname (varlist [1].c_str ());

        if (in->sin_addr.s_addr) {
            in->sin_family = AF_INET;
            in->sin_port = htons (atoi (varlist [2].c_str ()));

            SCIM_DEBUG_SOCKET(3) << "  inet:"
                << inet_ntoa (in->sin_addr) << ":"
                << ntohs (in->sin_port) << "\n";

            new_family = SCIM_SOCKET_INET;
            new_data = (struct sockaddr *) in;
        } else {
            delete in;
        }
    }

    if (new_data) {
        if (m_data) delete m_data;

        m_data = new_data;
        m_family = new_family;
        m_address = addr;
        return valid ();
    }

    return false;
}

// Implementation of SocketAddress
SocketAddress::SocketAddress (const String &addr)
    : m_impl (new SocketAddressImpl (addr))
{
}

SocketAddress::SocketAddress (const SocketAddress &addr)
    : m_impl (new SocketAddressImpl (*addr.m_impl))
{
}

SocketAddress::~SocketAddress ()
{
    delete m_impl;
}

const SocketAddress&
SocketAddress::operator = (const SocketAddress &addr)
{
    if (this != &addr) {
        SocketAddressImpl new_impl (*addr.m_impl);
        m_impl->swap (new_impl);
    }
    return *this;
}

bool
SocketAddress::valid () const
{
    return m_impl->valid ();
}

SocketFamily
SocketAddress::get_family () const
{
    return m_impl->get_family ();
}

bool
SocketAddress::set_address (const String &addr)
{
    SCIM_DEBUG_SOCKET(2) << " SocketAddress::set_address (" << addr << ")\n";
    return m_impl->set_address (addr);
}

String
SocketAddress::get_address () const
{
    return m_impl->get_address ();
}

const void * 
SocketAddress::get_data () const
{
    return m_impl->get_data ();
}

int
SocketAddress::get_data_length () const
{
    return m_impl->get_data_length ();
}

// Implementation of Socket
class Socket::SocketImpl
{
    int           m_id;
    int           m_err;
    bool          m_binded;
    bool          m_no_close;
    SocketFamily  m_family;
    SocketAddress m_address;

public:

    SocketImpl (int id = -1)
        : m_id (id), m_err (0), m_binded (false), m_no_close (true),
          m_family (SCIM_SOCKET_UNKNOWN) {
    }

    ~SocketImpl () {
        close ();
    }

    bool valid () const {
        return m_id >= 0;
    }

    int read (void *buf, size_t size) {
        if (!buf || !size) { m_err = EINVAL; return -1; }
        if (m_id < 0) { m_err = EBADF; return -1; }

        m_err = 0;
        int ret;
        while (1) {
            ret = ::read (m_id, buf, size);
            if (ret >= 0)
                break;
            if (errno == EINTR)
                continue;
            m_err = errno;
        }
        return ret;
    }

    int read_with_timeout (void *buf, size_t size, int timeout) {
        if (!buf || !size) { m_err = EINVAL; return -1; }
        if (m_id < 0) { m_err = EBADF; return -1; }
 
        if (timeout < 0)
            return read (buf, size);
 
        int   ret;
        int   nbytes = 0;
        char *cbuf = static_cast<char *> (buf);
 
        while (size > 0) {
            ret = wait_for_data_internal (&timeout);
 
            if (ret < 0) return ret;
            if (ret == 0) return nbytes;
 
            ret = read (cbuf, size);
 
            if (ret < 0) return ret;
            if (ret == 0) return nbytes;
 
            cbuf += ret;
            nbytes += ret;
            size -= ret;
        }
        return nbytes;
    }

    int write (const void *buf, size_t size) {
        if (!buf || !size) { m_err = EINVAL; return -1; }
        if (m_id < 0) { m_err = EBADF; return -1; }
 
        int ret = -1;
 
        typedef void (*_scim_sighandler_t)(int);
        _scim_sighandler_t orig_handler = signal (SIGPIPE, SIG_IGN);
 
        m_err = 0;

        const char *cbuf = static_cast<const char*> (buf);

        while (size > 0) {
            ret = ::write (m_id, cbuf, size);
            if (ret > 0) {
                size -= (size_t) ret;
                cbuf += ret;
                continue;
            }
            if (errno == EINTR)
                continue;
            break;
        }
 
        m_err = errno;
 
        if (orig_handler != SIG_ERR)
            signal (SIGPIPE, orig_handler);
        else
            signal (SIGPIPE, SIG_DFL);

        return ret;
    }

    int wait_for_data (int timeout = -1) {
        if (m_id < 0) { m_err = EBADF; return -1; }
        return wait_for_data_internal (&timeout);
    }

    int get_error_number () const {
        return m_err;
    }

    String get_error_message () const {
        if (m_err > 0)
            return String (strerror (m_err));
        return String ();
    }

    int get_id () const {
        return m_id;
    }

    bool connect (const SocketAddress &addr) {
        SCIM_DEBUG_SOCKET(1) << "Socket: Connect to server: "
                             << addr.get_address () << " ...\n";

        m_err = EBADF;

        if (m_binded) return false;
 
        if (addr.valid () && m_id >= 0 && m_family == addr.get_family ()) {
            struct sockaddr * data = (sockaddr *) addr.get_data ();
            int len = addr.get_data_length ();
 
            if (::connect (m_id, data, len) == 0) {
                m_address = addr;
                m_err = 0;
                return true;
            }
            m_err = errno;
        }
        return false;
    }

    bool bind (const SocketAddress &addr) {
        SCIM_DEBUG_SOCKET(1) << "Socket: Bind to address: "
                             << addr.get_address () << " ...\n";
 
        m_err = EBADF;

        if (m_binded) return false;
 
        if (addr.valid () && m_id >= 0 && m_family == addr.get_family ()) {
            const struct sockaddr_un * data_un = 0;
            const struct sockaddr * data = static_cast <const struct sockaddr *>(addr.get_data ());
            int len = addr.get_data_length ();
 
            // Unlink the broken socket file.
            if (m_family == SCIM_SOCKET_LOCAL) {
                data_un = static_cast <const struct sockaddr_un *>(addr.get_data ());
                // The file is already exist, check if it's broken
                // by connecting to it.
                SCIM_DEBUG_SOCKET(2) << "Try to remove the broken socket file: " << data_un->sun_path << "\n";
 
                if (::access (data_un->sun_path, F_OK) == 0) {
                    SocketClient tmp_socket (addr);
 
                    if (!tmp_socket.is_connected ()) {
                        struct stat statbuf;
 
                        // If it's a socket file, then 
                        // delete it.
                        if (::stat (data_un->sun_path, &statbuf) == 0 && S_ISSOCK (statbuf.st_mode)) { // TODO need to separate stat and S_ISSOCK here
                            if (::unlink (data_un->sun_path) == -1) {
                                std::cerr << _("Creating socket") << " " << data_un->sun_path << ": "
                                          << _("file exists and we were unable to delete it") << ": " << _("syscall") << " unlink " << _("failed")
                                          << ": " << strerror(errno)
                                          << ": " << _("exiting") << ""
                                          << std::endl;
                                exit(-1);
                            }
                        } else {
                            std::cerr << _("Creating socket") << " " << data_un->sun_path << ": "
                                      << _("file exists and is not a socket") << ", " <<  _("exiting") << " ..." << std::endl;
                            exit(-1);
                        }
                    } else {
                        // NOTE another instance of the server is running: we can't just keep running like this
                        //      if we attempt to bind anyway there may be more than one servers listening on this socket.
                        //      In addition on FreeBSD bind will get EADDRINUSE, not clear why Linux doesn;t produce this same error.
                        std::cerr << _("Creating socket") << " " << data_un->sun_path << ": "
                                  << _("another instance of the server is already listening on this socket") << ", "  << _("exiting") << " ..."
                                  << std::endl;
                        exit(-1);
                    }
 
                    tmp_socket.close ();
                }
            }
 
            if (::bind (m_id, data, len) == 0) {
                m_address = addr;
                m_binded = true;
                m_err = 0;

                // Set correct permission for the socket file
                if (m_family == SCIM_SOCKET_LOCAL) {
                    if (::chmod (data_un->sun_path, S_IRUSR | S_IWUSR) == -1) {
                        std::cerr << _("Creating socket") << " " << data_un->sun_path << ": "
                                  << _("unable to change the mode of this file") << ": "  << _("syscall") << " chmod " << _("failed")
                                  << ", " << _("continuing") << " ..."
                                  << std::endl;
                    }
                }

                return true;
            }

            std::cerr << _("Error creating socket") << ": bind " << _("syscall failed") << ": " << strerror(errno) << std::endl;
            m_err = errno;
        }
        return false;
    }

    bool listen (int queue_length = 5) {
        if (m_id < 0) { m_err = EBADF; return -1; }

        SCIM_DEBUG_SOCKET(1) << "Socket: Listen: "
                             << queue_length << " ...\n";
 
        m_err = 0;

        int ret = ::listen (m_id, queue_length);
        if (ret == -1) {
            std::cerr << _("Error creating socket") << ": listen " << _("syscall failed") << ": " << strerror(errno) << std::endl;
            m_err = errno;
            return false;
        }
 
        return true;
    }

    int accept () {
        if (m_id < 0) { m_err = EBADF; return -1; }

        int ret = -1;
        int addrlen = 0;
 
        m_err = 0;
 
        if (m_family == SCIM_SOCKET_LOCAL) {
            struct sockaddr_un addr;
            addrlen = sizeof (addr);
            ret = ::accept (m_id, (struct sockaddr*) &addr, (socklen_t*) &addrlen);
        } else if (m_family == SCIM_SOCKET_INET) {
            struct sockaddr_in addr;
            addrlen = sizeof (addr);
            ret = ::accept (m_id, (struct sockaddr*) &addr, (socklen_t*) &addrlen);
        }
 
        if (ret < 0 && addrlen > 0)
            m_err = errno;
 
        SCIM_DEBUG_SOCKET(1) << "Socket: Accept connection, ret: " << ret << "\n";
 
        return ret;
    }

    bool create (SocketFamily family) {
        int ret = -1;
 
        if (family == SCIM_SOCKET_LOCAL) 
            ret = ::socket (PF_UNIX, SOCK_STREAM, 0);
        else if (family == SCIM_SOCKET_INET)
            ret = ::socket (PF_INET, SOCK_STREAM, 0);
        else {
            m_err = EINVAL;
            return false;
        }
 
        if (ret > 0) {
            if (m_id >= 0) close ();
            m_no_close = false;
            m_binded = false;
            m_err = 0;
            m_family = family;
            m_id = ret;
        } else {
            std::cerr << _("Error creating socket") << ": socket " << _("syscall failed") << ": " << strerror(errno) << std::endl;
            m_err = errno;
        }
 
        SCIM_DEBUG_SOCKET(1) << "Socket: Socket created, family: "
                             << family << " ret: " << ret << "\n";
 
        return ret >= 0;
    }

    void close () {
        if (m_id < 0) return;
 
        if (!m_no_close) {
            SCIM_DEBUG_SOCKET(2) << "  Closing the socket: " << m_id << " ...\n";
            ::close (m_id);
 
            // Unlink the socket file.
            if (m_binded && m_family == SCIM_SOCKET_LOCAL) {
                const struct sockaddr_un * data = static_cast <const struct sockaddr_un *>(m_address.get_data ());
                // FIXME: Don't unlink the socket file, because if the process is forked and child process exits
                // the socket file will be unlinked by child process.
                //SCIM_DEBUG_SOCKET(3) << "  Unlinking socket file " << data->sun_path << "...\n";
                ::unlink (data->sun_path);
            }
        }
 
        m_id = -1;
        m_err = 0;
        m_binded = false;
        m_no_close = false;
        m_family = SCIM_SOCKET_UNKNOWN;
        m_address = SocketAddress ();
    }

private:
    int wait_for_data_internal (int *timeout) {
        fd_set fds;
        struct timeval tv;
        struct timeval begin_tv;
        int ret;

        if (*timeout >= 0) {
            gettimeofday(&begin_tv, 0);
            tv.tv_sec = *timeout / 1000;
            tv.tv_usec = (*timeout % 1000) * 1000;
        }

        m_err = 0;

        while (1) {
            FD_ZERO(&fds);
            FD_SET(m_id, &fds);

            ret = select(m_id + 1, &fds, NULL, NULL, (*timeout >= 0) ? &tv : NULL);
            if (*timeout > 0) {
                int elapsed;
                struct timeval cur_tv;
                gettimeofday (&cur_tv, 0);
                elapsed = (cur_tv.tv_sec - begin_tv.tv_sec) * 1000 +
                          (cur_tv.tv_usec - begin_tv.tv_usec) / 1000;
                *timeout = *timeout - elapsed;
                if (*timeout > 0) {
                    tv.tv_sec = *timeout / 1000;
                    tv.tv_usec = (*timeout % 1000) * 1000;
                } else {
                    tv.tv_sec = 0;
                    tv.tv_usec = 0;
                    *timeout = 0;
                }
            }
            if (ret > 0) {
                return ret;
            } else if (ret == 0) {
                if (*timeout == 0)
                    return ret;
                else
                    continue;
            }

            if (errno == EINTR)
                continue;

            m_err = errno;
            return ret;
        }
    }
};

Socket::Socket (int id)
    : m_impl (new SocketImpl (id))
{
}

Socket::~Socket ()
{
    m_impl->close ();
    delete m_impl;
}

bool
Socket::valid () const
{
    return m_impl->valid ();
}

int
Socket::read (void *buf, size_t size) const
{
    return m_impl->read (buf, size);
}

int
Socket::read_with_timeout (void *buf, size_t size, int timeout) const
{
    return m_impl->read_with_timeout (buf, size, timeout);
}

int
Socket::write (const void *buf, size_t size) const
{
    return m_impl->write (buf, size);
}

int
Socket::wait_for_data (int timeout) const
{
    return m_impl->wait_for_data (timeout);
}

int
Socket::get_error_number () const
{
    return m_impl->get_error_number ();
}

String
Socket::get_error_message () const
{
    return m_impl->get_error_message ();
}

bool
Socket::connect (const SocketAddress &addr) const
{
    return m_impl->connect (addr);
}

bool
Socket::bind (const SocketAddress &addr) const
{
    return m_impl->bind (addr);
}

bool
Socket::listen (int queue_length) const
{
    return m_impl->listen (queue_length);
}

int
Socket::accept () const
{
    return m_impl->accept ();
}

int
Socket::get_id () const
{
    return m_impl->get_id ();
}

bool
Socket::create (SocketFamily family)
{
    return m_impl->create (family);
}

void
Socket::close ()
{
    return m_impl->close ();
}

// Implementation of SocketServer
struct SocketServer::SocketServerImpl
{
    fd_set   active_fds;
    int      max_fd;
    int      err;
    bool     running;
    bool     created;
    int      num_clients;
    int      max_clients;

    std::vector <int> ext_fds;

    SocketServerSignalSocket accept_signal;
    SocketServerSignalSocket receive_signal;
    SocketServerSignalSocket exception_signal;

    SocketServerImpl (int mc)
        : max_fd (0), err (0), running (false), created (false),
          num_clients (0), max_clients (std::min (mc, SCIM_SOCKET_SERVER_MAX_CLIENTS)) {
        FD_ZERO (&active_fds);
    }
};

SocketServer::SocketServer (int max_clients)
    : Socket (-1), m_impl (new SocketServerImpl (max_clients))
{
}

SocketServer::SocketServer (const SocketAddress &address, int max_clients)
    : Socket (-1), m_impl (new SocketServerImpl (max_clients))
{
    create (address);
}

SocketServer::~SocketServer ()
{
    delete m_impl;
}

bool
SocketServer::valid () const
{
    return m_impl->created;
}

bool
SocketServer::create (const SocketAddress &address)
{
    m_impl->err = EBUSY;
    if (!m_impl->created) {
        SocketFamily family = address.get_family ();

        SCIM_DEBUG_SOCKET (1) << "Creating Socket Server, family: " << family << "\n";

        if (family != SCIM_SOCKET_UNKNOWN) {
            if (Socket::create (family) &&
                Socket::bind (address) &&
                Socket::listen ()) {
                m_impl->created = true;
                m_impl->max_fd = Socket::get_id ();
                FD_ZERO (&(m_impl->active_fds));
                FD_SET (m_impl->max_fd, &(m_impl->active_fds));
                m_impl->err = 0;
                return true;
            }
            m_impl->err = Socket::get_error_number ();
            Socket::close ();
        } else {
            m_impl->err = EBADF;
        }
    }
    return false;
}

bool
SocketServer::run ()
{
    if (m_impl->created && !m_impl->running) {
        fd_set read_fds, exception_fds;
        int client;
        int i;

        m_impl->running = true;
        m_impl->err = 0;
        while (1) {
            read_fds = m_impl->active_fds;
            exception_fds = m_impl->active_fds;

            SCIM_DEBUG_SOCKET (2) << " SocketServer: Watching socket...\n";

            if (select (m_impl->max_fd + 1, &read_fds, NULL, &exception_fds, NULL) < 0) {
                m_impl->err = errno;
                m_impl->running = false;
                SCIM_DEBUG_SOCKET (3) << "  SocketServer: Error: "
                                      << get_error_message () << "\n";
                return false;
            }

            //The server has been shut down.
            if (!m_impl->running)
                return true;

            for (i = 0; i<m_impl->max_fd + 1; i++) {
                if (FD_ISSET (i, &read_fds)) {

                    //New connection
                    if (i == Socket::get_id ()) {
                        client = Socket::accept ();

                        SCIM_DEBUG_SOCKET (3) << "  SocketServer: Accept new connection:"
                                              << client << "\n";

                        if (client < 0) {
                            m_impl->err = Socket::get_error_number ();
                            m_impl->running = false;

                            SCIM_DEBUG_SOCKET (4) << "   SocketServer: Error occurred: "
                                << Socket::get_error_message () << "\n";

                            return false;
                        }

                        if (m_impl->max_clients > 0 &&
                            m_impl->num_clients >= m_impl->max_clients) {
                            SCIM_DEBUG_SOCKET (4) << "   SocketServer: Too many clients.\n";
                            ::close (client);
                        } else {
                            m_impl->num_clients ++;

                            //Store the new client
                            FD_SET (client, &(m_impl->active_fds));
                            if (m_impl->max_fd < client)
                                m_impl->max_fd = client;

                            Socket client_socket (client);
                            //emit the signal.
                            m_impl->accept_signal.emit (this, client_socket);
                        }

                    //Client reading
                    } else {
                        SCIM_DEBUG_SOCKET (3) << "  SocketServer: Accept client reading...\n";

                        Socket client_socket (i);
                        //emit the signal.
                        m_impl->receive_signal.emit (this, client_socket);
                    }
                }

                if (FD_ISSET (i, &exception_fds)) {

                    //The server got an exception, return.
                    if (i == Socket::get_id ()) {

                        SCIM_DEBUG_SOCKET (3) << "  SocketServer: Server got an exception, exiting...\n";

                        shutdown ();
                        return true;

                    } else {
                        SCIM_DEBUG_SOCKET (3) << "  SocketServer: Client "
                                              << i
                                              << "got an exception, callbacking...\n";

                        Socket client_socket (i);
                        m_impl->exception_signal.emit (this, client_socket);
                    }
                }

                if (!m_impl->running)
                    break;
            }

            //The server has been shut down.
            if (!m_impl->running)
                return true;
        }
    }

    m_impl->err = EBADF;
    return false;
}

bool
SocketServer::is_running () const
{
    return m_impl->running;
}

void
SocketServer::shutdown ()
{
    if (m_impl->created) {
        SCIM_DEBUG_SOCKET (2) << " SocketServer: Shutting down the server...\n";

        m_impl->running = false;

        for (int i = 0; i<m_impl->ext_fds.size (); i++)
            FD_CLR (m_impl->ext_fds [i], &m_impl->active_fds);

        for (int i = 0; i<m_impl->max_fd + 1; i++) {
            //Close all client
            if (FD_ISSET (i, &(m_impl->active_fds)) && i != Socket::get_id ()) {
                SCIM_DEBUG_SOCKET (3) << "  SocketServer: Closing client: "
                                      << i << "\n";
                ::close (i);
            }
        }
        m_impl->max_fd = 0;
        m_impl->created = false;
        m_impl->err = 0;
        m_impl->num_clients = 0;
        m_impl->ext_fds.clear ();
        FD_ZERO (&(m_impl->active_fds));

        Socket::close ();
    }
}

bool
SocketServer::close_connection (const Socket &socket)
{
    int id = socket.get_id ();
    if (m_impl->created && m_impl->running && id > 0 && FD_ISSET (id, &(m_impl->active_fds))) {

        SCIM_DEBUG_SOCKET (2) << " SocketServer: Closing the connection: " << id << "\n";

        m_impl->num_clients --;

        FD_CLR (id, &(m_impl->active_fds));

        std::vector <int>::iterator it = std::find (m_impl->ext_fds.begin (), m_impl->ext_fds.end (), id);
        if (it != m_impl->ext_fds.end ()) m_impl->ext_fds.erase (it);

        ::close (id);
        return true;
    }
    return false;
}

int
SocketServer::get_error_number () const
{
    if (m_impl->err)
        return m_impl->err;

    return Socket::get_error_number ();
}

String
SocketServer::get_error_message () const
{
    if (m_impl->err)
        return String (strerror (m_impl->err));

    return Socket::get_error_message ();
}

int
SocketServer::get_max_clients () const
{
    return m_impl->max_clients;
}

void
SocketServer::set_max_clients (int max_clients)
{
    if (max_clients < SCIM_SOCKET_SERVER_MAX_CLIENTS)
        m_impl->max_clients = max_clients;
}

bool
SocketServer::insert_external_socket (const Socket &sock)
{
    int fd = sock.get_id ();

    if (valid () && sock.valid () && sock.wait_for_data (0) >= 0 &&
        m_impl->num_clients < m_impl->max_clients &&
        !FD_ISSET (fd, &m_impl->active_fds)) {
        m_impl->ext_fds.push_back (fd);
        FD_SET (fd, &m_impl->active_fds);
        if (m_impl->max_fd < fd) m_impl->max_fd = fd;
        m_impl->num_clients ++;
        return true;
    }
    return false;
}

bool
SocketServer::remove_external_socket (const Socket &sock)
{
    int fd = sock.get_id ();

    if (valid () && FD_ISSET (fd, &m_impl->active_fds)) {
        FD_CLR (fd, &m_impl->active_fds);
        std::vector <int>::iterator it = std::find (m_impl->ext_fds.begin (), m_impl->ext_fds.end (), fd);
        if (it != m_impl->ext_fds.end ()) m_impl->ext_fds.erase (it);
        m_impl->num_clients --;
        return true;
    }
    return false;
}

Connection
SocketServer::signal_connect_accept (SocketServerSlotSocket *slot)
{
    return m_impl->accept_signal.connect (slot);
}

Connection
SocketServer::signal_connect_receive (SocketServerSlotSocket *slot)
{
    return m_impl->receive_signal.connect (slot);
}

Connection
SocketServer::signal_connect_exception (SocketServerSlotSocket *slot)
{
    return m_impl->exception_signal.connect (slot);
}

//Implementation of SocketClient
SocketClient::SocketClient ()
    : Socket (-1), m_connected (false)
{
}

SocketClient::SocketClient (const SocketAddress &address)
    : Socket (-1), m_connected (false)
{
    connect (address);
}

SocketClient::~SocketClient ()
{
}

bool
SocketClient::is_connected () const
{
    return Socket::valid () && m_connected;
}

bool
SocketClient::connect (const SocketAddress &address)
{
    if (m_connected) close ();

    if (Socket::create (address.get_family ()) && Socket::connect (address)) {
        m_connected = true;
        return true;
    } else {
        close ();
    }

    return false;
}

void
SocketClient::close ()
{
    Socket::close ();
    m_connected = false;
}

#define SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS        "local:/tmp/scim-socket-frontend"
#define SCIM_DEFAULT_PANEL_SOCKET_ADDRESS           "local:/tmp/scim-panel-socket"
#define SCIM_DEFAULT_HELPER_MANAGER_SOCKET_ADDRESS  "local:/tmp/scim-helper-manager-socket"

String scim_get_default_socket_frontend_address ()
{
    String address (SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS);

    address = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_FRONTEND_ADDRESS, address);

    const char *env = getenv ("SCIM_SOCKET_ADDRESS");
    if (env && strlen (env) > 0) {
        address = String (env);
    } else {
        env = getenv ("SCIM_FRONTEND_SOCKET_ADDRESS");
        if (env && strlen (env))
            address = String (env);
    }

    if (address == "default")
        address = SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS;

    return address;
}

String scim_get_default_socket_imengine_address ()
{
    String address (SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS);

    address = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_IMENGINE_ADDRESS, address);

    const char *env = getenv ("SCIM_SOCKET_ADDRESS");
    if (env && strlen (env) > 0) {
        address = String (env);
    } else {
        env = getenv ("SCIM_IMENGINE_SOCKET_ADDRESS");
        if (env && strlen (env))
            address = String (env);
    }

    if (address == "default")
        address = SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS;

    return address;
}

String scim_get_default_socket_config_address ()
{
    String address (SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS);

    address = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_CONFIG_ADDRESS, address);

    const char *env = getenv ("SCIM_SOCKET_ADDRESS");
    if (env && strlen (env) > 0) {
        address = String (env);
    } else {
        env = getenv ("SCIM_CONFIG_SOCKET_ADDRESS");
        if (env && strlen (env))
            address = String (env);
    }

    if (address == "default")
        address = SCIM_DEFAULT_SOCKET_FRONTEND_ADDRESS;

    return address;
}

String scim_get_default_panel_socket_address (const String &display)
{
    String address (SCIM_DEFAULT_PANEL_SOCKET_ADDRESS);

    address = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_SOCKET_ADDRESS, address);

    const char *env = getenv ("SCIM_PANEL_SOCKET_ADDRESS");

    if (env && strlen (env) > 0) {
        address = String (env);
    }

    if (address == "default")
        address = SCIM_DEFAULT_PANEL_SOCKET_ADDRESS;

    SocketAddress sockaddr (address);

    if (!sockaddr.valid ())
        return String ();

    String::size_type colon_pos = display.rfind (':');
    String disp_name = display;
    int    disp_num  = 0;

    // Maybe It's a X11 DISPLAY name
    if (colon_pos != String::npos) {
        String::size_type dot_pos = display.find ('.', colon_pos+1);
        // It has screen number
        if (dot_pos != String::npos) {
            disp_name = display.substr (0, dot_pos);
        }
        // FIXME: ignore remote X Server name.
        disp_num = atoi (display.substr (colon_pos + 1, String::npos).c_str());
    }

    if (sockaddr.get_family () == SCIM_SOCKET_LOCAL) {
        for (size_t i = 0; i < disp_name.length(); ++i) {
            if (disp_name[i] == '/') disp_name[i] = '_';
        }
        address = address + disp_name;
    } else if (sockaddr.get_family () == SCIM_SOCKET_INET) {
        std::vector <String> varlist;
        scim_split_string_list (varlist, address, ':');
        if (varlist.size () == 3) {
            int port = atoi (varlist [2].c_str ()) + disp_num;
            char buf [10];
            snprintf (buf, 10, "%d", port);
            varlist [2] = String (buf);
            address = scim_combine_string_list (varlist, ':');
        }
    }

    sockaddr.set_address (address);

    if (sockaddr.valid ())
        return address;

    return String ();
}

String scim_get_default_helper_manager_socket_address ()
{
    String address (SCIM_DEFAULT_HELPER_MANAGER_SOCKET_ADDRESS);

    address = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_HELPER_MANAGER_SOCKET_ADDRESS, address);

    const char *env = getenv ("SCIM_HELPER_MANAGER_SOCKET_ADDRESS");
    if (env && strlen (env) > 0) {
        address = String (env);
    }

    if (address == "default")
        address = SCIM_DEFAULT_HELPER_MANAGER_SOCKET_ADDRESS;

    return address;
}

int scim_get_default_socket_timeout ()
{
    int timeout = 5000;

    timeout = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_SOCKET_TIMEOUT, timeout);

    const char *env = getenv ("SCIM_SOCKET_TIMEOUT");

    if (env && strlen (env))
        timeout = atoi (env);

    if (timeout <= 0) timeout = -1;

    return timeout;
}

static bool
scim_socket_check_type (const String &types,
                        const String &atype)
{
    std::vector <String> type_list;
    scim_split_string_list (type_list, types, ',');

    return std::find (type_list.begin (), type_list.end (), atype) != type_list.end ();
}

bool
scim_socket_open_connection   (uint32       &key,
                               const String &client_type,
                               const String &server_type,
                               const Socket &socket,
                               int           timeout)
{
    if (!socket.valid () || !client_type.length () || !server_type.length ())
        return false;

    Transaction trans;

    trans.put_command (SCIM_TRANS_CMD_REQUEST);
    trans.put_command (SCIM_TRANS_CMD_OPEN_CONNECTION);
    trans.put_data (String (SCIM_BINARY_VERSION));
    trans.put_data (client_type);

    if (trans.write_to_socket (socket)) {
        int cmd;
        String server_types;
        if (trans.read_from_socket (socket, timeout) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (server_types) && scim_socket_check_type (server_types, server_type) &&
            trans.get_data (key)) {
            trans.clear ();
            trans.put_command (SCIM_TRANS_CMD_REPLY);
            trans.put_command (SCIM_TRANS_CMD_OK);
            if (trans.write_to_socket (socket))
                return true;
        } else {
            trans.clear ();
            trans.put_command (SCIM_TRANS_CMD_REPLY);
            trans.put_command (SCIM_TRANS_CMD_FAIL);
            trans.write_to_socket (socket);
        }
    }

    return false;
}

String
scim_socket_accept_connection (uint32       &key,
                               const String &server_types,
                               const String &client_types,
                               const Socket &socket,
                               int           timeout)
{
    if (!socket.valid () || !client_types.length () || !server_types.length ())
        return String ("");

    Transaction trans;

    if (trans.read_from_socket (socket, timeout)) {
        int cmd;
        String version;
        String client_type;
        if (trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_REQUEST &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_OPEN_CONNECTION &&
            trans.get_data (version) && version == String (SCIM_BINARY_VERSION) &&
            trans.get_data (client_type) && 
            (scim_socket_check_type (client_types, client_type) || client_type == "ConnectionTester")) {
            key = (uint32) rand ();
            trans.clear ();
            trans.put_command (SCIM_TRANS_CMD_REPLY);
            trans.put_data (server_types);
            trans.put_data (key);

            if (trans.write_to_socket (socket) &&
                trans.read_from_socket (socket, timeout) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

                // Client is ok, return the client type.
                return (client_type == "ConnectionTester") ? String ("") : client_type;
            }
        }
    }
    return String ("");
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
