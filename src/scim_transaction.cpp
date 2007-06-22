/** @file scim_transaction.cpp
 *  @brief Implementation of Transaction related classes.
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
 * $Id: scim_transaction.cpp,v 1.13.2.1 2006/06/07 09:27:57 suzhe Exp $
 *
 */

#define Uses_SCIM_TRANSACTION
#define Uses_STL_ALGORITHM
#define Uses_C_STDLIB
#define Uses_C_STRING

#include "scim_private.h"
#include "scim.h"

namespace scim {

#define SCIM_TRANS_MIN_BUFSIZE 512
#define SCIM_TRANS_MAX_BUFSIZE (1048576*16)
#define SCIM_TRANS_MAGIC       0x4d494353
#define SCIM_TRANS_HEADER_SIZE (sizeof (uint32) * 4)

class TransactionHolder
{
    mutable int    m_ref;

public:
    size_t         m_buffer_size;
    size_t         m_write_pos;
    unsigned char *m_buffer;

public:
    TransactionHolder (size_t bufsize)
        : m_ref (0),
          m_buffer_size (std::max ((size_t)SCIM_TRANS_MIN_BUFSIZE, bufsize)),
          m_write_pos (SCIM_TRANS_HEADER_SIZE),
          m_buffer ((unsigned char*) malloc (std::max ((size_t)SCIM_TRANS_MIN_BUFSIZE, bufsize))) {
        if (!m_buffer)
            throw Exception ("TransactionHolder::TransactionHolder() Out of memory");
    }

    ~TransactionHolder () {
        free (m_buffer);
    }

    bool valid () const {
        return m_buffer && m_buffer_size;
    }

    void ref () const {
        ++m_ref;
    }

    void unref () const {
        if ((--m_ref) <= 0) delete this;
    }

    void request_buffer_size (size_t request) {
        if (m_write_pos + request >= m_buffer_size) {
            size_t bufsize = std::max ((size_t) SCIM_TRANS_MIN_BUFSIZE, request + 1) + m_buffer_size;
            unsigned char *tmp = (unsigned char*) realloc (m_buffer, bufsize);

            if (!tmp)
                throw Exception ("TransactionHolder::request_buffer_size() Out of memory");

            m_buffer = tmp;
            m_buffer_size = bufsize;
        }
    }

    uint32 calc_checksum () const {
        uint32 sum = 0;

        unsigned char *ptr = m_buffer + SCIM_TRANS_HEADER_SIZE;
        unsigned char *ptr_end = m_buffer + m_write_pos;

        while (ptr < ptr_end) {
            sum += (uint32) (*ptr);
            sum = (sum << 1) | (sum >> 31);
            ++ ptr;
        }

        return sum;
    }
};

Transaction::Transaction (size_t bufsize)
    : m_holder (new TransactionHolder (bufsize)),
      m_reader (new TransactionReader ())
{
    m_holder->ref ();
    m_reader->attach (*this);
}

Transaction::~Transaction ()
{
    delete m_reader;
    m_holder->unref ();
}

bool
Transaction::valid () const
{
    return m_holder->valid ();
}

bool
Transaction::write_to_socket (const Socket &socket, uint32 signature) const
{
    if (socket.valid () && valid ()) {
        scim_uint32tobytes (m_holder->m_buffer, signature);
        scim_uint32tobytes (m_holder->m_buffer + sizeof (uint32), SCIM_TRANS_MAGIC);
        scim_uint32tobytes (m_holder->m_buffer + sizeof (uint32) * 2, m_holder->m_write_pos - SCIM_TRANS_HEADER_SIZE);
        scim_uint32tobytes (m_holder->m_buffer + sizeof (uint32) * 3, m_holder->calc_checksum ());
        return socket.write (m_holder->m_buffer, m_holder->m_write_pos) == (int) m_holder->m_write_pos;
    }
    return false;
}

bool
Transaction::read_from_socket (const Socket &socket, int timeout)
{
    if (socket.valid () && valid ()) {
        unsigned char buf [sizeof (uint32) * 2];
        uint32 sign1, sign2;
        uint32 checksum;
        int size;
        int nbytes;

        nbytes = socket.read_with_timeout (buf, sizeof (uint32) * 2, timeout);
        if (nbytes < sizeof (uint32) * 2)
            return false;

        sign1 = scim_bytestouint32 (buf);
        sign2 = scim_bytestouint32 (buf + sizeof (uint32));

        if (sign1 != SCIM_TRANS_MAGIC && sign2 != SCIM_TRANS_MAGIC)
            return false;

        if (sign2 == SCIM_TRANS_MAGIC) {
            nbytes = socket.read_with_timeout (buf, sizeof (uint32), timeout);
            if (nbytes < sizeof (uint32))
                return false;
            size = scim_bytestouint32 (buf);
        } else {
            size = (int) sign2;
        }

        nbytes = socket.read_with_timeout (buf, sizeof (uint32), timeout);
        if (nbytes < sizeof (uint32))
            return false;

        checksum = scim_bytestouint32 (buf);

        if (size <= 0 || size > SCIM_TRANS_MAX_BUFSIZE)
            return false;

        clear ();

        m_holder->request_buffer_size (size);

        while (size != 0) {
            nbytes = socket.read_with_timeout (m_holder->m_buffer + m_holder->m_write_pos, size, timeout);
            if (nbytes <= 0) {
                m_holder->m_write_pos = SCIM_TRANS_HEADER_SIZE;
                return false;
            }

            size -= nbytes;
            m_holder->m_write_pos += nbytes;
        }

        if (checksum != m_holder->calc_checksum ()) {
            m_holder->m_write_pos = SCIM_TRANS_HEADER_SIZE;
            return false;
        }

        return true;
    }
    return false;
}

size_t
Transaction::get_size () const
{
    return m_holder->m_write_pos;
}

bool
Transaction::write_to_buffer (void *buf, size_t bufsize) const
{
    if (valid () && buf && bufsize >= m_holder->m_write_pos) {
        unsigned char *cbuf = static_cast <unsigned char *> (buf);

        memcpy (buf, m_holder->m_buffer, m_holder->m_write_pos); 

        scim_uint32tobytes (cbuf, 0);
        scim_uint32tobytes (cbuf + sizeof (uint32), SCIM_TRANS_MAGIC);
        scim_uint32tobytes (cbuf + sizeof (uint32) * 2, m_holder->m_write_pos - SCIM_TRANS_HEADER_SIZE);
        scim_uint32tobytes (cbuf + sizeof (uint32) * 3, m_holder->calc_checksum ());

        return true;
    }
    return false;
}

bool
Transaction::read_from_buffer (const void *buf, size_t bufsize)
{
    const unsigned char * cbuf = static_cast <const unsigned char *> (buf);

    if (valid () && buf &&
        scim_bytestouint32 (cbuf) == 0 &&
        scim_bytestouint32 (cbuf + sizeof (uint32)) == SCIM_TRANS_MAGIC &&
        scim_bytestouint32 (cbuf + sizeof (uint32) * 2) <= bufsize - SCIM_TRANS_HEADER_SIZE) {

        uint32 size = scim_bytestouint32 (cbuf + sizeof (uint32) * 2) + SCIM_TRANS_HEADER_SIZE;
        uint32 checksum = scim_bytestouint32 (cbuf + sizeof (uint32) * 3);

        if (m_holder->m_buffer_size < size)
            m_holder->request_buffer_size (size - m_holder->m_buffer_size);

        memcpy (m_holder->m_buffer, buf, size);

        m_holder->m_write_pos = SCIM_TRANS_HEADER_SIZE;

        if (checksum == m_holder->calc_checksum ())
            return true;
    }
    return false;
}

void
Transaction::put_command (int type)
{
    m_holder->request_buffer_size (1 + sizeof (uint32));

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_COMMAND;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, (uint32) type);
    m_holder->m_write_pos += sizeof (uint32);
}

void
Transaction::put_data (uint32 val)
{
    m_holder->request_buffer_size (1 + sizeof (val));

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_UINT32;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, val);

    m_holder->m_write_pos += sizeof (uint32);
}

void
Transaction::put_data (const String &str)
{
    m_holder->request_buffer_size (1 + str.length () + sizeof (uint32));

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_STRING;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, str.length ());

    m_holder->m_write_pos += sizeof (uint32);

    if (str.length ())
        memcpy (m_holder->m_buffer + m_holder->m_write_pos, str.c_str (), str.length ());

    m_holder->m_write_pos += str.length ();
}

void
Transaction::put_data (const WideString &str)
{
    String mbs = utf8_wcstombs (str);

    m_holder->request_buffer_size (1 + mbs.length () + sizeof (uint32));

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_WSTRING;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, mbs.length ());

    m_holder->m_write_pos += sizeof (uint32);

    if (mbs.length ())
        memcpy (m_holder->m_buffer + m_holder->m_write_pos, mbs.c_str (), mbs.length ());

    m_holder->m_write_pos += mbs.length ();
}

void
Transaction::put_data (const KeyEvent &key)
{
    m_holder->request_buffer_size (1 + sizeof (uint32) + sizeof (uint16) * 2);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_KEYEVENT;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, key.code);

    m_holder->m_write_pos += sizeof (uint32);

    scim_uint16tobytes (m_holder->m_buffer + m_holder->m_write_pos, key.mask);

    m_holder->m_write_pos += sizeof (uint16);

    scim_uint16tobytes (m_holder->m_buffer + m_holder->m_write_pos, key.layout);

    m_holder->m_write_pos += sizeof (uint16);
}

void
Transaction::put_data (const AttributeList &attrs)
{
    size_t size = attrs.size () * (sizeof (uint32) * 3 + 1) + sizeof (uint32) + 1;

    m_holder->request_buffer_size (size);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_ATTRIBUTE_LIST;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, attrs.size ());

    m_holder->m_write_pos += sizeof (uint32);

    for (size_t i=0; i<attrs.size (); ++i) {
        m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) attrs[i].get_type ();
        scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, attrs[i].get_value ());
        m_holder->m_write_pos += sizeof (uint32);
        scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, attrs[i].get_start ());
        m_holder->m_write_pos += sizeof (uint32);
        scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, attrs[i].get_length ());
        m_holder->m_write_pos += sizeof (uint32);
    }
}

void
Transaction::put_data (const Property &property)
{
    size_t request = property.get_key ().length () +
                     property.get_label ().length () +
                     property.get_icon ().length () +
                     property.get_tip ().length () +
                     (sizeof (uint32) + 1) * 4 + 3;

    m_holder->request_buffer_size (request);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_PROPERTY;

    put_data (property.get_key ());
    put_data (property.get_label ());
    put_data (property.get_icon ());
    put_data (property.get_tip ());

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) property.visible ();
    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) property.active();
}

void
Transaction::put_data (const PropertyList &properties)
{
    m_holder->request_buffer_size (1 + sizeof(uint32));

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_PROPERTY_LIST;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, properties.size ());

    m_holder->m_write_pos += sizeof (uint32);

    for (PropertyList::const_iterator it = properties.begin (); it != properties.end (); ++ it)
        put_data (*it);
}

void
Transaction::put_data (const LookupTable &table)
{
    unsigned char stat = 0;
    size_t i;

    m_holder->request_buffer_size (4);

    //Can be page up.
    if (table.get_current_page_start ())
        stat |= 1;

    //Can be page down.
    if (table.get_current_page_start () + table.get_current_page_size () <
        table.number_of_candidates ())
        stat |= 2;

    //Cursor is visible.
    if (table.is_cursor_visible ())
        stat |= 4;

    //Pagesize is fixed.
    if (table.is_page_size_fixed ())
        stat |= 8;

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_LOOKUP_TABLE;
    m_holder->m_buffer [m_holder->m_write_pos++] = stat;
    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) table.get_current_page_size ();
    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) table.get_cursor_pos_in_current_page ();

    // Store page labels.
    for (i = 0; i < table.get_current_page_size (); ++i)
        put_data (table.get_candidate_label (i));

    // Store page candidates, attributes.
    for (i = 0; i < table.get_current_page_size (); ++i) {
        put_data (table.get_candidate_in_current_page (i));
        put_data (table.get_attributes_in_current_page (i));
    }
}

void
Transaction::put_data (const std::vector<uint32> &vec)
{
    size_t size = vec.size () * sizeof (uint32) + sizeof (uint32) + 1;

    m_holder->request_buffer_size (size);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_VECTOR_UINT32;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, vec.size ());

    m_holder->m_write_pos += sizeof (uint32);

    for (size_t i=0; i<vec.size ();i++) {
        scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, vec[i]);
        m_holder->m_write_pos += sizeof (uint32);
    }
}

void
Transaction::put_data (const std::vector<String> &vec)
{
    m_holder->request_buffer_size (sizeof(uint32) + 1);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_VECTOR_STRING;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, vec.size ());

    m_holder->m_write_pos += sizeof (uint32);

    for (size_t i=0; i<vec.size ();i++) {
        put_data (vec[i]);
    }
}

void
Transaction::put_data (const std::vector<WideString> &vec)
{
    m_holder->request_buffer_size (sizeof(uint32) + 1);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_VECTOR_WSTRING;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, vec.size ());

    m_holder->m_write_pos += sizeof (uint32);

    for (size_t i=0; i<vec.size ();i++) {
        put_data (vec[i]);
    }
}

void
Transaction::put_data (const char *raw, size_t bufsize)
{
    if (!raw || !bufsize)
        return;

    m_holder->request_buffer_size (bufsize + sizeof (uint32) + 1);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_RAW;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, (uint32) bufsize);

    m_holder->m_write_pos += sizeof (uint32);

    memcpy (m_holder->m_buffer + m_holder->m_write_pos, raw, bufsize);

    m_holder->m_write_pos += bufsize;
}

void
Transaction::put_data (const Transaction &trans)
{
    if (!trans.valid ())
        return;

    m_holder->request_buffer_size (trans.m_holder->m_write_pos + sizeof (uint32) + 1);

    m_holder->m_buffer [m_holder->m_write_pos++] = (unsigned char) SCIM_TRANS_DATA_TRANSACTION;

    scim_uint32tobytes (m_holder->m_buffer + m_holder->m_write_pos, (uint32) trans.m_holder->m_write_pos);

    m_holder->m_write_pos += sizeof (uint32);

    memcpy (m_holder->m_buffer + m_holder->m_write_pos, trans.m_holder->m_buffer, trans.m_holder->m_write_pos);

    m_holder->m_write_pos += trans.m_holder->m_write_pos;
}

void
Transaction::clear ()
{
    m_holder->m_write_pos = SCIM_TRANS_HEADER_SIZE;
    m_reader->rewind ();
}

TransactionDataType
Transaction::get_data_type () const
{
    return m_reader->get_data_type ();
}

bool
Transaction::get_command (int &type)
{
    return m_reader->get_command (type);
}

bool
Transaction::get_data (uint32 &val)
{
    return m_reader->get_data (val);
}

bool
Transaction::get_data (String &str)
{
    return m_reader->get_data (str);
}

bool
Transaction::get_data (WideString &str)
{
    return m_reader->get_data (str);
}

bool
Transaction::get_data (KeyEvent &key)
{
    return m_reader->get_data (key);
}

bool
Transaction::get_data (AttributeList &attrs)
{
    return m_reader->get_data (attrs);
}

bool
Transaction::get_data (Property &property)
{
    return m_reader->get_data (property);
}

bool
Transaction::get_data (PropertyList &properties)
{
    return m_reader->get_data (properties);
}

bool
Transaction::get_data (CommonLookupTable &table)
{
    return m_reader->get_data (table);
}

bool
Transaction::get_data (std::vector<uint32> &vec)
{
    return m_reader->get_data (vec);
}

bool
Transaction::get_data (std::vector<String> &vec)
{
    return m_reader->get_data (vec);
}

bool
Transaction::get_data (std::vector<WideString> &vec)
{
    return m_reader->get_data (vec);
}

bool
Transaction::get_data (char **raw, size_t &bufsize)
{
    return m_reader->get_data (raw, bufsize);
}

bool
Transaction::get_data (Transaction &trans)
{
    return m_reader->get_data (trans);
}

bool
Transaction::skip_data ()
{
    return m_reader->skip_data ();
}

void
Transaction::rewind ()
{
    return m_reader->rewind ();
}

// TransactionReader implementation.
class TransactionReader::TransactionReaderImpl
{
public:
    const TransactionHolder *m_holder;
    size_t                   m_read_pos;

public:
    TransactionReaderImpl (const TransactionHolder *holder = 0)
        : m_holder (holder),
          m_read_pos (SCIM_TRANS_HEADER_SIZE) {
        if (m_holder) m_holder->ref ();
    }

    ~TransactionReaderImpl () {
        if (m_holder) m_holder->unref ();
    }

    bool valid () const {
        return m_holder && m_holder->valid ();
    }

    void attach (const TransactionHolder *holder) {
        if (m_holder) m_holder->unref ();
        m_holder = holder;
        if (m_holder) m_holder->ref ();
        m_read_pos = SCIM_TRANS_HEADER_SIZE;
    }

    void detach () {
        if (m_holder) m_holder->unref ();
        m_holder = 0;
        m_read_pos = SCIM_TRANS_HEADER_SIZE;
    }

    void rewind () {
        m_read_pos = SCIM_TRANS_HEADER_SIZE;
    }
};

TransactionReader::TransactionReader ()
    : m_impl (new TransactionReaderImpl ())
{
}

TransactionReader::TransactionReader (const Transaction &trans)
    : m_impl (new TransactionReaderImpl (trans.m_holder))
{
}

TransactionReader::TransactionReader (const TransactionReader &reader)
    : m_impl (new TransactionReaderImpl (reader.m_impl->m_holder))
{
}

const TransactionReader &
TransactionReader::operator = (const TransactionReader &reader)
{
    m_impl->attach (reader.m_impl->m_holder);
    m_impl->m_read_pos = reader.m_impl->m_read_pos;
    return *this;
}

TransactionReader::~TransactionReader ()
{
    delete m_impl;
}

void
TransactionReader::attach (const Transaction &trans)
{
    m_impl->attach (trans.m_holder);
}

void
TransactionReader::detach ()
{
    m_impl->detach ();
}

bool
TransactionReader::valid () const
{
    return m_impl->valid ();
}

TransactionDataType
TransactionReader::get_data_type () const
{
    if (!valid () || m_impl->m_holder->m_write_pos <= m_impl->m_read_pos)
        return SCIM_TRANS_DATA_UNKNOWN;

    return (TransactionDataType) m_impl->m_holder->m_buffer [m_impl->m_read_pos];
}

bool
TransactionReader::get_command (int &type)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_COMMAND) {

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        type = (int) scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);

        m_impl->m_read_pos += sizeof (uint32);

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (uint32 &val)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_UINT32) {

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        val = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);

        m_impl->m_read_pos += sizeof (uint32);

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (String &str)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_STRING) {

        size_t len;
        size_t old_read_pos = m_impl->m_read_pos;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        len = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + len)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        if (len)
            str = String (m_impl->m_holder->m_buffer + m_impl->m_read_pos, m_impl->m_holder->m_buffer + m_impl->m_read_pos + len);
        else
            str = String ("");

        m_impl->m_read_pos += len;

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (WideString &str)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_WSTRING) {

        String mbs;
        size_t len;
        size_t old_read_pos = m_impl->m_read_pos;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        len = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + len)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        if (len)
            mbs = String (m_impl->m_holder->m_buffer + m_impl->m_read_pos, m_impl->m_holder->m_buffer + m_impl->m_read_pos + len);
        else
            mbs = String ("");

        m_impl->m_read_pos += len;

        str = utf8_mbstowcs (mbs);
        return true;
    }
    return false;
}

bool
TransactionReader::get_data (KeyEvent &key)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_KEYEVENT) {

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) * 2 + 1))
            return false;

        m_impl->m_read_pos ++;

        key.code   = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        key.mask   = scim_bytestouint16 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint16);

        key.layout = scim_bytestouint16 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint16);

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (AttributeList &attrs)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_ATTRIBUTE_LIST) {

        AttributeType type;
        size_t num;
        uint32 value;
        uint32 start;
        uint32 length;

        attrs.clear ();

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        num = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + (sizeof (uint32) * 3 + 1) * num)) {
            m_impl->m_read_pos -= (sizeof (uint32) + 1);
            return false;
        }

        for (size_t i=0; i<num; i++) {
            type = (AttributeType) m_impl->m_holder->m_buffer [m_impl->m_read_pos];
            m_impl->m_read_pos ++;
            value = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
            m_impl->m_read_pos += sizeof (uint32);
            start = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
            m_impl->m_read_pos += sizeof (uint32);
            length = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
            m_impl->m_read_pos += sizeof (uint32);

            attrs.push_back (Attribute (start, length, type, value));
        }
        return true;
    }
    return false;
}

bool
TransactionReader::get_data (Property &property)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_PROPERTY) {

        size_t old_read_pos = m_impl->m_read_pos;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) * 4 + 3))
            return false;

        m_impl->m_read_pos ++;

        String str;

        if (! get_data (str)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }
        property.set_key (str);

        if (! get_data (str)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }
        property.set_label (str);

        if (! get_data (str)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }
        property.set_icon (str);

        if (! get_data (str)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }
        property.set_tip (str);

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + 2)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        property.show ((bool) m_impl->m_holder->m_buffer [m_impl->m_read_pos]);
        m_impl->m_read_pos ++;

        property.set_active ((bool) m_impl->m_holder->m_buffer [m_impl->m_read_pos]);
        m_impl->m_read_pos ++;

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (PropertyList &properties)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_PROPERTY_LIST) {

        size_t old_read_pos = m_impl->m_read_pos;
        size_t num;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        num = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        properties.clear ();
        Property prop;

        for (size_t i = 0; i < num; ++ i) {
            if (!get_data (prop)) {
                m_impl->m_read_pos = old_read_pos;
                return false;
            }
            properties.push_back (prop);
        }

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (CommonLookupTable &table)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_LOOKUP_TABLE) {

        size_t i;

        size_t old_read_pos = m_impl->m_read_pos;

        unsigned char stat;
        uint32 page_size;
        uint32 cursor_pos;

        WideString    wstr;
        AttributeList attrs;

        std::vector<WideString> labels;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + 4))
            return false;

        table.clear ();

        m_impl->m_read_pos ++;

        stat = m_impl->m_holder->m_buffer [m_impl->m_read_pos];
        m_impl->m_read_pos ++;

        page_size = (uint32) m_impl->m_holder->m_buffer [m_impl->m_read_pos];
        m_impl->m_read_pos ++;

        cursor_pos = (uint32) m_impl->m_holder->m_buffer [m_impl->m_read_pos];
        m_impl->m_read_pos ++;

        if (page_size > SCIM_LOOKUP_TABLE_MAX_PAGESIZE ||
            (cursor_pos >= page_size && page_size > 0)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        table.set_page_size (page_size);

        for (i = 0; i < page_size; ++i) {
            if (!get_data (wstr)) {
                m_impl->m_read_pos = old_read_pos;
                return false;
            }
            labels.push_back (wstr);
        }

        table.set_candidate_labels (labels);

        //Can be paged up.
        if (stat & 1)
            table.append_candidate (0x3400);

        for (i = 0; i < page_size; ++i) {
            if (get_data (wstr) && get_data (attrs)) {
                table.append_candidate (wstr, attrs);
            } else {
                m_impl->m_read_pos = old_read_pos;
                return false;
            }
        }

        // Can be paged down.
        if (stat & 2)
            table.append_candidate (0x3400);

        if (stat & 1) {
            table.set_page_size (1);
            table.page_down ();
            table.set_page_size (page_size);
        }

        table.set_cursor_pos_in_current_page (cursor_pos);

        if (stat & 4)
            table.show_cursor (true);
        else
            table.show_cursor (false);

        if (stat & 8)
            table.fix_page_size (true);
        else
            table.fix_page_size (false);

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (std::vector<uint32> &vec)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_VECTOR_UINT32) {

        size_t old_read_pos = m_impl->m_read_pos;
        size_t num;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        num = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) * num)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        vec.clear ();

        for (size_t i=0; i<num; i++) {
            vec.push_back (scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos));
            m_impl->m_read_pos += sizeof (uint32);
        }

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (std::vector<String> &vec)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_VECTOR_STRING) {

        size_t old_read_pos = m_impl->m_read_pos;
        size_t num;
        String str;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        num = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        vec.clear ();

        for (size_t i=0; i<num; i++) {
            if (!get_data (str)) {
                m_impl->m_read_pos = old_read_pos;
                return false;
            }
            vec.push_back (str);
        }

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (std::vector<WideString> &vec)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_VECTOR_WSTRING) {

        size_t old_read_pos = m_impl->m_read_pos;
        size_t num;
        WideString str;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        num = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        vec.clear ();

        for (size_t i=0; i<num; i++) {
            if (!get_data (str)) {
                m_impl->m_read_pos = old_read_pos;
                return false;
            }
            vec.push_back (str);
        }

        return true;
    }
    return false;
}

bool
TransactionReader::get_data (char **raw, size_t &bufsize)
{
    if (valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_RAW) {

        size_t old_read_pos = m_impl->m_read_pos;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        bufsize = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        if (!bufsize || m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + bufsize)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        if (raw) {
            *raw = new char [bufsize];
            if (! (*raw)) {
                m_impl->m_read_pos = old_read_pos;
                return false;
            }

            memcpy (*raw, m_impl->m_holder->m_buffer + m_impl->m_read_pos, bufsize);
        }

        m_impl->m_read_pos += bufsize;
        return true;
    }
    return false;
}

bool
TransactionReader::get_data (Transaction &trans)
{
    if (valid () && trans.valid () &&
        m_impl->m_holder->m_write_pos > m_impl->m_read_pos &&
        m_impl->m_holder->m_buffer [m_impl->m_read_pos] == SCIM_TRANS_DATA_TRANSACTION) {

        size_t len;
        size_t old_read_pos = m_impl->m_read_pos;

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
            return false;

        m_impl->m_read_pos ++;

        len = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos);
        m_impl->m_read_pos += sizeof (uint32);

        if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + len)) {
            m_impl->m_read_pos = old_read_pos;
            return false;
        }

        trans.m_holder->request_buffer_size (len);

        memcpy (trans.m_holder->m_buffer, m_impl->m_holder->m_buffer + m_impl->m_read_pos, len);

        trans.m_holder->m_write_pos = len;
        trans.m_reader->rewind ();

        m_impl->m_read_pos += len;

        return true;
    }
    return false;
}

bool
TransactionReader::skip_data ()
{
    if (valid () && m_impl->m_holder->m_write_pos > m_impl->m_read_pos) {
        switch (m_impl->m_holder->m_buffer [m_impl->m_read_pos]) {
            case SCIM_TRANS_DATA_COMMAND:
            {
                int cmd;
                return get_command (cmd);
            }
            case SCIM_TRANS_DATA_UINT32:
            {
                uint32 val;
                return get_data (val);
            }
            case SCIM_TRANS_DATA_STRING:
            {
                String str;
                return get_data (str);
            }
            case SCIM_TRANS_DATA_WSTRING:
            {
                WideString wstr;
                return get_data (wstr);
            }
            case SCIM_TRANS_DATA_KEYEVENT:
            {
                KeyEvent key;
                return get_data (key);
            }
            case SCIM_TRANS_DATA_ATTRIBUTE_LIST:
            {
                AttributeList attrs;
                return get_data (attrs);
            }
            case SCIM_TRANS_DATA_PROPERTY:
            {
                Property prop;
                return get_data (prop);
            }
            case SCIM_TRANS_DATA_PROPERTY_LIST:
            {
                PropertyList proplist;
                return get_data (proplist);
            }
            case SCIM_TRANS_DATA_LOOKUP_TABLE:
            {
                CommonLookupTable table;
                return get_data (table);
            }
            case SCIM_TRANS_DATA_VECTOR_UINT32:
            {
                std::vector <uint32> vec;
                return get_data (vec);
            }
            case SCIM_TRANS_DATA_VECTOR_STRING:
            {
                std::vector <String> vec;
                return get_data (vec);
            }
            case SCIM_TRANS_DATA_VECTOR_WSTRING:
            {
                std::vector <WideString> vec;
                return get_data (vec);
            }
            case SCIM_TRANS_DATA_RAW:
            {
                size_t bufsize;
                return get_data (NULL, bufsize);
            }
            case SCIM_TRANS_DATA_TRANSACTION:
            {
                size_t len;

                if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + sizeof (uint32) + 1))
                    return false;

                len = scim_bytestouint32 (m_impl->m_holder->m_buffer + m_impl->m_read_pos + 1);

                if (m_impl->m_holder->m_write_pos < (m_impl->m_read_pos + len + sizeof (uint32) + 1))
                    return false;

                m_impl->m_read_pos += (len + sizeof (uint32) + 1);
                return true;
            }
        }
    }
    return false;
}

void
TransactionReader::rewind ()
{
    m_impl->rewind ();
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/

