/** @file scim_setup_helper.cpp
 * implementation of Setup Helper module.
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
 * $Id: scim_setup_helper.cpp,v 1.5 2005/01/13 14:54:18 suzhe Exp $
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_MODULE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HELPER
#define Uses_STL_MAP
#include "scim_private.h"
#include "scim.h"
#include "scim_setup_module.h"
#include "scim_setup_ui.h"

#define scim_module_init setup_LTX_scim_module_init
#define scim_module_exit setup_LTX_scim_module_exit
#define scim_helper_module_number_of_helpers setup_LTX_scim_helper_module_number_of_helpers
#define scim_helper_module_get_helper_info setup_LTX_scim_helper_module_get_helper_info
#define scim_helper_module_run_helper setup_LTX_scim_helper_module_run_helper

using namespace scim;

static HelperInfo __helper_info (String ("8034d025-bdfc-4a10-86a4-82b9461b32b0"),
                                 String (_("SCIM Setup")),
                                 String (SCIM_ICONDIR "/setup.png"),
                                 String (_("Integrated Setup Utility based on GTK Widget library.")),
                                 SCIM_HELPER_STAND_ALONE);
                                 

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
    }

    void scim_module_exit (void)
    {
    }

    unsigned int scim_helper_module_number_of_helpers (void)
    {
        return 1;
    }

    bool scim_helper_module_get_helper_info (unsigned int idx, HelperInfo &info)
    {
        if (idx == 0) {
            info = __helper_info; 
            return true;
        }
        return false;
    }

    void scim_helper_module_run_helper (const String &uuid, const ConfigPointer &config, const String &display)
    {
        SCIM_DEBUG_MAIN(1) << "setup_LTX_scim_helper_module_run_helper ()\n";

        if (uuid == "8034d025-bdfc-4a10-86a4-82b9461b32b0") {
            SetupUI * setup_ui = new SetupUI (config, display, __helper_info);

            std::vector<String>  setup_list;

            SetupModule         *setup_module = 0;

            scim_get_setup_module_list (setup_list);

            for (size_t i = 0; i < setup_list.size (); ++ i) {
                setup_module = new SetupModule (setup_list [i]);

                if (setup_module && setup_module->valid ()) {
                    setup_ui->add_module (setup_module);
                } else if (setup_module) {
                    delete setup_module;
                }
            }

            setup_ui->run ();
            delete setup_ui;
        }

        SCIM_DEBUG_MAIN(1) << "exit setup_LTX_scim_helper_module_run_helper ()\n";
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/

