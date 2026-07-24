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
 * $Id: scim_module.cpp,v 1.26 2005/05/24 12:22:51 suzhe Exp $
 *
 */

#define Uses_SCIM_MODULE
#define Uses_STL_ALGORITHM
#include "scim_private.h"
#include "scim.h"
#include <dlfcn.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace scim {

typedef void (*ModuleInitFunc) (void);
typedef void (*ModuleExitFunc) (void);

struct Module::ModuleImpl
{
    void          *handle;
    ModuleInitFunc init;
    ModuleExitFunc exit;
    String         path;
    String         name;
    bool           resident;

    ModuleImpl () : handle (0), init (0), exit (0), resident (false) { }
};

static std::vector <ModuleInitFunc> _scim_modules;

static void
_scim_get_module_paths (std::vector <String> &paths, const String &type)
{
    const char *module_path_env;

    std::vector <String> module_paths;
    std::vector <String>::iterator it;

    paths.clear ();

    module_path_env = getenv ("SCIM_MODULE_PATH");

    if (module_path_env)
        module_paths.push_back (String (module_path_env));

    module_paths.push_back (String (SCIM_MODULE_PATH));

    // append version to the end of the paths
    for (it = module_paths.begin (); it != module_paths.end (); ++it) {
        String tmp_dir;

        tmp_dir = *it + String (SCIM_PATH_DELIM_STRING) +
                    String (SCIM_BINARY_VERSION) +
                    String (SCIM_PATH_DELIM_STRING) + type;

        paths.push_back (tmp_dir);

        tmp_dir = *it + String (SCIM_PATH_DELIM_STRING) + type;
        paths.push_back (tmp_dir);
    }
}

int
scim_get_module_list (std::vector <String>& mod_list, const String& type)
{
    std::vector<String> paths;
    _scim_get_module_paths (paths, type);

    mod_list.clear ();

    for (std::vector<String>::iterator i = paths.begin (); i!= paths.end (); ++i) {
        DIR *dir = opendir (i->c_str ());
        if (dir) {
            struct dirent *file = readdir (dir);
            while (file) {
                struct stat filestat;
                String absfn = *i + String (SCIM_PATH_DELIM_STRING) + file->d_name;
                stat (absfn.c_str (), &filestat);
                if (S_ISREG (filestat.st_mode)) {
                    std::vector<String> vec;
                    scim_split_string_list (vec, String (file->d_name), '.');
                    mod_list.push_back (vec [0]);
                }
                file = readdir (dir);
            }
            closedir (dir);
        }
    }
    std::sort (mod_list.begin (), mod_list.end ());
    mod_list.erase (std::unique (mod_list.begin(), mod_list.end()), mod_list.end());
    return mod_list.size ();
}

Module::Module ()
    : m_impl (new ModuleImpl)
{
    init ();
}

Module::Module (const String &name, const String &type)
    : m_impl (new ModuleImpl)
{
    init ();
    load (name, type);
}

void Module::init ()
{
    // Nothing to do: dlopen(3) needs no global initialization.
}

Module::~Module ()
{
    unload ();
    delete m_impl;
}

// Open a module given a base path (without extension), trying the shared
// object suffix first, then the base name as given.
//
// RTLD_LOCAL keeps each module's symbols out of the global scope: modules
// resolve libscim through their own DT_NEEDED, and their entry points are
// looked up on the specific handle (and namespaced with the <name>_LTX_
// prefix), so nothing relies on cross-module global symbol sharing.
// RTLD_NOW surfaces unresolved symbols at load time rather than on first call.
static const int _scim_dlopen_flags = RTLD_NOW | RTLD_LOCAL;

static void *
_scim_dlopen (const String &base, String &opened_path)
{
    String so = base + String (".so");

    void *handle = dlopen (so.c_str (), _scim_dlopen_flags);
    if (handle) {
        opened_path = so;
        return handle;
    }

    handle = dlopen (base.c_str (), _scim_dlopen_flags);
    if (handle) {
        opened_path = base;
        return handle;
    }

    return 0;
}

static String
_concatenate_ltdl_prefix (const String &name, const String &symbol)
{
    String prefix (name);

    for (size_t i=0; i<prefix.length (); i++)
        if (!isalnum ((int)prefix[i]))
            prefix[i] = '_';

    return prefix + String ("_LTX_") + symbol;
}

bool
Module::load (const String &name, const String &type)
{
    // If cannot unload original module (it's resident), then return false.
    if (is_resident ())
        return false;

    std::vector <String> paths;
    std::vector <String>::iterator it;

    String module_path;
    String new_path;

    void *new_handle = 0;

    ModuleInitFunc new_init;
    ModuleExitFunc new_exit;

    _scim_get_module_paths (paths, type);

    for (it = paths.begin (); it != paths.end (); ++it) {
        module_path = *it + String (SCIM_PATH_DELIM_STRING) + name;
        new_handle = _scim_dlopen (module_path, new_path);
        if (new_handle)
            break;
    }

    if (!new_handle)
        new_handle = _scim_dlopen (name, new_path);

    if (!new_handle)
        return false;

    String symbol;

    // Try to load the symbol scim_module_init
    symbol = "scim_module_init";
    new_init = (ModuleInitFunc) dlsym (new_handle, symbol.c_str ());

    // If symbol load failed, try to add LTX prefix and load again.
    // Built-in modules export their entry points as <name>_LTX_scim_*.
    if (!new_init) {
        symbol = _concatenate_ltdl_prefix (name, symbol);
        new_init = (ModuleInitFunc) dlsym (new_handle, symbol.c_str ());

        // Failed again? Try to prepend a under score to the symbol name.
        if (!new_init) {
            symbol.insert (symbol.begin (),'_');
            new_init = (ModuleInitFunc) dlsym (new_handle, symbol.c_str ());
        }
    }

    // Could not load the module!
    if (!new_init) {
        dlclose (new_handle);
        return false;
    }

    // Try to load the symbol scim_module_exit
    symbol = "scim_module_exit";
    new_exit = (ModuleExitFunc) dlsym (new_handle, symbol.c_str ());

    // If symbol load failed, try to add LTX prefix and load again.
    // Built-in modules export their entry points as <name>_LTX_scim_*.
    if (!new_exit) {
        symbol = _concatenate_ltdl_prefix (name, symbol);
        new_exit = (ModuleExitFunc) dlsym (new_handle, symbol.c_str ());

        // Failed again? Try to prepend a under score to the symbol name.
        if (!new_exit) {
            symbol.insert (symbol.begin (),'_');
            new_exit = (ModuleExitFunc) dlsym (new_handle, symbol.c_str ());
        }
    }

    //Check if the module is already loaded.
    if (std::find (_scim_modules.begin (), _scim_modules.end (), new_init)
        != _scim_modules.end ()) {
        dlclose (new_handle);
        return false;
    }

    if (unload ()) {
        _scim_modules.push_back (new_init);

        m_impl->handle = new_handle;
        m_impl->init   = new_init;
        m_impl->exit   = new_exit;
        m_impl->path   = new_path;
        m_impl->name   = name;

        try {
            m_impl->init ();
            return true;
        } catch (...) {
            unload ();
        }
    } else {
        dlclose (new_handle);
    }

    return false;
}

bool
Module::unload ()
{
    if (!m_impl->handle)
        return true;

    if (is_resident ())
        return false;

    if (m_impl->exit) {
        try { m_impl->exit (); } catch (...) { }
    }

    dlclose (m_impl->handle);

    std::vector <ModuleInitFunc>::iterator it =
        std::find (_scim_modules.begin (), _scim_modules.end (), m_impl->init);

    if (it != _scim_modules.end ())
        _scim_modules.erase (it);

    m_impl->handle   = 0;
    m_impl->init     = 0;
    m_impl->exit     = 0;
    m_impl->path     = String ();
    m_impl->name     = String ();
    m_impl->resident = false;

    return true;
}

bool
Module::make_resident () const
{
    // dlopen(3) has no "resident" flag; emulate it so unload() refuses to
    // release the module (the memory stays mapped for the process lifetime).
    if (m_impl->handle) {
        m_impl->resident = true;
        return true;
    }
    return false;
}

bool
Module::is_resident () const
{
    return (m_impl->handle && m_impl->resident);
}

bool
Module::valid () const
{
    return (m_impl->handle && m_impl->init);
}

String
Module::get_path () const
{
    return m_impl->path;
}

void *
Module::symbol (const String & sym) const
{
    void * func = 0;

    if (m_impl->handle) {
        String symbol = sym;
        func = dlsym (m_impl->handle, symbol.c_str ());
        if (!func) {
            symbol = _concatenate_ltdl_prefix (m_impl->name, symbol);
            func = dlsym (m_impl->handle, symbol.c_str ());
            if (!func) {
                symbol.insert (symbol.begin (), '_');
                func = dlsym (m_impl->handle, symbol.c_str ());
            }
        }
    }
    return func;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
