/** @file scim_global_config.h
 *  @brief functions to read the global configurations.
 *
 *  The global configuration file (normally /etc/scim/global) is used to store
 *  the configurations for libscim itself and the system wide configurations which
 *  will be read before any Config module is loaded.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004-2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_global_config.h,v 1.4 2005/01/10 08:30:54 suzhe Exp $
 */

#ifndef __SCIM_GLOBAL_CONFIG_H
#define __SCIM_GLOBAL_CONFIG_H

namespace scim {
/**
 * @addtogroup Accessories
 * @{
 */

/**
 * @brief Read a string value from the global configuration file.
 *
 * @param key The key to be read, like /PanelProgram etc.
 * @param defVal The default value to be returned if there is no such key in the configuration file.
 *
 * @return the value string of the key.
 */
String scim_global_config_read (const String &key, const String &defVal = String ());

/**
 * @brief Read an int value from the global configuration file.
 *
 * @param key The key to be read, like /SocketTimeout etc.
 * @param defVal The default value to be returned if there is no such key in the configuration file.
 *
 * @return the value of the key.
 */
int    scim_global_config_read (const String &key, int defVal);

/**
 * @brief Read a bool value from the global configuration file.
 *
 * @param key The key to be read.
 * @param defVal The default value to be returned if there is no such key in the configuration file.
 *
 * @return the value of the key.
 */
bool   scim_global_config_read (const String &key, bool defVal);

/**
 * @brief Read a double value from the global configuration file.
 *
 * @param key The key to be read.
 * @param defVal The default value to be returned if there is no such key in the configuration file.
 *
 * @return the value of the key.
 */
double scim_global_config_read (const String &key, double defVal);

/**
 * @brief Read a string list from the global configuration file.
 *
 * @param key The key to be read.
 * @param defVal The default value to be returned if there is no such key in the configuration file.
 *
 * @return the value of the key.
 */
std::vector <String> scim_global_config_read (const String &key, const std::vector <String> &defVal);

/**
 * @brief Read an int list from the global configuration file.
 *
 * @param key The key to be read.
 * @param defVal The default value to be returned if there is no such key in the configuration file.
 *
 * @return the value of the key.
 */
std::vector <int>    scim_global_config_read (const String &key, const std::vector <int> &defVal);

/**
 * @brief Write a string value into the user global config.
 *
 * @param key The key to be associated.
 * @param val The string value to be written.
 */
void scim_global_config_write (const String &key, const String &val);

/**
 * @brief Write an int value into the user global config.
 *
 * @param key The key to be associated.
 * @param val The int value to be written.
 */
void scim_global_config_write (const String &key, int val);

/**
 * @brief Write a bool value into the user global config.
 *
 * @param key The key to be associated.
 * @param val The bool value to be written.
 */
void scim_global_config_write (const String &key, bool val);

/**
 * @brief Write a double value into the user global config.
 *
 * @param key The key to be associated.
 * @param val The double value to be written.
 */
void scim_global_config_write (const String &key, double val);

/**
 * @brief Write a string list into the user global config.
 *
 * @param key The key to be associated.
 * @param val The string list to be written.
 */
void scim_global_config_write (const String &key, const std::vector <String> &val);

/**
 * @brief Write an int list into the user global config.
 *
 * @param key The key to be associated.
 * @param val The int list to be written.
 */
void scim_global_config_write (const String &key, const std::vector <int> &val);

/**
 * @brief Reset the value associated to the specified key to its default value.
 *
 * @param key The key to be reset.
 */
void scim_global_config_reset (const String &key);

/**
 * @brief Flush the updated global config into user global config file.
 * @return true if success.
 */
bool scim_global_config_flush ();

/** @} */
} // namespace scim

#endif //__SCIM_GLOBAL_CONFIG_H
/*
vi:ts=4:nowrap:ai:expandtab
*/

