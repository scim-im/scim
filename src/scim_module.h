/** @file scim_module.h
 * @brief definition of Module related classes.
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
 * $Id: scim_module.h,v 1.19 2005/01/10 08:30:54 suzhe Exp $
 */

#pragma once

namespace scim {

/**
 * @addtogroup Accessories
 * @{
 */

class ModuleError: public Exception
{
public:
    ModuleError (const String& what_arg)
        : Exception (String("scim::Module: ") + what_arg) { }
};

class Module 
{
    class ModuleImpl;
    ModuleImpl *m_impl;

    Module (const Module &);
    Module & operator= (const Module &);

public:
    Module ();
    Module (const String &name, const String &type);
    ~Module ();

    /**
     * @brief Load a module, sharing it with any other holder in this process.
     *
     * Several Module objects may hold the same module at once; they share one
     * mapping, and the module's scim_module_init () runs only for the first of
     * them. scim_module_exit () runs when the last one unloads, so a module's
     * global state outlives every individual holder.
     */
    bool load (const String &name, const String &type);

    /**
     * @brief Release this holder's claim on the module.
     *
     * Finalizes the module only if this was the last holder.  Refuses, and
     * returns false, if the module was made resident.
     */
    bool unload ();

    bool valid () const;

    bool is_resident () const;
    bool make_resident () const;

    String get_path () const;

    void * symbol (const String & sym) const;
protected:
    void init();
};

int scim_get_module_list (std::vector <String>& mod_list, const String& type = "");

/** @} */

} // namespace scim

/*
vi:ts=4:ai:nowrap:expandtab
*/

