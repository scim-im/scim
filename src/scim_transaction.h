/**
 * @file scim_transaction.h
 * @brief Transaction class.
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
 * $Id: scim_transaction.h,v 1.10 2005/05/24 12:22:51 suzhe Exp $
 */

#ifndef __SCIM_TRANSACTION_H
#define __SCIM_TRANSACTION_H

namespace scim {

/**
 * @addtogroup SocketCommunication
 * @{
 */

/**
 * @brief Signature of all valid data types which can be store into transaction.
 */
enum TransactionDataType
{
    SCIM_TRANS_DATA_UNKNOWN,        //!< Unknown transaction data type.
    SCIM_TRANS_DATA_COMMAND,        //!< Send/Receive command.
    SCIM_TRANS_DATA_RAW,            //!< Send/Receive raw buffer.
    SCIM_TRANS_DATA_UINT32,         //!< Send/Receive uint32.
    SCIM_TRANS_DATA_STRING,         //!< Send/Receive String.
    SCIM_TRANS_DATA_WSTRING,        //!< Send/Receive WideString.
    SCIM_TRANS_DATA_KEYEVENT,       //!< Send/Receive KeyEvent.
    SCIM_TRANS_DATA_ATTRIBUTE_LIST, //!< Send/Receive AttributeList.
    SCIM_TRANS_DATA_LOOKUP_TABLE,   //!< Send/Receive LookupTable.
    SCIM_TRANS_DATA_PROPERTY,       //!< Send/Receive Property.
    SCIM_TRANS_DATA_PROPERTY_LIST,  //!< Send/Receive PropertyList.
    SCIM_TRANS_DATA_VECTOR_UINT32,  //!< Send/Receive vector<uint32>.
    SCIM_TRANS_DATA_VECTOR_STRING,  //!< Send/Receive vector<String>.
    SCIM_TRANS_DATA_VECTOR_WSTRING, //!< Send/Receive vector<WideString>.
    SCIM_TRANS_DATA_TRANSACTION     //!< Send/Receive another Transaction.
};

/**
 * @brief An exception class to hold Transaction related errors.
 *
 * scim::Transaction and its related classes must throw
 * scim::TransactionError object when error.
 */
class TransactionError: public Exception
{
public:
    TransactionError (const String& what_arg)
        : Exception (String("scim::Transaction: ") + what_arg) { }
};

class TransactionHolder;
class TransactionReader;

/**
 * @brief This class is used to pack up many data and commands into one package
 *        and send them via socket.
 */
class Transaction
{
    friend class TransactionReader;

    TransactionHolder * m_holder;
    TransactionReader * m_reader;

    Transaction (const Transaction &);
    const Transaction & operator = (const Transaction &);
public:
    /**
     * @brief Constructor.
     *
     * @param bufsize the initial buffer size, maybe grow afterwards.
     */
    Transaction (size_t bufsize = 512);

    /**
     * @brief Destructor.
     */
    ~Transaction ();

    /**
     * @brief Check if the transaction is valid.
     *
     * @return true if this Transaction object is valid and ready to use.
     */
    bool valid () const;

    /**
     * @brief Get the size of this transaction.
     */
    size_t get_size () const;

    /**
     * @brief Write the transaction to a socket.
     *
     * @param socket the socket to be written to.
     * @param signature the leading signature to be written
     *        into the socket before the transaction itself,
     *        this signature maybe missing when receive the transaction.
     *        It's useful to check the connection before receiving
     *        a transaction by reading this signature.
     *
     * @return true if success.
     */
    bool write_to_socket (const Socket &socket, uint32 signature = 0) const;

    /**
     * @brief Read a transaction from a socket.
     *
     * @param socket the socket to be read from.
     * @param timeout time out in millisecond (1/1000 second), -1 means infinity.
     *
     * @return true if success.
     */
    bool read_from_socket (const Socket &socket, int timeout = -1);

    /**
     * @brief Write the transaction into a buffer.
     *
     * @param buf A buffer to store the transaction data.
     * @param bufsize The size of this buffer.
     * @return true if success (the buf is large enough).
     */
    bool write_to_buffer (void *buf, size_t bufsize) const;

    /**
     * @brief Read a transaction from a buffer.
     * 
     * @param buf A buffer contains the transaction.
     * @param bufsize The size of this buffer.
     * @return true if success.
     */
    bool read_from_buffer (const void *buf, size_t bufsize);

    /**
     * @brief Store a command into this transaction.
     *
     * @param cmd the command code, like SCIM_TRANS_CMD_FOCUS_IN etc.
     */
    void put_command (int cmd);

    /**
     * @brief Store a uint32 value into this transaction.
     */
    void put_data (uint32 val);

    /**
     * @brief Store a String object into this transaction.
     */
    void put_data (const String &str);

    /**
     * @brief Store a WideString object into this transaction.
     */
    void put_data (const WideString &str);

    /**
     * @brief Store a KeyEvent object into this transaction.
     */
    void put_data (const KeyEvent &key);

    /**
     * @brief Store an AttributeList object into this transaction.
     */
    void put_data (const AttributeList &attrs);

    /**
     * @brief Store a Property object into this transaction.
     */
    void put_data (const Property &property);

    /**
     * @brief Store a PropertyList object into this transaction.
     */
    void put_data (const PropertyList &properties);

    /**
     * @brief Store a LookupTable object into this transaction.
     */
    void put_data (const LookupTable &table);

    /**
     * @brief Store a std::vector<uint32> object into this transaction.
     */
    void put_data (const std::vector<uint32> &vec);

    /**
     * @brief Store a std::vector<String> object into this transaction.
     */
    void put_data (const std::vector<String> &vec);

    /**
     * @brief Store a std::vector<WideString> object into this transaction.
     */
    void put_data (const std::vector<WideString> &vec);

    /**
     * @brief Store a raw buffer into this transaction.
     */
    void put_data (const char *raw, size_t bufsize);

    /**
     * @brief Store another Transaction object into this transaction.
     */
    void put_data (const Transaction &trans);

    /**
     * @brief Get the type of the data at current read position.
     *
     * @return The type of the data can be read currently.
     */
    TransactionDataType get_data_type () const;

    /**
     * @brief Get a command from current read position.
     */
    bool get_command (int &cmd);

    /**
     * @brief Get an uint32 value from current read position.
     */
    bool get_data (uint32 &val);

    /**
     * @brief Get a String from current read position.
     */
    bool get_data (String &str);

    /**
     * @brief Get a WideString from current read position.
     */
    bool get_data (WideString &str);

    /**
     * @brief Get a KeyEvent from current read position.
     */
    bool get_data (KeyEvent &key);

    /**
     * @brief Get an AttributeList from current read position.
     */
    bool get_data (AttributeList &attrs);

    /**
     * @brief Get a Property from current read position.
     */
    bool get_data (Property &property);

    /**
     * @brief Get a PropertyList from current read position.
     */
    bool get_data (PropertyList &properties);

    /**
     * @brief Get a CommonLookupTable from current read position.
     */
    bool get_data (CommonLookupTable &table);

    /**
     * @brief Get a std::vector<uint32> from current read position.
     */
    bool get_data (std::vector<uint32> &vec);

    /**
     * @brief Get a std::vector<String> from current read position.
     */
    bool get_data (std::vector<String> &vec);

    /**
     * @brief Get a std::vector<WideString> from current read position.
     */
    bool get_data (std::vector<WideString> &vec);

    /**
     * @brief Get a raw buffer from current read position.
     * 
     * if raw == NULL then return the bufsize and skip this data.
     * *raw should be deleted afterwards (do not use free!).
     */
    bool get_data (char **raw, size_t &bufsize);

    /**
     * @brief Get a Transaction object from current read position.
     */
    bool get_data (Transaction &trans);

    /**
     * @brief Skip one data from current read position.
     */
    bool skip_data ();

    /**
     * @brief Rewind the current read position, then the data can be read again.
     */
    void rewind ();

    /**
     * @brief Clear the transaction, all data in this transaction will be freed.
     */
    void clear ();
};

/**
 * @brief This class is used to read data from a transaction without changing it.
 */
class TransactionReader
{
    class TransactionReaderImpl;

    TransactionReaderImpl *m_impl;

public:
    /**
     * @brief Default constructor.
     *
     * Construct an empty TransactionReader object.
     */
    TransactionReader ();

    /**
     * @brief Constructor.
     *
     * Construct a TransactionReader object and attach to a Transaction object.
     *
     * @param trans The Transaction to be read.
     */
    TransactionReader (const Transaction &trans);

    /**
     * @brief Destructor.
     */
    ~TransactionReader ();

    /**
     * @brief Copy constructor.
     */
    TransactionReader (const TransactionReader &);

    /**
     * @brief Copy operator.
     */
    const TransactionReader & operator = (const TransactionReader &);

    /**
     * @brief Attach this TransactionReader object to a Transaction.
     *
     * An empty TransactionReader object must be attached to a
     * Transaction object before reading.
     *
     * @param trans The Transaction object to be read.
     */
    void attach (const Transaction &trans);

    /**
     * @brief Detach this TransactionReader object from
     * currently attached Transaction object.
     */
    void detach ();

    /**
     * @brief Check if the transaction reader is valid.
     *
     * @return true if this TransactionReader object
     * is attached to a Transaction object and ready to be read.
     */
    bool valid () const;

    /**
     * @brief Get the type of the data at current read position.
     *
     * @return The type of the data can be read currently.
     */
    TransactionDataType get_data_type () const;

    /**
     * @brief Get a command from current read position.
     */
    bool get_command (int &cmd);

    /**
     * @brief Get an uint32 value from current read position.
     */
    bool get_data (uint32 &val);

    /**
     * @brief Get a String from current read position.
     */
    bool get_data (String &str);

    /**
     * @brief Get a WideString from current read position.
     */
    bool get_data (WideString &str);

    /**
     * @brief Get a KeyEvent from current read position.
     */
    bool get_data (KeyEvent &key);

    /**
     * @brief Get an AttributeList from current read position.
     */
    bool get_data (AttributeList &attrs);

    /**
     * @brief Get a Property from current read position.
     */
    bool get_data (Property &property);

    /**
     * @brief Get a PropertyList from current read position.
     */
    bool get_data (PropertyList &properties);

    /**
     * @brief Get a CommonLookupTable from current read position.
     */
    bool get_data (CommonLookupTable &table);

    /**
     * @brief Get a std::vector<uint32> from current read position.
     */
    bool get_data (std::vector<uint32> &vec);

    /**
     * @brief Get a std::vector<String> from current read position.
     */
    bool get_data (std::vector<String> &vec);

    /**
     * @brief Get a std::vector<WideString> from current read position.
     */
    bool get_data (std::vector<WideString> &vec);

    /**
     * @brief Get a raw buffer from current read position.
     * 
     * if raw == NULL then return the bufsize and skip this data.
     * *raw should be deleted afterwards (do not use free!).
     */
    bool get_data (char **raw, size_t &bufsize);

    /**
     * @brief Get a Transaction object from current read position.
     */
    bool get_data (Transaction &trans);

    /**
     * @brief Skip one data from current read position.
     */
    bool skip_data ();

    /**
     * @brief Rewind the current read position, then the data can be read again.
     */
    void rewind ();
};

/** @} */

} // namespace scim

#endif //__SCIM_TRANSACTION_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

