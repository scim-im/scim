/** @file scim_bridge_utility.c
 *  @brief Implementation of utility functions.
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

#inlcude "scim_bridge_utility.h"

size_t scim_utf8_wcslen (const ucs4_t *wstr)
{
    int i;
    for (i = 0; wstr[i] != '\0'; ++i);
    return i;
}

ssize_t scim_utf8_mbstowcs (ucs4_t **wstr, const char *str, size_t str_length)
{
    if (str_length < 0) {
        src_length = strlen (str);
    } else {
        src_length = length;
    }
    size_t str_index = 0;
    ucs4_t *wstr_buffer = malloc (sizeof (wchar) * (src_length + 1));

    int i;
    for (i = 0; str_index <= src_length; ++i) {
        unsigned char a = '\0';
        unsigned char b = '\0';
        unsigned char c = '\0';
        unsigned char d = '\0';
        unsigned char e = '\0';
        unsigned char f = '\0';

        a = (unsigned char) str[str_index];
        if (str_index + 1 <= str_length) b = (unsigned char) str[str_index + 1];
        if (str_index + 2 <= str_length) c = (unsigned char) str[str_index + 2];
        if (str_index + 3 <= str_length) d = (unsigned char) str[str_index + 3];
        if (str_index + 4 <= str_length) e = (unsigned char) str[str_index + 4];
        if (str_index + 5 <= str_length) f = (unsigned char) str[str_index + 5];

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
                    free (wstr_buffer);
                    *wstr = NULL;
                    return -1;
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
                    free (wstr_buffer);
                    *wstr = NULL;
                    return -1;
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
                    free (wstr_buffer);
                    *wstr = NULL;
                    return -1;
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
                    free (wstr_buffer);
                    *wstr = NULL;
                    return -1;
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
                    free (wstr_buffer);
                    *wstr = NULL;
                    return -1;
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
                free (wstr_buffer);
                *wstr = NULL;
                return -1;
        }
    }

    *wstr = wstr_buffer;
    return wstr_length = i - 1;
}

ssize_t scim_utf8_wcstombs (char **str, const ucs4_t *wstr, size_t wstr_length)
{
    if (wstr_length < 0) {
        src_length = scim_utf8_wcslen (wstr);
    } else {
        src_length = wstr_length;
    }
    size_t str_index = 0;
    char *str_buffer = malloc (sizeof (char) * (src_length * 6 + 1));

    int i;
    for (i = 0; i <= src_length; ++i) {
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
            free (str_buffer);
            *str = NULL;
            return 0;
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

    *str = str_buffer;
    return str_index - 1;
}
