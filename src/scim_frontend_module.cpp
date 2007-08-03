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
 * $Id: scim_frontend_module.cpp,v 1.13 2005/01/10 08:30:54 suzhe Exp $
 *
 */

#define Uses_SCIM_FRONTEND_MODULE
#include "scim_private.h"
#include "scim.h"

namespace scim {

FrontEndModule::FrontEndModule ()
    : m_frontend_init (0),
      m_frontend_run (0)
{
}

FrontEndModule::FrontEndModule (const String &name,
                                const BackEndPointer &backend,
                                const ConfigPointer &config,
                                int argc,
                                char **argv)
    : m_frontend_init (0),
      m_frontend_run (0)
{
    load (name, backend, config, argc, argv);
}

bool
FrontEndModule::load (const String &name,
                        const BackEndPointer &backend,
                        const ConfigPointer &config,
                        int argc,
                        char **argv)
{
    try {
        if (!m_module.load (name, "FrontEnd"))
            return false;

        m_frontend_init = (FrontEndModuleInitFunc) m_module.symbol ("scim_frontend_module_init");
        m_frontend_run =  (FrontEndModuleRunFunc) m_module.symbol ("scim_frontend_module_run");

        if (!m_frontend_init || !m_frontend_run) {
            m_module.unload ();
            m_frontend_init = 0;
            m_frontend_run = 0;
            return false;
        }

        m_frontend_init (backend, config, argc, argv);
    } catch (...) {
        /* Don't unload FrontEnd module to avoid possible crash, when the
         * exception is thrown by the module itself.
         * m_module.unload ();
         */
        m_frontend_init = 0;
        m_frontend_run = 0;
        return false;
    }

    return true;
}

bool
FrontEndModule::valid () const
{
    return (m_module.valid () && m_frontend_init && m_frontend_run);
}

void
FrontEndModule::run () const
{
    if (valid ())
        m_frontend_run ();
}

int scim_get_frontend_module_list (std::vector <String>& mod_list)
{
    return scim_get_module_list (mod_list, "FrontEnd");
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
