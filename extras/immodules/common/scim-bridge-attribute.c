/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <string.h>

#include "scim-bridge-attribute.h"

/* Data type */
struct _ScimBridgeAttribute
{
    size_t begin;
    size_t end;

    scim_bridge_attribute_type_t type;
    scim_bridge_attribute_value_t value;
};

/* Functions */
ScimBridgeAttribute *scim_bridge_alloc_attribute ()
{
    ScimBridgeAttribute *attribute = malloc (sizeof (ScimBridgeAttribute));
    attribute->begin = 0;
    attribute->end = 0;
    attribute->type = ATTRIBUTE_NONE;
    attribute->value = SCIM_BRIDGE_ATTRIBUTE_DECORATE_NONE;

    return attribute;
}


void scim_bridge_free_attribute (ScimBridgeAttribute *attribute)
{
    if (attribute != NULL) free (attribute);
}


void scim_bridge_copy_attribute (ScimBridgeAttribute *dst, const ScimBridgeAttribute *src)
{
    if (dst == src) return;

    memcpy (dst, src, sizeof (ScimBridgeAttribute));
}

boolean scim_bridge_attribute_equals (const ScimBridgeAttribute *atr1, const ScimBridgeAttribute *atr2)
{
    return atr1->begin == atr2->begin && atr1->end == atr2->end && atr1->type == atr2->type && atr1->value == atr2->value;
}

scim_bridge_attribute_type_t scim_bridge_attribute_get_type (const ScimBridgeAttribute *attribute)
{
    return attribute->type;
}


void scim_bridge_attribute_set_type (ScimBridgeAttribute *attribute, scim_bridge_attribute_type_t type)
{
    attribute->type = type;
}


size_t scim_bridge_attribute_get_begin (const ScimBridgeAttribute *attribute)
{
    return attribute->begin;
}


void scim_bridge_attribute_set_begin (ScimBridgeAttribute *attribute, size_t begin)
{
    attribute->begin = begin;
}


size_t scim_bridge_attribute_get_end (const ScimBridgeAttribute *attribute)
{
    return attribute->end;
}


void scim_bridge_attribute_set_end (ScimBridgeAttribute *attribute, size_t end)
{
    attribute->end = end;
}


scim_bridge_attribute_value_t scim_bridge_attribute_get_value (const ScimBridgeAttribute *attribute)
{
    return attribute->value;
}


void scim_bridge_attribute_set_value (ScimBridgeAttribute *attribute, scim_bridge_attribute_value_t value)
{
    attribute->value = value;
}


void scim_bridge_attribute_set_color (ScimBridgeAttribute *attribute, unsigned int red, unsigned int green, unsigned int blue)
{
    attribute->value = (scim_bridge_attribute_value_t) (((red & 0xFF) << 16) | ((green & 0xFF) << 8) | ((blue & 0xFF) << 0));
}


unsigned int scim_bridge_attribute_get_red (const ScimBridgeAttribute *attribute)
{
    return ((unsigned int) (attribute->value & 0xFF0000)) >> 16;
}


void scim_bridge_attribute_set_red (ScimBridgeAttribute *attribute, unsigned int red)
{
    unsigned int green = scim_bridge_attribute_get_green (attribute);
    unsigned int blue = scim_bridge_attribute_get_blue (attribute);

    scim_bridge_attribute_set_color (attribute, red, green, blue);
}


unsigned int scim_bridge_attribute_get_green (const ScimBridgeAttribute *attribute)
{
    return ((unsigned int) (attribute->value & 0x00FF00)) >> 8;
}


void scim_bridge_attribute_set_green (ScimBridgeAttribute *attribute, unsigned int green)
{
    unsigned int red = scim_bridge_attribute_get_red (attribute);
    unsigned int blue = scim_bridge_attribute_get_blue (attribute);

    scim_bridge_attribute_set_color (attribute, red, green, blue);
}


unsigned int scim_bridge_attribute_get_blue (const ScimBridgeAttribute *attribute)
{
    return ((unsigned int) (attribute->value & 0x0000FF)) >> 0;
}


void scim_bridge_attribute_set_blue (ScimBridgeAttribute *attribute, unsigned int blue)
{
    unsigned int red = scim_bridge_attribute_get_red (attribute);
    unsigned int green = scim_bridge_attribute_get_green (attribute);

    scim_bridge_attribute_set_color (attribute, red, green, blue);
}
