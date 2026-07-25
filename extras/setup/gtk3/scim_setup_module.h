/** @file scim_setup_module.h
 *  @brief definition of SetupModule related classes.
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
 * $Id: scim_setup_module.h,v 1.9 2005/01/10 08:30:45 suzhe Exp $
 */

#if !defined (__SCIM_SETUP_MODULE_H)
#define __SCIM_SETUP_MODULE_H

#include <gtk/gtk.h>

using namespace scim;

typedef GtkWidget * (*SetupModuleCreateUIFunc) (void);
typedef String (*SetupModuleGetCategoryFunc) (void);
typedef String (*SetupModuleGetNameFunc) (void);
typedef String (*SetupModuleGetDescriptionFunc) (void);
typedef void (*SetupModuleLoadConfigFunc) (const ConfigPointer &config);
typedef void (*SetupModuleSaveConfigFunc) (const ConfigPointer &config);
typedef bool (*SetupModuleQueryChangedFunc) (void);

class SetupModule
{
    Module      m_module;

    SetupModuleCreateUIFunc       m_create_ui;
    SetupModuleGetCategoryFunc    m_get_category;
    SetupModuleGetNameFunc        m_get_name;
    SetupModuleGetDescriptionFunc m_get_description;
    SetupModuleLoadConfigFunc     m_load_config;
    SetupModuleSaveConfigFunc     m_save_config;
    SetupModuleQueryChangedFunc   m_query_changed;

    SetupModule (const SetupModule &);
    SetupModule & operator= (const SetupModule &);

public:
    SetupModule ();
    SetupModule (const String &name);

    bool load  (const String &name);

    bool valid () const;

    GtkWidget * create_ui () const;

    String get_category () const;
    String get_name () const;
    String get_description () const;

    void load_config (const ConfigPointer &config) const;
    void save_config (const ConfigPointer &config) const;

    bool query_changed () const;
};

int scim_get_setup_module_list (std::vector <String>& mod_list);

#endif // __SCIM_SETUP_MODULE_H

/*
vi:ts=4:ai:nowrap:expandtab
*/
