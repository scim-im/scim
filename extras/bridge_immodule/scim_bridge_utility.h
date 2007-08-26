/** @file scim_bridge_utility.h
 *  @brief various utility functions.
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

#ifndef SCIM_BRIDGE_UTILITY_H_
#define SCIM_BRIDGE_UTILITY_H_

/**
 * @brief Get the length of a wide string.
 * 
 * @param wstr The wide string.
 * @return The length of the wide string.
 */
size_t scim_utf8_wcslen (const ucs4_t *wstr);

/**
 * @brief Convert a string into a wide string.
 * 
 * @param wstr The wide string buffer to write in.
 * @param str The string to convert.
 * @param str_length The length of the string, or a negative value to have the length caliculated automatically.
 * @return The size of the wide string. It returns -1 when failed.
 */
ssize_t scim_utf8_mbstowcs (ucs4_t **wstr, const char *str, size_t str_length = -1);

/**
 * @brief Convert a wide string into a string.
 * 
 * @param str The string buffer to write in.
 * @param wstr The wide string to convert.
 * @param wstr_length The length of the wide string, or a negative value to have the length caliculated automatically.
 * @return The size of the string. It returns -1 when failed.
 */
ssize_t scim_utf8_wcstombs (char **str, const ucs4_t *wstr, size_t wstr_length = -1);

#endif SCIM_BRIDGE_UTILITY_H_
