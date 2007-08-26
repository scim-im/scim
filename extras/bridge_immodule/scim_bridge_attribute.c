/**
 * @file scim_bridge_attribute.c
 * @brief The implementation for string attribute interfaces.
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

#include "scim_bridge_attribute.h"

struct _ScimAttributeList {
    size_t size;
    size_t capacity;
    ScimAttribute *elements;
};

ScimAttributeList *scim_alloc_attribute_list ()
{
    ScimAttributeList *attribute_list = malloc (sizeof (ScimAttributeList));
    attribute_list->capacity = 10;
    attribute_list->size = 0;
    attribute_list->elements = malloc (sizeof (ScimAttribute) * attribute_list->capacity);
}

void scim_free_attribute_list (ScimAttributeList *attribute_list)
{
    free (attribute_list->elements);
    free (attribute_list);
}

size_t scim_attribute_list_get_size (const ScimAttributeList *attribute_list)
{
    return attribute_list->size;
}

void scim_attribute_list_set_size (ScimAttributeList *attribute_list, size_t new_size)
{
    if (new_size > attribute_list->capacity) {
        attribute_list->capacity = new_size + 10;
        attribute_list->elements = realloc (sizeof (ScimAttribute) * attribute_list->capacity);
    }
    attribute_list->size = new_size;
}

ScimAttribute *scim_attribute_list_get (ScimAttributeList *attribute_list, size_t index)
{
    if (index < attribute_list->size) {
        return attribute_list->elements[index];
    } else {
        return NULL;
    }
}

bool scim_attribute_color_to_value (unsigned int *value, unsigned int red, unsigned int green, unsigned int blue)
{
    if (red > 255 || green > 255 || blue > 255) {
        return false;
    } else {
        *value = (unsigned int) ((red << 16) | (green << 8) | (blue << 0));
        return true;
    }
}

bool scim_attribute_value_to_color (unsigned int value, unsigned int *red, unsigned int *green, unsigned int *blue)
{
    if (value > 0xFFFFFF) {
        return false;
    } else {
        *red = (value >> 16) & 0xFF;
        *green = (value >> 8) & 0xFF;
        *blue = (value >> 0) & 0xFF;
        return true;
    }
}
