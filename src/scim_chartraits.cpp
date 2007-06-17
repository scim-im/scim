/** @file scim_chartraits.cpp
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
 *
 * $Id: scim_chartraits.cpp,v 1.12 2005/07/05 16:18:17 suzhe Exp $
 */

#include <string>
#include "scim_types.h"

using namespace scim;

#define GCC_VERSION (__GNUC__ * 10000 \
                    + __GNUC_MINOR__ * 100 \
                    + __GNUC_PATCHLEVEL__)

#if !defined(__STDC_ISO_10646__) && GCC_VERSION >= 30200

namespace std
{
  
template<>
void
char_traits<ucs4_t>::assign(char_type& __c1, const char_type& __c2)
{
    __c1 = __c2;
}

template<>
bool 
char_traits<ucs4_t>::eq(const char_type& __c1, const char_type& __c2)
{
    return __c1 == __c2;
}

template<>
bool 
char_traits<ucs4_t>::lt(const char_type& __c1, const char_type& __c2)
{
    return __c1 < __c2;
}

template<>
char_traits<ucs4_t>::char_type*
char_traits<ucs4_t>::assign(char_type* __s, size_t __n, char_type __a)
{
    char_type* dest = __s;
    while (__n-- > 0) 
        *(dest++) = __a;
    return __s;
}

template<>
char_traits<ucs4_t>::char_type*
char_traits<ucs4_t>::copy(char_type* __s1, const char_type* __s2, size_t __n)
{
    char_type* dest = __s1;
    const char_type* from = __s2;
    while (__n-- > 0)
        *(dest++) = *(from++);
    return __s1;
}

template<>
char_traits<ucs4_t>::char_type*
char_traits<ucs4_t>::move(char_type* __s1, const char_type* __s2, size_t __n)
{
    if (__s1 + __n > __s2) {
        char_type* dest = __s1 + __n - 1;
        const char_type* from = __s2;
        while (__n-- > 0)
            *(dest--) = *(from++);
        return __s1;
    } else {
        return copy(__s1, __s2, __n);
    }
}

template<>
size_t
char_traits<ucs4_t>::length(const char_type* __s)
{
    size_t __result = 0;
    while ( *(__s++) != 0 )
        __result++;
    return __result;
}

template<>
int
char_traits<ucs4_t>::compare(const char_type* __s1, const char_type* __s2, size_t __n)
{
    while ( (*__s1 == *__s2++) && __n-- > 0 )
        if (*__s1++ == 0)
            return (0);
    if (__n <= 0) return (0);
    return ( *__s1 - *(__s2 - 1) );
}

template<>
const char_traits<ucs4_t>::char_type*
char_traits<ucs4_t>::find(const char_type* __s, size_t __n, const char_type& __a)
{
    while (__n-- > 0) {
        if (*__s == __a)
            return __s;
        ++__s;
    }
    return 0;
}

template<>
char_traits<ucs4_t>::char_type
char_traits<ucs4_t>::to_char_type (const int_type& __c)
{
    return static_cast<char_type>(__c);
}

template<>
char_traits<ucs4_t>::int_type
char_traits<ucs4_t>::to_int_type (const char_type& __c)
{
    return static_cast<int_type>(__c);
}

template<>
bool
char_traits<ucs4_t>::eq_int_type(const int_type& __c1, const int_type& __c2)
{
    return __c1 == __c2;
}

template<>
char_traits<ucs4_t>::int_type
char_traits<ucs4_t>::eof ()
{
    return static_cast<int_type>(EOF);
}

template<>
char_traits<ucs4_t>::int_type
char_traits<ucs4_t>::not_eof (const int_type& __c)
{
    return (__c == eof()) ? 0 : __c;
}

}
#endif

/*
vi:ts=4:nowrap:ai:expandtab
*/
