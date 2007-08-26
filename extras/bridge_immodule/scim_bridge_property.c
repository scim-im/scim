/**
 * @file scim_bridge_property.h
 * @brief Implementation of properties.
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
 
#include <stdlib.h>

#include "scim_bridge_property.h"

struct _ScimPropertyList {
    size_t capacity;
    size_t size;
    ScimProperty elements;
};

ScimPropertyList *scim_alloc_property_list ()
{
    ScimPropertyList *property_list = malloc (sizeof (ScimPropertyList));
    property_list->size = 0;
    property_list->capacity = 20;
    property_list->elements = malloc (sizeof (ScimProperty) * property_list->capacity);
    return property_list;
}

void scim_free_property_list (ScimPropertyList *property_list)
{
    free (property_list->elements);
    free (property_list);
}

size_t scim_property_list_get_size (const ScimPropertyList *property_list)
{
    return property_list->size;
}

void scim_property_list_set_size (ScimPropertyList *property_list, size_t new_size)
{
    if (property_list->capacity < new_size) {
        property_list->capacity = new_size + 20;
        property_list->elements = realloc (sizeof (ScimPropertyList) * property_list->capacity);
    }
    
    property_list->size = new_size;
}

ScimProperty *scim_property_list_get (ScimPropertyList *property_list, size_t index)
{
    if (index < property_list->size) {
        return property_list->elements[index];
    } else {
        return NULL;
    }
}
