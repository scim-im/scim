/** @file scim_bridge_types.h
 *  @brief defines some basic data types.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2004 James Su <suzhe@tsinghua.org.cn>
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

#ifndef SCIM_BRIDGE_TYPES_H
#define SCIM_BRIDGE_TYPES_H

#include <stdint.h>

#ifdef __FreeBSD__
# include <osreldate.h>
# if __FreeBSD_version > 500035
#  define __STDC_ISO_10646__
# endif
#endif

typedef int16 int16_t;
typedef int32 int32_t;
typedef int64 int64_t;

typedef unsigned int16 uint16_t;
typedef unsigned int32 uint32_t;
typedef unsigned int64 uint64_t;

#ifdef __STDC_ISO_10646__
    typedef wchar_t ucs4_t;
#else
    typedef uint32 ucs4_t;
#endif

#endif //SCIM_BRIDGE_TYPES_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
