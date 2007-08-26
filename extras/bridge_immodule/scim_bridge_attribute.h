/**
 * @file scim_bridge_attribute.h
 * @brief The definition for string attribute interfaces.
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
 */

#ifndef SCIM_BRIDGE_ATTRIBUTE_H_
#define SCIM_BRIDGE_ATTRIBUTE_H_

#include "scim-bridge-type.h"

/**
 * @brief Enum values of the valid attribute type.
 */
enum ScimAttributeType
{
    SCIM_ATTR_NONE = 0,         ///< No attribute.
    SCIM_ATTR_DECORATE = 1,     ///< A decorate attribute, eg. underline etc.
    SCIM_ATTR_FOREGROUND = 2,   ///< A foreground color attribute, in RGB format.
    SCIM_ATTR_BACKGROUND = 3,   ///< A background color attribute, in RGB format.
};

/**
 * The value for invalid text decorations.
 */
static const unsigned int SCIM_ATTR_DECORATE_NONE = 0;

/**
 * The value for underline text decorations.
 */
static const unsigned int SCIM_ATTR_DECORATE_UNDERLINE = 1;

/**
 * The value for invalid hilight text decorations.
 */
static const unsigned int SCIM_ATTR_DECORATE_HIGHLIGHT = 2;

/**
 * The value for reverse text decorations.
 */
static const unsigned int SCIM_ATTR_DECORATE_REVERSE = 4;

/**
 * @brief Data structure for string attributes.
 */
typedef struct _ScimAttribute
{
    uint32 start;
    uint32 length;
    ScimAttributeType type;
    uint32 value;
} ScimAttribute;

/**
 * @brief List of string attributes.
 */
typedef _ScimAttributeList ScimAttributeList;

/**
 * @brief Allocate a new string attribute list.
 * 
 * @return The new string attribute list.
 */
ScimAttributeList *scim_alloc_attribute_list ();

/**
 * @brief Free a string attribute list.
 * 
 * @param attribute_list The list to free.
 */
void scim_free_attribute_list (ScimAttributeList *attribute_list);

/**
 * @brief Get the size of the attribute list.
 * 
 * @param attribute_list The attribute list.
 * @return The size of the list.
 */
size_t scim_attribute_list_get_size (const ScimAttributeList *attribute_list);

/**
 * @biref Set the size of the attribute list.
 * 
 * @param attribute_list The attribute list.
 * @param new_size The new size.
 */
void scim_attribute_list_set_size (ScimAttributeList *attribute_list, size_t new_size);

/**
 * @brief Get an element from the attribute list.
 * 
 * @param index The index of the element.
 * @return The element in the given position, or NULL if the given index is out of bounds.
 */
ScimAttribute *scim_attribute_list_get (ScimAttributeList *attribute_list, size_t index);

/**
 * @brief Get the attribute value for the specific color.
 * 
 * @param value The destination for the result.
 * @param red The intensity of red.
 * @param green The intensity of green.
 * @param blue The intensity of blue.
 * @return true if it succeeds.
 */
bool scim_attribute_color_to_value (unsigned int *value, unsigned int red, unsigned int green, unsigned int blue);

/**
 * @brief Get the color for the given attribute value.
 * 
 * @param value The attribute value.
 * @param red The destination for the intensity of red.
 * @param green The destinatio for the intensity of green.
 * @param blue The destination for the intensity of blue.
 * @return true if it succeeds.
 */
bool scim_attribute_value_to_color (unsigned int value, unsigned int *red, unsigned int *green, unsigned int *blue);

#endif /*SCIM_BRIDGE_ATTRIBUTE_H_*/
