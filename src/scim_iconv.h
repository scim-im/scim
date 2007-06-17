/** @file scim_iconv.h
 *  @brief definition of IConvert related classes.
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
 * $Id: scim_iconv.h,v 1.18 2005/08/15 12:45:46 suzhe Exp $
 */

#ifndef __SCIM_ICONVERT_H
#define __SCIM_ICONVERT_H

namespace scim {

/**
 * @addtogroup Accessories
 * @{
 */

#define SCIM_MAX_BUFSIZE    4096

/**
 * @brief A class to convert strings between UCS-4 and local encodings.
 */
class IConvert
{
    class IConvertImpl;

    IConvertImpl * m_impl;

public:
    /**
     * @brief Constructor
     * @param encoding the local encoding to be used.
     */
    IConvert (const String& encoding = String ());

    /**
     * @brief Copy constructor
     */
    IConvert (const IConvert & iconvert);

    ~IConvert ();

    /**
     * @brief Assign operator
     */
    const IConvert & operator = (const IConvert & iconvert);

    /**
     * @brief Set the working local encoding.
     * @param encoding the local encoding to be used.
     * @return whether the encoding is ok or not.
     */
    bool set_encoding (const String& encoding);

    /**
     * @brief Get the current working local encoding. 
     * @return The name of the local encoding, like "UTF-8", "GB2312" etc.
     */
    String get_encoding () const;

    /**
     * @brief Convert a UCS-4 encoded WideString into a local encoded String.
     * @param dest the result string will be stored here.
     * @param src the WideString to be converted.
     * @return true if success.
     */
    bool convert (String &dest, const WideString &src) const;

    /**
     * @brief Convert a UCS-4 encoded WideString into a local encoded String.
     * @param dest the result string will be stored here.
     * @param src the ucs-4 encoded string to be converted.
     * @param src_len the length of source string.
     * @return true if success.
     */
    bool convert (String &dest, const ucs4_t *src, int src_len) const;

    /**
     * @brief Convert a local encoded String into a UCS-4 encoded WideString.
     * @param dest the result string will be stored here.
     * @param src the local encoded string to be converted.
     * @return ture if success.
     */
    bool convert (WideString &dest, const String &src) const;

    /**
     * @brief Convert a local encoded String into a UCS-4 encoded WideString.
     * @param dest the result string will be stored here.
     * @param src the local encoded string to be converted.
     * @param src_len the length of source string.
     * @return ture if success.
     */
    bool convert (WideString &dest, const char *src, int src_len) const;

    /**
     * @brief Test if a UCS-4 encoded WideString can be converted to a local encoded String.
     * @param src the ucs-4 encoded string to be test.
     * @return true if it can be converted without any problem.
     */
    bool test_convert (const WideString &src) const;

    /**
     * @brief Test if a ucs-4 encoded string can be converted to a local encoded String.
     * @param src the ucs-4 encoded string to be test.
     * @param src_len the length of source string.
     * @return true if it can be converted without any problem.
     */
    bool test_convert (const ucs4_t *src, int src_len) const;

    /**
     * @brief Test if a local encoded string can be converted to a UCS-4 encoded WideString.
     * @param src the local encoded string to be test.
     * @return true if it can be converted without any problem.
     */
    bool test_convert (const String &src) const;

    /**
     * @brief Test if a local encoded string can be converted to a UCS-4 encoded WideString.
     * @param src the local encoded string to be test.
     * @param src_len the length of source string.
     * @return true if it can be converted without any problem.
     */
    bool test_convert (const char *src, int src_len) const;
};

/** @} */

} // namespace scim

#endif //__SCIM_ICONVERT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
