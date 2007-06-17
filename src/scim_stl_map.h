/** @file scim_stl_map.h
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_stl_map.h,v 1.1 2005/01/07 15:28:21 suzhe Exp $
 */

#ifndef __SCIM_STL_MAP_H
#define __SCIM_STL_MAP_H

#if ENABLE_HASH_MAP
 #if defined (HAVE_EXT_HASH_MAP)
  #define SCIM_USE_STL_EXT_HASH_MAP 1
  #include <ext/hash_map>
 #elif defined (HAVE_HASH_MAP)
  #define SCIM_USE_STL_HASH_MAP 1
  #include <hash_map>
 #endif
#endif

#if !defined (SCIM_USE_STL_HASH_MAP) && !defined (SCIM_USE_STL_EXT_HASH_MAP)
#define SCIM_USE_STL_MAP 1
#include <map>
#endif

namespace scim {

#if SCIM_USE_STL_EXT_HASH_MAP
class scim_hash_string {
    __gnu_cxx::hash <const char *> _h;
public:
    size_t operator ()(const String &str) const {
        return _h (str.c_str ());
    }
};
#elif SCIM_USE_STL_HASH_MAP
class scim_hash_string {
    std::hash <const char *> _h;
public:
    size_t operator ()(const String &str) const {
        return _h (str.c_str ());
    }
};
#endif

} // Namespace

#endif //__SCIM_STL_MAP_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
