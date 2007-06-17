/**
 * @file scim_panel_common.h
 * @brief Defines some structures and types which are used by both scim::PanelAgent and scim::PanelClient.
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
 * $Id: scim_panel_common.h,v 1.4 2005/05/13 04:21:29 suzhe Exp $
 */

#ifndef __SCIM_PANEL_COMMON_H
#define __SCIM_PANEL_COMMON_H

namespace scim {

/**
 * @addtogroup Panel
 * The accessory classes to help develop Panel daemons and FrontEnds
 * which need to communicate with Panel daemons.
 * @{
 */

class PanelError: public Exception
{
public:
    PanelError (const String& what_arg)
        : Exception (String("scim::Panel: ") + what_arg) { }
};

/**
 * @brief Structure to hold factory information. It's used by PanelAgent and PanelClient classes.
 */
struct PanelFactoryInfo
{
    String uuid;
    String name;
    String lang;
    String icon;

    PanelFactoryInfo () { }
    PanelFactoryInfo (const String &u, const String &n, const String &l, const String &i)
        : uuid (u), name (n), lang (l), icon (i) { }
};

/**  @} */

} // namespace scim

#endif //__SCIM_PANEL_COMMON_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

