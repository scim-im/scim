/** @file scim_attribute.h
 *  @brief Definition of scim::Attribute and scim::AttributeList
 *
 *  Provide class scim::Attribute to control the
 *  drawing effect of strings.
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
 * $Id: scim_attribute.h,v 1.7 2005/08/05 16:12:31 suzhe Exp $
 */


#ifndef __SCIM_ATTRIBUTE_H
#define __SCIM_ATTRIBUTE_H

namespace scim {

/**
 * @addtogroup Accessories
 *
 * The accessorial classes and functions, including Attribute, IConvert, LookupTable etc.
 *
 * @{
 */

/**
 * @brief Enum values of the valid attribute type.
 */
enum AttributeType
{
    SCIM_ATTR_NONE,         ///< No attribute.
    SCIM_ATTR_DECORATE,     ///< A decorate attribute, eg. underline etc.
    SCIM_ATTR_FOREGROUND,   ///< A foreground color attribute, in RGB format.
    SCIM_ATTR_BACKGROUND    ///< A background color attribute, in RGB format.
};

const unsigned int SCIM_ATTR_DECORATE_NONE       = 0;    ///< No decorate
const unsigned int SCIM_ATTR_DECORATE_UNDERLINE  = 1;    ///< Draw a line under the text
const unsigned int SCIM_ATTR_DECORATE_HIGHLIGHT  = 2;    ///< Draw the text in highlighted color
const unsigned int SCIM_ATTR_DECORATE_REVERSE    = 4;    ///< Draw the text in reverse color mode

#define SCIM_RGB_COLOR(RED,GREEN,BLUE)  ((unsigned int)(((RED)<<16) + ((GREEN)<<8) + (BLUE)))
#define SCIM_RGB_COLOR_RED(COLOR)       ((unsigned int)((COLOR>>16) & 0x00ff))
#define SCIM_RGB_COLOR_GREEN(COLOR)     ((unsigned int)((COLOR>>8)  & 0x00ff))
#define SCIM_RGB_COLOR_BLUE(COLOR)      ((unsigned int)((COLOR)     & 0x00ff))

/**
 * @brief Class to store the string attributes.
 *
 * The string attributes control the effect of the string
 * drawn by FrontEnds. There are currently four valid types.
 *
 * A attribute could be one of the following types:
 *   - SCIM_ATTR_NONE        No attribute
 *   - SCIM_ATTR_DECORATE    Decorate attribute, eg. underline, highlight etc.
 *   - SCIM_ATTR_FOREGROUND  Foreground color attribute, in RGB format.
 *   - SCIM_ATTR_BACKGROUND  Background color attribute, in RGB format.
 *
 * For a DECORATE attribute, it can be one of the following values:
 *   - SCIM_ATTR_DECORATE_NONE        No decorate
 *   - SCIM_ATTR_DECORATE_UNDERLINE   Underline
 *   - SCIM_ATTR_DECORATE_HIGHLIGHT   Highlight
 *   - SCIM_ATTR_DECORATE_REVERSE     Reverse
 *
 * For a FOREGROUND or BACKGROUND attribute, it's a RGB color value generated with 
 * SCIM_RGB_COLOR (red,green,blue) macro.
 * You may use SCIM_RGB_COLOR_RED, SCIM_RGB_COLOR_GREEN and SCIM_RGB_COLOR_BLUE to extract
 * the RGB color later.
 */
class Attribute
{
    unsigned int  m_start;
    unsigned int  m_length;

    AttributeType m_type;
    unsigned int  m_value;

public:
    /**
     * @brief Constructor
     *
     * @param start - the start position in the string of this attribute.
     * @param length - the length of this attribute, the range is [start,start+length).
     * @param type - the type of this attribute.
     * @param value - the value of this attribute.
     */
    Attribute (unsigned int  start = 0,
               unsigned int  length = 0,
               AttributeType type  = SCIM_ATTR_NONE,
               unsigned int  value = 0) :
        m_start (start), m_length (length), m_type (type), m_value (value)
        { }

    /**
     * @brief Get the type of this attribute.
     *
     * @return The type of this attribute.
     */
    AttributeType get_type () const { return m_type; }

    /**
     * @brief Get the value of this attribute.
     *
     * @return The value of this attribute.
     */
    unsigned int  get_value () const { return m_value; }

    /**
     * @brief Get the start position of this attribute.
     * @return The start position of this attribute in the string.
     */
    unsigned int  get_start () const { return m_start; }

    /**
     * @brief Get the length of this attribute.
     * @return The length of this attribute in the string.
     */
    unsigned int  get_length () const { return m_length; }

    /**
     * @brief Get the end position of this attribute.
     * @return The end position of this attribute.
     */
    unsigned int  get_end () const { return m_start + m_length; }

    /**
     * @brief Set the type of this attribute.
     * @param type - the new attribute type to be set.
     */
    void set_type (AttributeType type) { m_type = type; }

    /**
     * @brief Set the value of this attribute.
     * @param value - the new attribute value to be set.
     */
    void set_value (unsigned int value) { m_value = value; }

    /**
     * @brief Set the start position of this attribute.
     * @param start - the new start position in the string.
     */
    void set_start (unsigned int start) { m_start = start; }

    /**
     * @brief Set the length of this attribute.
     * @param length - the new length of this attribute.
     */
    void set_length (unsigned int length) { m_length = length; }
};

inline bool
operator < (const Attribute &lhs, const Attribute &rhs)
{
    return lhs.get_start () < rhs.get_start () ||
           (lhs.get_start () == rhs.get_start () &&
            (lhs.get_length () < rhs.get_length () ||
             (lhs.get_length () == rhs.get_length () &&
              (lhs.get_type () < rhs.get_type () ||
               (lhs.get_type () == rhs.get_type () &&
                (lhs.get_value () < rhs.get_value ()))))));
}

/**
 * @typedef typedef std::vector<Attribute> AttributeList
 * @brief The container to store a set of Attribute objects.
 *
 * You should use the STL container methods to manipulate its objects.
 */
typedef std::vector<Attribute> AttributeList;

/** @} */

} // namespace scim

#endif //__SCIM_ATTRIBUTE_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
