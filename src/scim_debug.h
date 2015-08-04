/**
 * @file scim_debug.h
 * @brief Defines class scim::DebugOutput and related MACROS.
 *
 * All of the debug information should be output via scim::DebugOutput class.
 * This class provides message filter and redirection ability.
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
 * $Id: scim_debug.h,v 1.18 2005/08/05 01:54:24 suzhe Exp $
 */

#ifndef __SCIM_DEBUG_H
#define __SCIM_DEBUG_H

#define SCIM_DEBUG_MAX_VERBOSE      7

namespace scim {

/**
 * @name The mask for debug messages filtering.
 * @{
 */
#define SCIM_DEBUG_AllMask          ((uint32)~0) /**< Show all messages. */
#define SCIM_DEBUG_MainMask         1    /**< Show messages of main application. */
#define SCIM_DEBUG_ConfigMask       2    /**< Show messages of Config objects */
#define SCIM_DEBUG_IMEngineMask     4    /**< Show messages of IMEngine objects */
#define SCIM_DEBUG_BackEndMask      8    /**< Show messages of BackEnd objects */
#define SCIM_DEBUG_FrontEndMask     16   /**< Show messages of FrontEnd objects */
#define SCIM_DEBUG_ModuleMask       32   /**< Show messages of Module objects */
#define SCIM_DEBUG_UtilityMask      64   /**< Show messages of utility functions */
#define SCIM_DEBUG_IConvMask        128  /**< Show messages of IConvert objects */
#define SCIM_DEBUG_LookupTableMask  256  /**< Show messages of LookupTable objects */
#define SCIM_DEBUG_SocketMask       512  /**< Show messages of Socket objects */
/**
 * @}
 */

/**
 * @name The macros to simplify the debug message print method.
 *
 * You can output debug messages by this way:
 *   SCIM_DEBUG_IMENGINE(1) << "Hello World!\n";
 *
 * @{
 */
#define SCIM_DEBUG(mask,level)        (scim::DebugOutput(mask,level) << scim::DebugOutput::serial_number () << __FILE__ << ":" << __LINE__ << " > ")
#define SCIM_DEBUG_MAIN(level)        SCIM_DEBUG(SCIM_DEBUG_MainMask,level)
#define SCIM_DEBUG_CONFIG(level)      SCIM_DEBUG(SCIM_DEBUG_ConfigMask,level)
#define SCIM_DEBUG_IMENGINE(level)    SCIM_DEBUG(SCIM_DEBUG_IMEngineMask,level)
#define SCIM_DEBUG_BACKEND(level)     SCIM_DEBUG(SCIM_DEBUG_BackEndMask,level)
#define SCIM_DEBUG_FRONTEND(level)    SCIM_DEBUG(SCIM_DEBUG_FrontEndMask,level)
#define SCIM_DEBUG_MODULE(level)      SCIM_DEBUG(SCIM_DEBUG_ModuleMask,level)
#define SCIM_DEBUG_UTILITY(level)     SCIM_DEBUG(SCIM_DEBUG_UtilityMask,level)
#define SCIM_DEBUG_ICONV(level)       SCIM_DEBUG(SCIM_DEBUG_IConvMask,level)
#define SCIM_DEBUG_LOOKUPTABLE(level) SCIM_DEBUG(SCIM_DEBUG_LookupTableMask,level)
#define SCIM_DEBUG_SOCKET(level)      SCIM_DEBUG(SCIM_DEBUG_SocketMask,level)
/**
 * @}
 */

/**
 * @brief The class to filter and redirect the debug messages.
 */
class DebugOutput
{
private:
    static uint32          current_verbose;
    static uint32          current_mask;

    static uint32          verbose_level;
    static uint32          output_mask;
    static std::ostream   *output_stream;

public:
    /**
     * @brief Constructor.
     * @param mask - the debug filter mask.
     * @param verbose - the verbose level of the debug message.
     */
    DebugOutput (uint32 mask = SCIM_DEBUG_AllMask, uint32 verbose = 1);

    /**
     * @brief A template stream output operator.
     *
     * All kinds of data and variables can be output via DebugOutput by
     * this operator.
     */
#if ENABLE_DEBUG
    template <typename T>
    const DebugOutput& operator << (const T& t) const {
        if (output_stream && (current_mask & output_mask) && (current_verbose <= verbose_level))
            (*output_stream) << t;
        return *this;
    }
#else
    template <typename T>
    const DebugOutput& operator << (const T&) const {
        return *this;
    }
#endif

public:
    /** 
     * @brief The global method to enable the debug output.
     * @param debug - the mask to indicate which kind of
     *                debug should be enabled.
     */
    static void enable_debug (uint32 debug);

    /**
     * @brief The global method to enable the debug output by their names.
     * @param debug - the name of the debug type to be enabled. The valid
     *                names are: all, main, config, imengine, backend, frontend,
     *                module, utility, iconv, lookuptable, socket.
     */
    static void enable_debug_by_name (const String &debug);

    /**
     * @brief Disable the debug type indicated by the given mask.
     * @param debug - the mask of the debug type to be disabled.
     */
    static void disable_debug (uint32 debug);

    /**
     * @brief Disable the debug type indicated by the given name.
     * @param debug - the name of the debug type to be disabled.
     */
    static void disable_debug_by_name (const String &debug);

    /**
     * @brief Set the debug verbose level.
     * @param verbose - the debug verbose level, 0 means no debug output.
     */
    static void set_verbose_level (uint32 verbose);

    /**
     * @brief Set the debug output file.
     *
     * @param file - the file to store the debug output.
     *               If equal to "stderr" or "cerr" then the debug
     *               output will be set to std:cerr.
     *               If equal to "stdout" or "cout" then the debug
     *               output will be set to std::cout.
     */
    static void set_output (const String &file);

    static String serial_number ();
};

} // namespace scim

#endif //__SCIM_DEBUG_H
/*
vi:ts=4:nowrap:ai:expandtab
*/
