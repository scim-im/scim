/**
 * @file scim_socket.h
 * @brief Socket interfaces.
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
 * $Id: scim_socket.h,v 1.25 2005/01/25 15:13:15 suzhe Exp $
 */

#ifndef __SCIM_SOCKET_H
#define __SCIM_SOCKET_H

namespace scim {

/**
 * @addtogroup SocketCommunication
 * @{
 */

class Socket;
class SocketAddress;
class SocketServer;
class SocketClient;

typedef Slot2<void, SocketServer *, const Socket &>
        SocketServerSlotSocket;

typedef Signal2<void, SocketServer *, const Socket &>
        SocketServerSignalSocket;

/**
 * @brief An exception class to hold Socket related errors.
 *
 * scim::Socket and its derived classes must throw
 * scim::SocketError object when error.
 */
class SocketError: public Exception
{
public:
    SocketError (const String& what_arg)
        : Exception (String("scim::Socket: ") + what_arg) { }
};

/**
 * @brief The vaild socket address/protocol family,
 *
 * Corresponding to libc PF_LOCAL/AF_LOCAL and PF_INET/AF_INET
 */
enum SocketFamily
{
    SCIM_SOCKET_UNKNOWN, /**< Unknown or invalid socket address/protocol */
    SCIM_SOCKET_LOCAL,   /**< Unix local socket address/protocol */
    SCIM_SOCKET_INET     /**< Internet (ipv4) socket address/protocol */
};

/**
 * @brief The class to hold a socket address.
 * 
 * Class SocketAddress encapsulates the details of
 * socket address, like socketaddr_un and socketaddr_in.
 * 
 * A SocketAddress object can be constructed from an
 * address string, which must start with one of the
 * following prefixes:
 *  - inet: or tcp:
 *    A internet address (ipv4). This kind of address must
 *    include two parts, separated by a colon. The first part
 *    is the ip address, the second part is the port. For example:
 *    inet:127.0.0.1:12345
 *  - local: or unix: or file:
 *    A unix or local socket address. It's a full path of a socket file.
 *    For example: local:/tmp/scim-socket-frontend
 */
class SocketAddress
{
    class SocketAddressImpl;
    SocketAddressImpl *m_impl;

public:
    /**
     * @brief Constructor.
     *
     * @param addr the address string.
     */
    SocketAddress (const String &addr = String ());

    /**
     * @brief Copy constructor.
     */
    SocketAddress (const SocketAddress &addr);

    /**
     * @brief Destructor.
     */
    ~SocketAddress ();

    /**
     * @brief Copy operator.
     */
    const SocketAddress& operator = (const SocketAddress &addr);

    /**
     * @brief Check if this address is valid.
     *
     * @return true if this address is valid.
     */
    bool valid () const;

    /**
     * @brief Get the family of this socket address.
     *
     * @return the family enum value of this address.
     *
     * @sa SocketFamily
     */
    SocketFamily get_family () const;

    /**
     * @brief Set a new address.
     *
     * @param addr the new address string.
     */
    bool set_address (const String &addr);

    /**
     * @brief Get the address string.
     *
     * @return the address string.
     */
    String get_address () const;

    /**
     * @brief Get the internal data of this socket address,
     * used by class Socket.
     *
     * @return the pointer to the data, usually a sockaddr struct.
     */
    const void * get_data () const;

    /**
     * @brief Get the size of the internall data.
     *
     * @return the size of the internall data returned by get_data ();
     */
    int get_data_length () const;
};

/**
 * @brief Socket communication class.
 *
 * Class Socket provides basic operation of socket,
 * such as bind connect, read, write etc.
 *
 * This class object cannot be created directly by user.
 * Only the object of its derived classes SocketServer and SocketClient
 * can be created directly.
 */
class Socket
{
    class SocketImpl;

    SocketImpl *m_impl;

    Socket (const Socket&);
    const Socket& operator = (const Socket&);

public:
    /**
     * @brief Create a Socket object from an already created socket_id.
     *
     * @param id an file id of an existing socket.
     */
    Socket (int id = -1);

    /**
     * @brief Destructor
     */
    ~Socket ();

    /**
     * @brief Check if the socket is valid.
     *
     * @return true if the socket is ready to read and write.
     */
    bool valid () const;

    /**
     * @brief Read data from socket.
     *
     * @param buf the buffer to store the data.
     * @param size size of the buffer.
     * 
     * @return the amount of data actually read, -1 means error occurred.
     */
    int read (void *buf, size_t size) const;

    /**
     * @brief read data from socket with a timeout.
     *
     * @param buf the buffer to store the data.
     * @param size size of the buffer, and the amount of data to be read.
     * @param timeout time out in millisecond (1/1000 second), -1 means infinity.
     * 
     * @return the amount of data actually read,
     *         0 means the connection is closed,
     *         -1 means error occurred.
     */
    int read_with_timeout (void *buf, size_t size, int timeout) const;

    /**
     * @brief Write data to socket.
     *
     * @param buf the buffer stores the data.
     * @param size size of the data to be sent.
     *
     * @return the amount of data acutally sent, or -1 if an error occurred.
     */
    int write (const void *buf, size_t size) const;

    /**
     * @brief Wait until there are some data ready to read.
     *
     * @param timeout time out in millisecond (1/1000 second), -1 means infinity.
     *
     * @return > 0 if data is OK, == 0 if time is out, < 0 if an error occurred.
     */
    int wait_for_data (int timeout = -1) const;

    /**
     * @brief Get the number of the last occurred error.
     *
     * @return the standard errno value.
     */
    int get_error_number () const;

    /**
     * @brief Get the message of the last occurred error.
     *
     * @return the error message of the last occurred error.
     */
    String get_error_message () const;

    /**
     * @brief Get the socket id.
     *
     * @return the file id of this socket object.
     */
    int get_id () const;

protected:

    /**
     * @brief Initiate a connection on a socket.
     *
     * @param addr the address to be connected to.
     *
     * @return true if success.
     */
    bool connect (const SocketAddress &addr) const;

    /**
     * @brief Bind a socket to an address, used by SocketServer.
     *
     * @param addr the address to be binded to.
     *
     * @return true if success.
     */
    bool bind (const SocketAddress &addr) const;

    /**
     * @brief Listen for connections on a socket.
     *
     * @param queue_length the length of the waiting queue.
     *
     * @return true if success.
     */
    bool listen (int queue_length = 5) const;

    /**
     * @brief Accept a connection on the socket, used by SocketServer.
     *
     * @return the id of the accepted client socket, or -1 if an error is occurred.
     */
    int accept () const;

    /**
     * @brief Create a socket for specific family.
     *
     * @param family the family type.
     *
     * @return true if success.
     */
    bool create (SocketFamily family);

    /**
     * @brief Close the socket.
     */
    void close ();
};

/**
 * @brief Socket Server class.
 *
 * Class SocketServer provides basic operations to create a Socket Server,
 * such as create, run etc.
 */
class SocketServer : private Socket
{
    class SocketServerImpl;

    SocketServerImpl *m_impl;

public:
    /**
     * @brief Default constructor, do nothing.
     */
    SocketServer (int max_clients = -1);

    /**
     * @brief Constructor.
     *
     * @param address create a server on this address.
     * @param max_clients the max number of socket clients, -1 means unlimited.
     */
    SocketServer (const SocketAddress &address, int max_clients = -1);

    /**
     * @brief Destructor.
     */
    ~SocketServer ();

    /**
     * @brief Test if the server is valid.
     *
     * @return true if the socket server is valid and ready to run.
     */
    bool valid () const;

    /**
     * @brief Create a socket on an address.
     *
     * @param address the address to be listen.
     *
     * @return true if OK.
     */
    bool create (const SocketAddress &address);

    /**
     * @brief Run the server.
     *
     * @return true if it ran successfully.
     */
    bool run ();

    /**
     * @brief Check if the server is running.
     *
     * @return true if it's running.
     */
    bool is_running () const;

    /**
     * @brief Shutdown the server.
     */
    void shutdown ();

    /**
     * @brief Close a client connection.
     *
     * @param socket the client socket object to be closed.
     * @return true if the socket was closed successfully.
     */
    bool close_connection (const Socket &socket);

    /**
     * @brief Get the number of the last occurred error.
     *
     * @return the standard errno value.
     */
    int get_error_number () const;

    /**
     * @brief Get the message of the last occurred error.
     *
     * @return the error message corresponding to the errno.
     */
    String get_error_message () const;

    /**
     * @brief Get the max number of socket clients.
     *
     * @return the max number of socket clients allowed to connect this server.
     */
    int get_max_clients () const;

    /**
     * @brief Set the max number of clients.
     *
     * @param max_clients the max number of socket clients allowed to connect this server.
     */
    void set_max_clients (int max_clients);

    /**
     * @brief Insert an external socket into the main loop.
     *
     * If data is available on this socket, then the receive signal will be emitted.
     *
     * @param sock The external socket to be inserted.
     * @return true if the socket is valid.
     */
    bool insert_external_socket (const Socket &sock);

    /**
     * @brief Remove an external socket which was inserted by insert_external_socket ().
     *
     * @param sock The external socket to be removed.
     * @return true if the socket is valid and has been removed successfully.
     */
    bool remove_external_socket (const Socket &sock);

public:
    /**
     * @brief Connect a slot to socket accept signal.
     *
     * Connect a slot to socket accept signal, if a client connection is accepted,
     * this signal will be emitted.
     *
     * @param slot the slot to be connected to this signal.
     *
     * @return the Connection object of this slot-signal connection, can be used
     *         to disconnect the slot later.
     */
    Connection signal_connect_accept (SocketServerSlotSocket *slot);

    /**
     * @brief Connect a slot to socket receive signal.
     * 
     * Connect a slot to socket receive signal, if a client send data to this server,
     * this signal will be emitted.
     *
     * @param slot the slot to be connected to this signal.
     *
     * @return the Connection object of this slot-signal connection, can be used
     *         to disconnect the slot later.
     */
    Connection signal_connect_receive (SocketServerSlotSocket *slot);

    /**
     * @brief Connect a slot to socket exception signal.
     *
     * Connect a slot to socket exception signal, if an exception was occurred
     * to a client connection, this signal will be emitted.
     *
     * @param slot the slot to be connected to this signal.
     *
     * @return the Connection object of this slot-signal connection, can be used
     *         to disconnect the slot later.
     */
    Connection signal_connect_exception (SocketServerSlotSocket *slot);
};

/**
 * @brief Socket client class.
 *
 * Class SocketClient provides basic operations to create a Socket Client,
 * such as connect, read, write, etc.
 */
class SocketClient : public Socket
{
    bool m_connected;

public:
    /**
     * @brief Constructor.
     */
    SocketClient ();

    /**
     * @brief Constructor.
     *
     * @param address the server address to be connected.
     */
    SocketClient (const SocketAddress &address);

    /**
     * @brief Destructor.
     */
    ~SocketClient ();

    /**
     * @brief Check if the socket is connected.
     *
     * @return true if the socket client is connected to a server.
     */
    bool is_connected () const;

    /**
     * @brief Connect to a server.
     *
     * @param address the server socket address to be connected to.
     *
     * @return true if connected successfully.
     */
    bool connect (const SocketAddress &address);

    /**
     * @brief Close the client.
     */
    void close ();
};

/**
 * @brief Get the default socket address of SocketFrontEnd 
 *
 * SocketFrontEnd should listen on this address by default.
 */
String scim_get_default_socket_frontend_address ();

/**
 * @brief Get the default socket address of SocketIMEngine
 *
 * SocketIMEngine should connect to this address by default.
 */
String scim_get_default_socket_imengine_address ();

/**
 * @brief Get the default socket address of SocketConfig
 *
 * SocketConfig should connect to this address by default.
 */
String scim_get_default_socket_config_address ();

/**
 * @brief Get the default socket address of the Panel running on localhost.
 *
 * The panel running on local host should listen on this address by default.
 * All FrontEnds which need panel should connect to this address by default.
 */
String scim_get_default_panel_socket_address (const String &display);

/**
 * @brief Get the default socket address of Helper Manager Server running on localhost.
 */
String scim_get_default_helper_manager_socket_address ();

/**
 * @brief Get the default socket timeout value.
 *
 * All socket connection should use this timeout value.
 */
int    scim_get_default_socket_timeout ();

/**
 * @brief Helper function to open a connection to a socket server
 * with a standard hand shake protocol.
 *
 * This function is used by a socket client to establish a connection
 * between a socket server with a standard hand shake protocol.
 *
 * The communication between Panel and FrontEnd, SocketFrontEnd and SocketIMEngine,
 * SocketFrontEnd and SocketConfig all uses this hand shake protocol.
 *
 * @param key         A random magic key sent from the socket server
 *                    to identify this client in later communications.
 * @param client_type The type of this socket client, for example:
 *                    "FrontEnd", "GUI", "SocketIMEngine", "SocketConfig" etc.
 *                    If the type is "ConnectionTester" then this call just
 *                    test if the connection can be established. The client
 *                    should close this socket just after the call.
 * @param server_type The request socket server type, for example:
 *                    "Panel", "SocketFrontEnd" etc.
 * @param socket      The reference to the client socket which has been
 *                    connected to the socket server.
 * @param timeout     The socket read timeout in millisecond, -1 means unlimited.
 *
 * @return true if the connection was established successfully, otherwise
 *         return false, and the client should close the socket.
 */
bool   scim_socket_open_connection   (uint32       &key,
                                      const String &client_type,
                                      const String &server_type,
                                      const Socket &socket,
                                      int           timeout = -1);

/**
 * @brief Helper function to accept a connection request from a socket client
 * with a standard hand shake protocol.
 *
 * This function is used by a socket server to accept a connection request
 * from a socket client which is calling scim_socket_open_connection ().
 *
 * If a client with type "ConnectionTester" connected to this socket server, then
 * this function will return an empty string, but tell the client the connection
 * was established successfully.
 *
 * @param key          A random magic key to identify the socket client in later
 *                     communications.
 * @param server_types The type of this server, for example:
 *                     "SocketFrontEnd", "Panel" etc.
 *                     One server can have multiple types, separated by comma.
 * @param client_types A list of acceptable client types, separated by comma.
 *                     The client type maybe: "FrontEnd", "GUI", "SocketIMEngine" etc.
 * @param socket       The socket connected to the client.
 * @param timeout      the socket read timeout in millisecond, -1 means unlimited.
 *
 * @return The type of the accepted socket client, or an empty string if the
 *         connection could not be established.
 */
String scim_socket_accept_connection (uint32       &key,
                                      const String &server_types,
                                      const String &client_types,
                                      const Socket &socket,
                                      int           timeout = -1);
/** @} */

} // namespace scim

#endif //__SCIM_SOCKET_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

