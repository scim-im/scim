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

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This header describes about attributes.
 *
 * Attributes are used to give the clients some hints how the preedit strings should be shown.\n
 * They are used to highlight the current segment in the convertion mode,
 * or to underline the newly-input-strings.
 */

#ifndef SCIMBRIDGEATTRIBUTE_H_
#define SCIMBRIDGEATTRIBUTE_H_

#include "scim-bridge.h"

enum _scim_bridge_attribute_type_t
{
    ATTRIBUTE_NONE,
    ATTRIBUTE_DECORATE,
    ATTRIBUTE_FOREGROUND,
    ATTRIBUTE_BACKGROUND
};

/**
 * This is enumeration type for the type of attributes.
 */
typedef enum _scim_bridge_attribute_type_t scim_bridge_attribute_type_t;

/**
 * This is type for the value or the color of attributes.
 */
typedef unsigned int scim_bridge_attribute_value_t;

/**
 * This is the type of attribute.
 */
typedef struct _ScimBridgeAttribute ScimBridgeAttribute;

/**
 * The attribute type, which means that the attribute is invalid.
 */
static const scim_bridge_attribute_value_t SCIM_BRIDGE_ATTRIBUTE_DECORATE_NONE = 0x1000000;

/**
 * The attribute type, which means that the attribute provides an underline.
 */
static const scim_bridge_attribute_value_t SCIM_BRIDGE_ATTRIBUTE_DECORATE_UNDERLINE = 0x2000000;

/**
 * The attribute type, which means that the attribute provides an highlight.
 */
static const scim_bridge_attribute_value_t SCIM_BRIDGE_ATTRIBUTE_DECORATE_HIGHLIGHT = 0x4000000;

/**
 * The attribute type, which means that the attribute provides an reversed highlight.
 */
static const scim_bridge_attribute_value_t SCIM_BRIDGE_ATTRIBUTE_DECORATE_REVERSE = 0x8000000;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Allocate an attribute.
     *
     * @return a new attribute.
     */
    ScimBridgeAttribute *scim_bridge_alloc_attribute ();

    /**
     * Free an attribute.
     *
     * @param attribute The attribute to free.
     * @note Do not use free (). It will cause memleaks.
     */
    void scim_bridge_free_attribute (ScimBridgeAttribute *attribute);

    /**
     * Copy an attribute into another.
     *
     * @param src The attribute to duplicate.
     * @param dst The destination to copy in.
     */
    void scim_bridge_copy_attribute (ScimBridgeAttribute *dst, const ScimBridgeAttribute *src);

    /**
     * See if one attribute equals the other.
     *
     * @param atr1 The attribute.
     * @param atr2 Another attribute.
     * @return true if the two attributes are the same.
     */
    boolean scim_bridge_attribute_equals (const ScimBridgeAttribute *atr1, const ScimBridgeAttribute *atr2);

    /**
     * Get the type of an attribute.
     *
     * @param attribute The attribute.
     * @return The type of the attribute.
     */
    scim_bridge_attribute_type_t scim_bridge_attribute_get_type (const ScimBridgeAttribute *attribute);

    /**
     * Set the type of an attribute.
     *
     * @param attribute The attribute.
     */
    void scim_bridge_attribute_set_type (ScimBridgeAttribute *attribute, scim_bridge_attribute_type_t type);

    /**
     * Get the begining index of the attribute.
     *
     * @param attribute The attribute.
     * @return The index of the first wide-character to apply this attribute.
     */
    size_t scim_bridge_attribute_get_begin (const ScimBridgeAttribute *attribute);

    /**
     * Set the begining index of the attribute.
     *
     * @param attribute The attribute.
     * @param begin The index of the first wide-character to apply this attribute.
     */
    void scim_bridge_attribute_set_begin (ScimBridgeAttribute *attribute, size_t begin);

    /**
     * Get the ending index of the attribute.
     *
     * @param attribute The attribute.
     * @return The index of the end wide-character to apply this attribute.
     */
    size_t scim_bridge_attribute_get_end (const ScimBridgeAttribute *attribute);

    /**
     * Set the ending index of the attribute.
     *
     * @param attribute The attribute.
     * @param end The index of the end wide-character to apply this attribute.
     */
    void scim_bridge_attribute_set_end (ScimBridgeAttribute *attribute, size_t end);

    /**
     * Get the type of the attribute.
     *
     * @param attribute The attribute.
     * @return The type of the attribute.
     */
    scim_bridge_attribute_type_t scim_bridge_attribute_get_type (const ScimBridgeAttribute *attribute);

    /**
     * Set the type of the attribute.
     *
     * @param attribute The attribute.
     * @param type The type of the attribute.
     */
    void scim_bridge_attribute_set_type (ScimBridgeAttribute *attribute, scim_bridge_attribute_type_t type);

    /**
     * Get the value of the attribute.\n
     * Notice, the value stands for RGB color from 0x000000 to 0xFFFFFF\n
     * if the attribute type is ATTRIBUTE_FOREGROUND or ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @return The value of the attribute.
     */
    scim_bridge_attribute_value_t scim_bridge_attribute_get_value (const ScimBridgeAttribute *attribute);

    /**
     * Set the value of the attribute.\n
     * Notice, the value stands for RGB color from 0x000000 to 0xFFFFFF\n
     * if the attribute type is ATTRIBUTE_FOREGROUND or ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @param value The value of the attribute.
     */
    void scim_bridge_attribute_set_value (ScimBridgeAttribute *attribute, scim_bridge_attribute_value_t value);

    /**
     * Set the color of the attribute.\n
     * The color value will be ignored
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @param red The red value, from 0x0 to 0xFF.
     * @param green The green value, from 0x0 to 0xFF.
     * @param blue The blue value, from 0x0 to 0xFF.
     */
    void scim_bridge_attribute_set_color (ScimBridgeAttribute *attribute, unsigned int red, unsigned int green, unsigned int blue);

    /**
     * Get the red value of the attribute.\n
     * The color value has no meanings
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @return The red value, from 0x0 to 0xFF.
     */
    unsigned int scim_bridge_attribute_get_red (const ScimBridgeAttribute *attribute);

    /**
     * Set the red value of the attribute.\n
     * The color value has no meanings
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @param red The red value, from 0x0 to 0xFF.
     */
    void scim_bridge_attribute_set_red (ScimBridgeAttribute *attribute, unsigned int red);

    /**
     * Get the green value of the attribute.\n
     * The color value has no meanings
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @return The green value, from 0x0 to 0xFF.
     */
    unsigned int scim_bridge_attribute_get_green (const ScimBridgeAttribute *attribute);

    /**
     * Set the green value of the attribute.\n
     * The color value has no meanings
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @param green The green value, from 0x0 to 0xFF.
     */
    void scim_bridge_attribute_set_green (ScimBridgeAttribute *attribute, unsigned int green);

    /**
     * Get the blue value of the attribute.\n
     * The color value has no meanings
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @return The blue value, from 0x0 to 0xFF.
     */
    unsigned int scim_bridge_attribute_get_blue (const ScimBridgeAttribute *attribute);

    /**
     * Set the blue value of the attribute.\n
     * The color value has no meanings
     * when the attribute type is not ATTRIBUTE_FOREGROUND nor ATTRIBUTE_BACKGROUND.
     *
     * @param attribute The attribute.
     * @param blue The blue value, from 0x0 to 0xFF.
     */
    void scim_bridge_attribute_set_blue (ScimBridgeAttribute *attribute, unsigned int blue);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEATTRIBUTE_H_*/
