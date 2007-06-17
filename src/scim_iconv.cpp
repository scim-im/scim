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
 * $Id: scim_iconv.cpp,v 1.18.2.1 2007/04/14 08:35:54 suzhe Exp $
 *
 */

#define Uses_SCIM_ICONV
#define Uses_C_LIMITS
#include "scim_private.h"
#include "scim.h"

namespace scim {

struct IConvert::IConvertImpl
{
    String  m_encoding;
    iconv_t m_iconv_from_unicode;
    iconv_t m_iconv_to_unicode;

    IConvertImpl ()
        : m_iconv_from_unicode ((iconv_t)-1),
          m_iconv_to_unicode ((iconv_t)-1) {
    }

    ~IConvertImpl () {
        if (m_iconv_from_unicode != (iconv_t) -1)
            iconv_close (m_iconv_from_unicode);
        if (m_iconv_to_unicode != (iconv_t) -1)
            iconv_close (m_iconv_to_unicode);
    }
};

IConvert::IConvert (const String& encoding)
    : m_impl (new IConvertImpl) 
{
    set_encoding (encoding);
}

IConvert::IConvert (const IConvert & iconvert)
    : m_impl (new IConvertImpl) 
{
    set_encoding (iconvert.m_impl->m_encoding);
}

IConvert::~IConvert ()
{
    delete m_impl;
}

const IConvert & 
IConvert::operator= (const IConvert & iconvert)
{
    if (this != &iconvert)
        set_encoding (iconvert.m_impl->m_encoding);

    return *this;
}

bool
IConvert::set_encoding (const String& encoding)
{
    if (encoding.length () == 0) {
        if (m_impl->m_iconv_from_unicode != (iconv_t) -1)
            iconv_close (m_impl->m_iconv_from_unicode);
        if (m_impl->m_iconv_to_unicode != (iconv_t) -1)
            iconv_close (m_impl->m_iconv_to_unicode);
        m_impl->m_iconv_from_unicode = (iconv_t) -1;
        m_impl->m_iconv_to_unicode = (iconv_t) -1;
        return true;
    }

    if (m_impl->m_iconv_from_unicode != (iconv_t) -1 &&
        m_impl->m_iconv_to_unicode != (iconv_t) -1 &&
        encoding == m_impl->m_encoding)
        return true;

    iconv_t new_iconv_from_unicode;
    iconv_t new_iconv_to_unicode;

    if (scim_is_little_endian ()) {
        new_iconv_from_unicode = iconv_open (encoding.c_str (), "UCS-4LE");
        new_iconv_to_unicode = iconv_open ("UCS-4LE", encoding.c_str ());
    } else {
        new_iconv_from_unicode = iconv_open (encoding.c_str (), "UCS-4BE");
        new_iconv_to_unicode = iconv_open ("UCS-4BE", encoding.c_str ());
    }

    if (new_iconv_from_unicode == (iconv_t) -1 ||
        new_iconv_to_unicode == (iconv_t) -1) {

        if (new_iconv_from_unicode != (iconv_t) -1)
            iconv_close (new_iconv_from_unicode);
        if (new_iconv_to_unicode != (iconv_t) -1)
            iconv_close (new_iconv_to_unicode);

        return false;
    }

    if (m_impl->m_iconv_from_unicode != (iconv_t) -1)
        iconv_close (m_impl->m_iconv_from_unicode);
    if (m_impl->m_iconv_to_unicode != (iconv_t) -1)
        iconv_close (m_impl->m_iconv_to_unicode);

    m_impl->m_iconv_from_unicode = new_iconv_from_unicode;
    m_impl->m_iconv_to_unicode = new_iconv_to_unicode;
    m_impl->m_encoding = encoding;

    return true;
}

String
IConvert::get_encoding () const
{
    return m_impl->m_encoding;
}

bool
IConvert::convert (String &dest, const ucs4_t *src, int src_len) const
{
    if (m_impl->m_iconv_from_unicode == (iconv_t) -1) return false;

    char dest_buf [SCIM_MAX_BUFSIZE * MB_LEN_MAX];

    size_t dest_buf_size = 0;
    size_t src_buf_size = 0;
    size_t ret;

    iconv (m_impl->m_iconv_from_unicode, 0, &src_buf_size, 0, &dest_buf_size); 

    char *dest_buf_ptr = dest_buf;
    ICONV_CONST char *src_buf_ptr = (ICONV_CONST char*) src;

    dest_buf_size = SCIM_MAX_BUFSIZE * MB_LEN_MAX;
    src_buf_size = src_len * sizeof (ucs4_t);

    ret = iconv (m_impl->m_iconv_from_unicode, &src_buf_ptr, &src_buf_size, &dest_buf_ptr, &dest_buf_size); 
    dest.assign (dest_buf, dest_buf_ptr);

    return ret != (size_t) -1;
}

bool
IConvert::convert (String &dest, const WideString &src) const
{
    return convert (dest, src.data (), src.length ());
}

bool
IConvert::test_convert (const ucs4_t *src, int src_len) const
{
    if (m_impl->m_iconv_from_unicode == (iconv_t) -1) return false;

    char dest_buf [SCIM_MAX_BUFSIZE * MB_LEN_MAX];
    size_t src_buf_size = 0;
    size_t dest_buf_size = 0;
    size_t ret;

    iconv (m_impl->m_iconv_from_unicode, 0, &src_buf_size, 0, &dest_buf_size); 

    char *dest_buf_ptr = dest_buf;
    ICONV_CONST char *src_buf_ptr = (ICONV_CONST char*) src;

    src_buf_size = src_len * sizeof (ucs4_t);
    dest_buf_size = SCIM_MAX_BUFSIZE * MB_LEN_MAX;

    ret = iconv (m_impl->m_iconv_from_unicode, &src_buf_ptr, &src_buf_size, &dest_buf_ptr, &dest_buf_size); 

    return ret != (size_t) -1;
}

bool
IConvert::test_convert (const WideString &src) const
{
    return test_convert (src.data (), src.length ());
}

bool
IConvert::convert (WideString &dest, const char *src, int src_len) const
{
    if (m_impl->m_iconv_to_unicode == (iconv_t) -1) return false;

    ucs4_t dest_buf [SCIM_MAX_BUFSIZE];

    size_t dest_buf_size = 0;
    size_t src_buf_size = 0;
    size_t ret;

    iconv (m_impl->m_iconv_to_unicode, 0, &src_buf_size, 0, &dest_buf_size); 

    char *dest_buf_ptr = (char*) dest_buf;
    ICONV_CONST char *src_buf_ptr = (ICONV_CONST char*) src;

    dest_buf_size = SCIM_MAX_BUFSIZE * sizeof (ucs4_t);
    src_buf_size = src_len;

    ret = iconv (m_impl->m_iconv_to_unicode, &src_buf_ptr, &src_buf_size, &dest_buf_ptr, &dest_buf_size); 
    dest.assign (dest_buf, (ucs4_t*) dest_buf_ptr);

    return ret != (size_t) -1;
}

bool
IConvert::convert (WideString &dest, const String &src) const
{
    return convert (dest, src.data (), src.length ());
}

bool
IConvert::test_convert (const char *src, int src_len) const
{
    if (m_impl->m_iconv_to_unicode == (iconv_t) -1) return false;

    ucs4_t dest_buf [SCIM_MAX_BUFSIZE];

    size_t src_buf_size = 0;
    size_t dest_buf_size = 0;
    size_t ret;

    iconv (m_impl->m_iconv_from_unicode, 0, &src_buf_size, 0, &dest_buf_size); 

    char *dest_buf_ptr = (char*) dest_buf;
    ICONV_CONST char *src_buf_ptr = (ICONV_CONST char*) src;

    src_buf_size = src_len;
    dest_buf_size = SCIM_MAX_BUFSIZE * sizeof (ucs4_t);

    ret = iconv (m_impl->m_iconv_to_unicode, &src_buf_ptr, &src_buf_size, &dest_buf_ptr, &dest_buf_size); 

    return ret != (size_t) -1;
}

bool
IConvert::test_convert (const String &src) const
{
    return test_convert (src.data (), src.length ());
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
