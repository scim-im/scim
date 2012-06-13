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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "scim-bridge-string.h"
#include "scim-bridge-output.h"

ssize_t scim_bridge_string_to_wstring (wchar **wstr, const char *str)
{
    if (str == NULL) {
        *wstr = NULL;
        scim_bridge_perrorln ("A NULL pointer is given as the UTF8 string at scim_bridge_string_to_wstring ()");
        return RETVAL_FAILED;
    }

    const size_t str_length = strlen (str);
    size_t str_index = 0;

    size_t wstr_capacity = 10;
    wchar *wstr_buffer = alloca (sizeof (wchar) * (wstr_capacity + 1));

    int i;
    for (i = 0; str_index <= str_length; ++i) {
        if (i > wstr_capacity) {
            const size_t new_wstr_capacity = wstr_capacity + 10;
            wchar *new_wstr_buffer = alloca (sizeof (wchar) * (new_wstr_capacity + 1));
            memcpy (new_wstr_buffer, wstr_buffer, sizeof (wchar) * (wstr_capacity + 1));
            wstr_buffer = new_wstr_buffer;
            wstr_capacity = new_wstr_capacity;
        }

        unsigned char a = '\0';
        unsigned char b = '\0';
        unsigned char c = '\0';
        unsigned char d = '\0';
        unsigned char e = '\0';
        unsigned char f = '\0';

        a = (unsigned char)str[str_index];
        if (str_index + 1 <= str_length) b = (unsigned char)str[str_index + 1];
        if (str_index + 2 <= str_length) c = (unsigned char)str[str_index + 2];
        if (str_index + 3 <= str_length) d = (unsigned char)str[str_index + 3];
        if (str_index + 4 <= str_length) e = (unsigned char)str[str_index + 4];
        if (str_index + 5 <= str_length) f = (unsigned char)str[str_index + 5];

        size_t wchar_size;
        if (a < 0x80) {
            wchar_size = 1;
        } else if (a < 0xc2) {
            wchar_size = 0;
        } else if (a < 0xe0) {
            wchar_size = 2;
        } else if (a < 0xf0) {
            wchar_size = 3;
        } else if (a < 0xf8) {
            wchar_size = 4;
        } else if (a < 0xfc) {
            wchar_size = 5;
        } else if (a < 0xfe) {
            wchar_size = 6;
        } else {
            wchar_size = 0;
        }

        switch (wchar_size) {
            case 1:
                wstr_buffer[i] = a;
                str_index += 1;
                break;
            case 2:
                if (str_index + 1 > str_length || ! ((b ^ 0x80) < 0x40)) {
                    *wstr = NULL;
                    scim_bridge_perrorln ("An invalid UTF8 string is given at scim_bridge_string_to_wstring ()");
                    return RETVAL_FAILED;
                } else {
                    wstr_buffer[i] = ((wchar) (a & 0x1f) << 6)
                        | (wchar) (b ^ 0x80);
                    str_index += 2;
                }
                break;
            case 3:
                if (str_index + 2 > str_length
                    || ! ((b ^ 0x80) < 0x40 && (c ^ 0x80) < 0x40
                    && (a >= 0xe1 || b >= 0xa0))) {
                    *wstr = NULL;
                    scim_bridge_perrorln ("An invalid UTF8 string is given at scim_bridge_string_to_wstring ()");
                    return RETVAL_FAILED;
                } else {
                    wstr_buffer[i] = ((wchar) (a & 0x0f) << 12)
                        | ((wchar) (b ^ 0x80) << 6)
                        | (wchar) (c ^ 0x80);
                    str_index += 3;
                }
                break;
            case 4:
                if (str_index + 3 > str_length
                    || ! ((b ^ 0x80) < 0x40 && (c ^ 0x80) < 0x40
                    && (d ^ 0x80) < 0x40 && (a >= 0xf1 || b >= 0x90))) {
                    *wstr = NULL;
                    scim_bridge_perrorln ("An invalid UTF8 string is given at scim_bridge_string_to_wstring ()");
                    return RETVAL_FAILED;
                } else {
                    wstr_buffer[i] = ((wchar) (a & 0x07) << 18)
                        | ((wchar) (b ^ 0x80) << 12)
                        | ((wchar) (c ^ 0x80) << 6)
                        | (wchar) (d ^ 0x80);
                    str_index += 4;
                }
                break;
            case 5:
                if (str_index + 4 > str_length
                    || ! ((b ^ 0x80) < 0x40 && (c ^ 0x80) < 0x40
                    && (d ^ 0x80) < 0x40 && (e ^ 0x80) < 0x40
                    && (a >= 0xf9 || b >= 0x88))) {
                    *wstr = NULL;
                    scim_bridge_perrorln ("An invalid UTF8 string is given at scim_bridge_string_to_wstring ()");
                    return RETVAL_FAILED;
                } else {
                    wstr_buffer[i] = ((wchar) (a & 0x03) << 24)
                        | ((wchar) (b ^ 0x80) << 18)
                        | ((wchar) (c ^ 0x80) << 12)
                        | ((wchar) (d ^ 0x80) << 6)
                        | (wchar) (e ^ 0x80);
                    str_index +=5;
                }
                break;
            case 6:
                if (str_index + 5 > str_length
                    || ! ((b ^ 0x80) < 0x40 && (c ^ 0x80) < 0x40
                    && (d ^ 0x80) < 0x40 && (e ^ 0x80) < 0x40
                    && (f ^ 0x80) < 0x40 && (a >= 0xfd || b >= 0x84))) {
                    *wstr = NULL;
                    scim_bridge_perrorln ("An invalid UTF8 string is given at scim_bridge_string_to_wstring ()");
                    return RETVAL_FAILED;
                } else {
                    wstr_buffer[i] = ((wchar) (a & 0x01) << 30)
                        | ((wchar) (b ^ 0x80) << 24)
                        | ((wchar) (c ^ 0x80) << 18)
                        | ((wchar) (d ^ 0x80) << 12)
                        | ((wchar) (e ^ 0x80) << 6)
                        | (wchar) (f ^ 0x80);
                    str_index += 6;
                }
            default:
                *wstr = NULL;
                scim_bridge_perrorln ("An invalid UTF8 string is given at scim_bridge_string_to_wstring ()");
                return RETVAL_FAILED;
        }
    }

    const size_t wstr_length = i - 1;
    *wstr = malloc (sizeof (wchar) * (wstr_length + 1));
    memcpy (*wstr, wstr_buffer, sizeof (wchar) * (wstr_length + 1));
    return wstr_length;
}


ssize_t scim_bridge_wstring_to_string (char **str, const wchar *wstr)
{
    if (wstr == NULL) {
        *str = NULL;
        scim_bridge_perrorln ("A NULL pointer is given as the UCS4 string at scim_bridge_wstring_to_string ()");
        return RETVAL_FAILED;
    }

    const size_t wstr_length = scim_bridge_wstring_get_length (wstr);
    size_t str_index = 0;

    size_t str_capacity = 10;
    char *str_buffer = alloca (sizeof (char) * (str_capacity + 1));

    int i;
    for (i = 0; i <= wstr_length; ++i) {
        wchar wc = wstr[i];

        size_t wc_size_in_utf8;
        if (wc < 0x80) {
            wc_size_in_utf8 = 1;
        } else if (wc < 0x800) {
            wc_size_in_utf8 = 2;
        } else if (wc < 0x10000) {
            wc_size_in_utf8 = 3;
        } else if (wc < 0x200000) {
            wc_size_in_utf8 = 4;
        } else if (wc < 0x4000000) {
            wc_size_in_utf8 = 5;
        } else if (wc <= 0x7fffffff) {
            wc_size_in_utf8 = 6;
        } else {
            wc_size_in_utf8 = 0;
        }

        if (wc_size_in_utf8 == 0) {
            *str = NULL;
            scim_bridge_perrorln ("An invalid UCS4 string is given at scim_bridge_wstring_to_string ()");
            return RETVAL_FAILED;
        }

        if (str_index + wc_size_in_utf8 > str_capacity) {
            const size_t new_str_capacity = str_capacity + 10;
            char *new_str_buffer = alloca (sizeof (char) * (new_str_capacity + 1));
            strncpy (new_str_buffer, str_buffer, str_capacity + 1);
            str_buffer = new_str_buffer;
            str_capacity = new_str_capacity;
        }
        /* note: code falls through cases! */
        switch (wc_size_in_utf8) {
            case 6: str_buffer[str_index + 5] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x4000000;
            case 5: str_buffer[str_index + 4] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x200000;
            case 4: str_buffer[str_index + 3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
            case 3: str_buffer[str_index + 2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
            case 2: str_buffer[str_index + 1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
            case 1: str_buffer[str_index + 0] = wc;
        }
        str_index += wc_size_in_utf8;
    }

    const size_t str_length = str_index - 1;
    *str = malloc (sizeof (char) * (str_length + 1));
    strcpy (*str, str_buffer);
    return str_length;
}


ssize_t scim_bridge_wstring_get_length (const wchar *wstr)
{
    if (wstr == NULL) {
        scim_bridge_perrorln ("A NULL pointer is given as the UCS4 string at scim_bridge_wstring_get_length ()");
        return RETVAL_FAILED;
    }

    size_t i;
    for (i = 0; wstr[i] != L'\0'; ++i);

    return i;
}


ssize_t scim_bridge_string_get_length (const char *str)
{
    if (str == NULL) {
        scim_bridge_perrorln ("A NULL pointer is given as the UTF8 string at scim_bridge_string_get_length ()");
        return RETVAL_FAILED;
    }

    return strlen (str);
}


retval_t scim_bridge_string_to_uint (unsigned int *dst, const char *str)
{
    if (str == NULL) {
        scim_bridge_perrorln ("A NULL pointer is given as a string at scim_bridge_string_to_uint ()");
        return RETVAL_FAILED;
    } else {
        unsigned long value = 0;

        size_t i;
        for (i = 0; str[i] != '\0'; ++i) {
            unsigned int digit;
            switch (str[i]) {
                case '0':
                    digit = 0;
                    break;
                case '1':
                    digit = 1;
                    break;
                case '2':
                    digit = 2;
                    break;
                case '3':
                    digit = 3;
                    break;
                case '4':
                    digit = 4;
                    break;
                case '5':
                    digit = 5;
                    break;
                case '6':
                    digit = 6;
                    break;
                case '7':
                    digit = 7;
                    break;
                case '8':
                    digit = 8;
                    break;
                case '9':
                    digit = 9;
                    break;
                default:
                    scim_bridge_perrorln ("An invalid char is given at scim_bridge_string_to_uint (): %c", str[i]);
                    return RETVAL_FAILED;
            }
            value *= 10;
            value += digit;
            if (value > UINT_MAX) {
                scim_bridge_perrorln ("An over flow exception occurred at scim_bridge_string_to_uint ()");
                return RETVAL_FAILED;
            }
        }

        *dst = value;
        return RETVAL_SUCCEEDED;
    }
}


retval_t scim_bridge_string_to_int (int *dst, const char *str)
{
    if (str == NULL) {
        scim_bridge_perrorln ("A NULL pointer is given as a string at scim_bridge_string_to_int ()");
        return RETVAL_FAILED;
    } else {
        unsigned long value = 0;
        boolean negative = FALSE;

        size_t i;
        for (i = 0; str[i] != '\0'; ++i) {
            unsigned int digit;
            switch (str[i]) {
                case '-':
                    if (i == 0) {
                        negative = TRUE;
                    } else {
                        scim_bridge_perrorln ("Negative signs cannot be given at the middle of the number at scim_bridge_string_to_int (): %s", str);
                        return RETVAL_FAILED;
                    }
                case '0':
                    digit = 0;
                    break;
                case '1':
                    digit = 1;
                    break;
                case '2':
                    digit = 2;
                    break;
                case '3':
                    digit = 3;
                    break;
                case '4':
                    digit = 4;
                    break;
                case '5':
                    digit = 5;
                    break;
                case '6':
                    digit = 6;
                    break;
                case '7':
                    digit = 7;
                    break;
                case '8':
                    digit = 8;
                    break;
                case '9':
                    digit = 9;
                    break;
                default:
                    scim_bridge_perrorln ("An invalid char is given at scim_bridge_string_to_int (): %c", str[i]);
                    return RETVAL_FAILED;
            }
            value *= 10;
            value += digit;
            if (!negative) {
                if (value > INT_MAX) {
                    scim_bridge_perrorln ("An over flow exception occurred at scim_bridge_string_to_int ()");
                    return RETVAL_FAILED;
                }
            } else {
                if (-((long) value) < INT_MIN) {
                    scim_bridge_perrorln ("An over flow exception at scim_bridge_string_to_int ()");
                    return RETVAL_FAILED;
                }
            }
        }

        if (!negative) {
            *dst = value;
        } else {
            *dst = -value;
        }
        return RETVAL_SUCCEEDED;
    }
}


retval_t scim_bridge_string_to_boolean (boolean *dst, const char *str)
{
    if (str == NULL) {
        scim_bridge_perrorln ("A NULL pointer is given as a string at scim_bridge_string_to_boolean ()");
        return RETVAL_FAILED;
    } else {
        if (strcmp (str, "TRUE") == 0 || strcmp (str, "True") == 0 || strcmp (str, "true") == 0) {
            *dst = TRUE;
            return RETVAL_SUCCEEDED;
        } else if (strcmp (str, "FALSE") == 0 || strcmp (str, "False") == 0 || strcmp (str, "false") == 0) {
            *dst = FALSE;
            return RETVAL_SUCCEEDED;
        } else {
            scim_bridge_perrorln ("An invalid string is given at scim_bridge_string_to_boolean (): %s", str);
            return RETVAL_FAILED;
        }
    }
}


size_t scim_bridge_string_from_uint (char **str, unsigned int value)
{
    const size_t str_length = snprintf (NULL, 0, "%u", value);
    *str = malloc (sizeof (char) * (str_length + 1));
    sprintf (*str, "%u", value);
    return str_length;
}


size_t scim_bridge_string_from_int (char **str, int value)
{
    const size_t str_length = snprintf (NULL, 0, "%d", value);
    *str = malloc (sizeof (char) * (str_length + 1));
    sprintf (*str, "%d", value);
    return str_length;
}


size_t scim_bridge_string_from_boolean (char **str, boolean value)
{
    if (value == TRUE) {
        *str = malloc (sizeof (char) * 5);
        strcpy (*str, "TRUE");
        return 4;
    } else {
        *str = malloc (sizeof (char) * 6);
        strcpy (*str, "FALSE");
        return 5;
    }
}
