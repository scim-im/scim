/*
 * Smart Common Input Method
 *
 * Copyright (c) 2026 SCIM developers
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

#define Uses_SCIM_UTILITY
#define Uses_STL_VECTOR

#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <libelf.h>
#include <gelf.h>

#include "scim_private.h"
#include "scim.h"
#include "scim_setup_classify.h"

// The SetupUI plugin directory, relative to a module search root. Kept in sync
// with the "SetupUI" type used by scim_get_setup_module_list ().
#define SETUP_MODULE_TYPE   "SetupUI"

// Build the list of directories the core module loader searches for a given
// module type. Mirrors the (static) _scim_get_module_paths () in the core so
// we can locate a plugin's .so file without loading it.
static void
_setup_get_module_paths (std::vector <String> &paths, const String &type)
{
    std::vector <String> roots;

    paths.clear ();

    const char *env = getenv ("SCIM_MODULE_PATH");
    if (env)
        roots.push_back (String (env));

    roots.push_back (String (SCIM_MODULE_PATH));

    for (size_t i = 0; i < roots.size (); ++i) {
        paths.push_back (roots [i] + String (SCIM_PATH_DELIM_STRING) +
                         String (SCIM_BINARY_VERSION) +
                         String (SCIM_PATH_DELIM_STRING) + type);
        paths.push_back (roots [i] + String (SCIM_PATH_DELIM_STRING) + type);
    }
}

String
scim_setup_find_module_path (const String &name)
{
    std::vector <String> paths;
    _setup_get_module_paths (paths, String (SETUP_MODULE_TYPE));

    for (size_t i = 0; i < paths.size (); ++i) {
        // The loader tries "<dir>/<name>.so" first, then the bare name.
        String candidate = paths [i] + String (SCIM_PATH_DELIM_STRING) + name + String (".so");
        struct stat st;
        if (stat (candidate.c_str (), &st) == 0 && S_ISREG (st.st_mode))
            return candidate;

        candidate = paths [i] + String (SCIM_PATH_DELIM_STRING) + name;
        if (stat (candidate.c_str (), &st) == 0 && S_ISREG (st.st_mode))
            return candidate;
    }

    return String ();
}

// Map a DT_NEEDED SONAME to a toolkit. Returns true if it identified a GTK
// major version, filling in @tk.
static bool
_soname_to_toolkit (const char *soname, SetupToolkit &tk)
{
    if (!soname) return false;

    if (strcmp (soname, "libgtk-4.so.1") == 0) {
        tk = SETUP_TOOLKIT_GTK4;
        return true;
    }
    if (strcmp (soname, "libgtk-3.so.0") == 0) {
        tk = SETUP_TOOLKIT_GTK3;
        return true;
    }
    // GTK2 links against libgtk-x11-2.0.so.0 (the only shipped GTK2 backend).
    if (strcmp (soname, "libgtk-x11-2.0.so.0") == 0) {
        tk = SETUP_TOOLKIT_GTK2;
        return true;
    }

    return false;
}

SetupToolkit
scim_setup_classify_module (const String &name)
{
    String path = scim_setup_find_module_path (name);
    if (path.length () == 0)
        return SETUP_TOOLKIT_UNKNOWN;

    if (elf_version (EV_CURRENT) == EV_NONE)
        return SETUP_TOOLKIT_UNKNOWN;

    int fd = open (path.c_str (), O_RDONLY);
    if (fd < 0)
        return SETUP_TOOLKIT_UNKNOWN;

    SetupToolkit result = SETUP_TOOLKIT_UNKNOWN;

    Elf *elf = elf_begin (fd, ELF_C_READ, NULL);
    if (elf && elf_kind (elf) == ELF_K_ELF) {
        Elf_Scn *scn = NULL;

        // Walk sections, find the dynamic section, and read its DT_NEEDED
        // entries. The needed SONAME strings live in the string table linked
        // by sh_link.
        while ((scn = elf_nextscn (elf, scn)) != NULL) {
            GElf_Shdr shdr;
            if (gelf_getshdr (scn, &shdr) != &shdr)
                continue;
            if (shdr.sh_type != SHT_DYNAMIC)
                continue;

            Elf_Data *data = elf_getdata (scn, NULL);
            if (!data)
                continue;

            size_t entsize = shdr.sh_entsize ? shdr.sh_entsize : 1;
            size_t count   = data->d_size / entsize;

            for (size_t i = 0; i < count; ++i) {
                GElf_Dyn dyn;
                if (gelf_getdyn (data, (int) i, &dyn) != &dyn)
                    continue;
                if (dyn.d_tag != DT_NEEDED)
                    continue;

                const char *soname = elf_strptr (elf, shdr.sh_link, dyn.d_un.d_val);
                SetupToolkit tk;
                if (_soname_to_toolkit (soname, tk)) {
                    result = tk;
                    break;      // first GTK dependency wins
                }
            }
            break;              // only one SHT_DYNAMIC section
        }
    }

    if (elf)
        elf_end (elf);
    close (fd);

    return result;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
