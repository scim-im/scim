/**
 * @file scim_bridge_property.h
 * @brief Definition of properties.
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
#ifndef SCIM_BRIDGE_PROPERTY_H_
#define SCIM_BRIDGE_PROPERTY_H_

#include <stddef.h>

#include "scim_bridge_types.h"

/**
 * The flag to indicate that the property is active.
 */
static const uint16 SCIM_PROPERTY_ACTIVE = 0x01;

/**
 * The flag to indicate that the property is visible;
 */
static const uint16 SCIM_PROPERTY_VISIBLE = 0x02;

/**
 * Data struct of properties.
 */
typedef struct _ScimProperty
{
    char *key;
    char *label;
    char *icon;
    char *tip;
    uint16 state;
} ScimProperty;

/**
 * The list of properties.
 */
typedef struct _ScimPropertyList ScimPropertyList;

/**
 * Allocate a new property list.
 * 
 * @return The new property list.
 */
ScimPropertyList *scim_alloc_property_list ();

/**
 * Free a property list.
 * 
 * @param property_list The property list.
 */
void scim_free_property_list (ScimPropertyList *property_list);

/**
 * Get the size of the property list.
 * 
 * @param property_list The property list.
 * @return The size of the property list.
 */
size_t scim_property_list_get_size (const ScimPropertyList *property_list);

/**
 * Set the size of the property list.
 * 
 * @param property_list The property list.
 * @param new_size The new size of the property list.
 */
void scim_property_list_set_size (ScimPropertyList *property_list, size_t new_size);

/**
 * Get the property in the given position from the property list.
 * 
 * @param property_list The property list.
 * @param index The position of the property.
 * @return The property in the given position, or NULL if the position is out of bounds.
 */
ScimProperty *scim_property_list_get (ScimPropertyList *property_list, size_t index);

#endif /*SCIM_BRIDGE_PROPERTY_H_*/
