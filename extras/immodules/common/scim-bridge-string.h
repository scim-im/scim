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
 * @brief This is the header for the functions to manupulate strings.
 */

#ifndef SCIMBRIDGESTRING_H_
#define SCIMBRIDGESTRING_H_

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "scim-bridge.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __STDC_ISO_10646__
    /**
     * The type for wide string.
     */
    typedef uint32_t wchar;
#else
    /**
     * The type for wide string.
     */
    typedef wchar_t wchar;
#endif

    /**
     * Translate an utf8 string into an ucs4 wide string.
     *
     * @param str String to translate.
     * @param wstr The destination for the new wide string.
     * @return The length of the new wide string. -1 means that it failed to translate.
     */
    ssize_t scim_bridge_string_to_wstring (wchar **wstr, const char *str);

    /**
     * Translate an ucs4 wide string into an utf8 string.
     *
     * @param wstr Wide string to translate.
     * @param str The destination for the new string.
     * @return The length of the new string. -1 means that it failed to translate.
     */
    ssize_t scim_bridge_wstring_to_string (char **str, const wchar *wstr);

    /**
     * Get the length of a string.
     *
     * @param str The string.
     * @return The length of the string.
     */
    ssize_t scim_bridge_string_get_length (const char *str);

    /**
     * Get the length of a wide string.
     *
     * @param str The wide string.
     * @return The length of the wide string.
     */
    ssize_t scim_bridge_wstring_get_length (const wchar *wstr);

    /**
     * Translate a string into an unsigned integer.
     *
     * @param str The string to translate.
     * @param dst The destination pointer of uint.
     * @return It returns RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_string_to_uint (unsigned int *dst, const char *str);

    /**
     * Translate a string into a integer.
     *
     * @param str The string to translate.
     * @param dst The destination pointer of int.
     * @return It returns RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_string_to_int (int *dst, const char *str);

    /**
     * Translate a string into a boolean.
     *
     * @param str The string to translate.
     * @param dst The destination pointer of boolean.
     * @return It returns RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_string_to_boolean (boolean *dst, const char *str);

    /**
     * Translate an unsigned integer into a string.
     *
     * @param value The value to translate.
     * @param str The destination pointer for the new string.
     * @return The length of the new string if succeeded, otherwise it returns -1.
     */
    size_t scim_bridge_string_from_uint (char **str, unsigned int value);

    /**
     * Translate a integer into a string.
     *
     * @param value The value to translate.
     * @param str The destination pointer for the new string.
     * @return The length of the new string if succeeded, otherwise it returns -1.
     */
    size_t scim_bridge_string_from_int (char **str, int value);

    /**
     * Translate a boolean into a string.
     *
     * @param value The value to translate.
     * @param str The destination pointer for the new string.
     * @return The length of the new string if succeeded, otherwise it returns -1.
     */
    size_t scim_bridge_string_from_boolean (char **str, boolean value);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGESTRING_H_*/
