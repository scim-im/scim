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

#pragma once

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace scim {

// Hash functor for String keys. std::hash<String> works directly too; this
// type is kept for source compatibility with code that names it explicitly.
class scim_hash_string {
public:
    size_t operator ()(const String &str) const {
        return std::hash <String> () (str);
    }
};

// scim_map / scim_set select between the ordered (red-black tree) and the
// unordered (hash table) standard containers at build time.
//
// Ordered is the default. Nothing SCIM keeps in these is large -- a few hundred
// entries at the very most -- so the lookup difference is not measurable, while
// a defined iteration order is worth having: the simple Config module writes
// ~/.scim/config by walking one of these, and a panel or setup list built by
// walking another comes out in whatever sequence the hash chose. Hash order also
// shifts whenever the table rehashes, so the same data reorders itself between
// runs.
//
// Configure with --enable-unordered-map for the hash containers. The Hash
// parameter is used by those and ignored by the ordered ones, which order by
// operator< instead -- every key type used here provides both.
#ifdef SCIM_USE_UNORDERED_MAP

template <typename Key, typename Value, typename Hash = std::hash <Key> >
using scim_map = std::unordered_map <Key, Value, Hash>;

template <typename Key, typename Hash = std::hash <Key> >
using scim_set = std::unordered_set <Key, Hash>;

#else

template <typename Key, typename Value, typename Hash = std::hash <Key> >
using scim_map = std::map <Key, Value>;

template <typename Key, typename Hash = std::hash <Key> >
using scim_set = std::set <Key>;

#endif

} // Namespace

/*
vi:ts=4:nowrap:ai:expandtab
*/
