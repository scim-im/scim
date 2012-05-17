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

#ifndef __SCIM_MODULE_H
#define __SCIM_MODULE_H

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

    bool load (const String &name, const String &type);
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

#endif //__SCIM_MODULE_H

/*
vi:ts=4:ai:nowrap:expandtab
*/

