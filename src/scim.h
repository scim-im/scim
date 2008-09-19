/** @file scim.h
 *  all of the header files are included within this file.
 *  source files may include this file instead of others headers.
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
 * $Id: scim.h,v 1.38 2005/05/17 06:45:14 suzhe Exp $
 */

// Define the macros
#define Uses_SCIM_TYPES
#define Uses_SCIM_UTILITY
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_EXCEPTION
#define Uses_SCIM_DEBUG
#define Uses_SCIM_OBJECT
#define Uses_SCIM_SIGNALS
#define Uses_SCIM_SLOT
#define Uses_SCIM_CONNECTION
#define Uses_SCIM_BIND
#define Uses_SCIM_POINTER
#define Uses_STL_STRING
#define Uses_STL_VECTOR
#define Uses_STL_ALGORITHM
#define Uses_STL_NEW

#ifdef Uses_SCIM_FILTER_MANAGER
    #define Uses_SCIM_FILTER
#endif

#ifdef Uses_SCIM_FILTER_MODULE
    #define Uses_SCIM_MODULE
    #define Uses_SCIM_CONFIG_BASE
    #define Uses_SCIM_BACKEND
    #define Uses_SCIM_FILTER
#endif

#ifdef Uses_SCIM_FILTER
    #define Uses_SCIM_IMENGINE
#endif

#ifdef Uses_SCIM_PANEL
    #define Uses_SCIM_PANEL_AGENT
    #define Uses_SCIM_PANEL_CLIENT
#endif

#ifdef Uses_SCIM_PANEL_AGENT
    #define Uses_SCIM_HELPER_MANAGER
    #define Uses_SCIM_TRANSACTION
#endif

#ifdef Uses_SCIM_PANEL_CLIENT
    #define Uses_SCIM_TRANSACTION
#endif

#ifdef Uses_SCIM_HELPER_MANAGER
    #define Uses_SCIM_HELPER
#endif

#ifdef Uses_SCIM_HELPER_MODULE
    #define Uses_SCIM_HELPER
    #define Uses_SCIM_MODULE
    #define Uses_SCIM_CONFIG_BASE
#endif

#ifdef Uses_SCIM_HELPER
    #define Uses_SCIM_TRANSACTION
    #define Uses_SCIM_EVENT
#endif

#ifdef Uses_SCIM_COMPOSE_KEY
    #define Uses_SCIM_IMENGINE
#endif

#ifdef Uses_SCIM_TRANSACTION
    #define Uses_SCIM_EVENT
    #define Uses_SCIM_LOOKUP_TABLE
    #define Uses_SCIM_SOCKET
    #define Uses_SCIM_ATTRIBUTE
    #define Uses_SCIM_PROPERTY
    #define Uses_SCIM_TRANS_COMMANDS
#endif

#ifdef Uses_SCIM_CONFIG_MODULE
    #define Uses_SCIM_MODULE
    #define Uses_SCIM_CONFIG_BASE
#endif

#ifdef Uses_SCIM_IMENGINE_MODULE
    #define Uses_SCIM_MODULE
    #define Uses_SCIM_CONFIG_BASE
    #define Uses_SCIM_IMENGINE
#endif

#ifdef Uses_SCIM_FRONTEND_MODULE
    #define Uses_SCIM_MODULE
    #define Uses_SCIM_CONFIG_BASE
    #define Uses_SCIM_FRONTEND
#endif

#ifdef Uses_SCIM_ICONV
    #define Uses_C_ICONV
#endif

#ifdef Uses_SCIM_FRONTEND
    #define Uses_SCIM_BACKEND
    #define Uses_SCIM_IMENGINE
    #define Uses_SCIM_EVENT
    #define Uses_SCIM_LOOKUP_TABLE
    #define Uses_STL_MAP
    #define Uses_C_STDIO
    #define Uses_SCIM_ATTRIBUTE
    #define Uses_SCIM_PROPERTY
    #define Uses_SCIM_TRANSACTION
    #define Uses_SCIM_SOCKET
#endif

#ifdef Uses_SCIM_BACKEND
    #define Uses_SCIM_IMENGINE
    #define Uses_SCIM_CONFIG_BASE
    #define Uses_SCIM_COMPOSE_KEY
#endif

#ifdef Uses_SCIM_IMENGINE
    #define Uses_SCIM_EVENT
    #define Uses_SCIM_LOOKUP_TABLE
    #define Uses_SCIM_ATTRIBUTE
    #define Uses_SCIM_PROPERTY
    #define Uses_SCIM_TRANSACTION
    #define Uses_SCIM_SOCKET
#endif

#ifdef Uses_SCIM_LOOKUP_TABLE
    #define Uses_SCIM_EVENT
    #define Uses_SCIM_ATTRIBUTE
#endif

#ifdef Uses_SCIM_CONFIG_BASE
    #define Uses_SCIM_MODULE
    #define Uses_SCIM_CONFIG_MODULE
    #define Uses_STL_LIST
#endif

#ifdef Uses_SCIM_EXCEPTION
    #define Uses_STL_EXCEPTION
#endif

#ifdef Uses_SCIM_DEBUG
    #define Uses_STL_IOSTREAM
    #define Uses_STL_FSTREAM
#endif

#ifdef Uses_SCIM_UTILITY
    #define Uses_STL_IOSTREAM
#endif

#ifdef Uses_SCIM_HOTKEY
    #define Uses_SCIM_EVENT
    #define Uses_SCIM_CONFIG_BASE
    #define Uses_SCIM_CONFIG_PATH
#endif

// Include Standard headers
#ifdef Uses_STL_EXCEPTION
    #include <exception>
#endif

#ifdef Uses_STL_NEW
    #include <new>
#endif

#ifdef Uses_STL_IOSTREAM
    #include <iostream>
#endif

#ifdef Uses_STL_FSTREAM
    #include <fstream>
#endif

#ifdef Uses_STL_FUNCTIONAL
    #include <functional>
#endif

#ifdef Uses_STL_IOMANIP
    #include <iomanip>
#endif

#ifdef Uses_STL_MEMORY
    #include <memory>
#endif
    
#ifdef Uses_STL_VECTOR
    #include <vector>
#endif

#ifdef Uses_STL_LIST
    #include <list>
#endif

#ifdef Uses_STL_MAP
    #include <map>
#endif

#ifdef Uses_STL_QUEUE
    #include <queue>
#endif

#ifdef Uses_STL_ALGORITHM
    #include <algorithm>
#endif

#ifdef Uses_STL_UTILITY
    #include <utility>
#endif

#ifdef Uses_STL_STRING
    #include <string>
#endif

#ifdef Uses_STL_STRSTREAM
    #include <strstream>
#endif

#ifdef Uses_C_STDIO
    #include <cstdio>
#endif

#ifdef Uses_C_STDLIB
    #include <cstdlib>
#endif

#ifdef Uses_C_LOCALE
    #include <clocale>
#endif

#ifdef Uses_C_CTYPE
    #include <cctype>
#endif

#ifdef Uses_C_WCTYPE
    #include <cwctype>
#endif

#ifdef Uses_C_STRING
    #include <cstring>
#endif

#ifdef Uses_C_LIMITS
    #include <climits>
#endif

#ifdef Uses_C_ICONV
    #include <iconv.h>
#endif

//Include SCIM Headers
#ifdef Uses_SCIM_TYPES
    #include <scim_types.h>
#endif

#ifdef Uses_SCIM_DEBUG
    #include <scim_debug.h>
#endif

#ifdef Uses_SCIM_EXCEPTION
    #include <scim_exception.h>
#endif

#ifdef Uses_SCIM_EVENT
    #include <scim_event.h>
#endif

#ifdef Uses_SCIM_UTILITY
    #include <scim_utility.h>
#endif

#ifdef Uses_SCIM_GLOBAL_CONFIG
    #include <scim_global_config.h>
#endif

#ifdef Uses_SCIM_POINTER
    #include <scim_pointer.h>
#endif

#ifdef Uses_SCIM_OBJECT
    #include <scim_object.h>
#endif

#ifdef Uses_SCIM_SLOT
    #include <scim_slot.h>
#endif

#ifdef Uses_SCIM_CONNECTION
    #include <scim_connection.h>
#endif

#ifdef Uses_SCIM_SIGNALS
    #include <scim_signals.h>
#endif

#ifdef Uses_SCIM_BIND
    #include <scim_bind.h>
#endif

#ifdef Uses_SCIM_CONFIG_BASE
    #include <scim_config_base.h>
#endif

#ifdef Uses_SCIM_ATTRIBUTE
    #include <scim_attribute.h>
#endif

#ifdef Uses_SCIM_PROPERTY
    #include <scim_property.h>
#endif

#ifdef Uses_SCIM_LOOKUP_TABLE
    #include <scim_lookup_table.h>
#endif

#ifdef Uses_SCIM_ICONV
    #include <scim_iconv.h>
#endif

#ifdef Uses_SCIM_MODULE
    #include <scim_module.h>
#endif

#ifdef Uses_SCIM_SOCKET
    #include <scim_socket.h>
#endif

#ifdef Uses_SCIM_TRANSACTION
    #include <scim_transaction.h>
#endif

#ifdef Uses_SCIM_IMENGINE
    #include <scim_imengine.h>
#endif

#ifdef Uses_SCIM_IMENGINE_MODULE
    #include <scim_imengine_module.h>
#endif

#ifdef Uses_SCIM_COMPOSE_KEY
    #include <scim_compose_key.h>
#endif

#ifdef Uses_SCIM_BACKEND
    #include <scim_backend.h>
#endif

#ifdef Uses_SCIM_FRONTEND
    #include <scim_frontend.h>
#endif

#ifdef Uses_SCIM_FRONTEND_MODULE
    #include <scim_frontend_module.h>
#endif

#ifdef Uses_SCIM_CONFIG_MODULE
    #include <scim_config_module.h>
#endif

#ifdef Uses_SCIM_CONFIG_PATH
    #include <scim_config_path.h>
#endif

#ifdef Uses_SCIM_TRANS_COMMANDS
    #include <scim_trans_commands.h>
#endif

#ifdef Uses_SCIM_HELPER
    #include <scim_helper.h>
#endif

#ifdef Uses_SCIM_HELPER_MODULE
    #include <scim_helper_module.h>
#endif

#ifdef Uses_SCIM_HELPER_MANAGER
    #include <scim_helper_manager.h>
#endif

#ifdef Uses_SCIM_PANEL_AGENT
    #include <scim_panel_agent.h>
#endif

#ifdef Uses_SCIM_PANEL_CLIENT
    #include <scim_panel_client.h>
#endif

#ifdef Uses_SCIM_HOTKEY
    #include <scim_hotkey.h>
#endif

#ifdef Uses_SCIM_FILTER
    #include <scim_filter.h>
#endif

#ifdef Uses_SCIM_FILTER_MODULE
    #include <scim_filter_module.h>
#endif

#ifdef Uses_SCIM_FILTER_MANAGER
    #include <scim_filter_manager.h>
#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/
