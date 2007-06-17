/** @file scim_config_base.h
 *  @brief scim::ConfigBase Interface.
 *
 *  Provide a unified interface to access the configuration data.
 *  All of SCIM objects should use this interface if they have any
 *  configuration data.
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
 * $Id: scim_config_base.h,v 1.22 2005/04/09 15:38:39 suzhe Exp $
 */

#ifndef __SCIM_CONFIG_BASE_H
#define __SCIM_CONFIG_BASE_H

namespace scim {
/**
 * @addtogroup Config
 * The base classes for config modules
 * @{
 */

/**
 * @brief An exception class to hold Config related errors.
 *
 * scim::ConfigBase and its derived classes must throw
 * scim::ConfigError object when error.
 */
class ConfigError: public Exception
{
public:
    ConfigError (const String& what_arg)
        : Exception (String("scim::Config: ") + what_arg) { }
};

class ConfigBase;
/**
 * @typedef typedef Pointer <ConfigBase> ConfigPointer;
 *
 * A smart pointer for scim::ConfigBase and its derived classes.
 */
typedef Pointer <ConfigBase> ConfigPointer;

/**
 * @typedef typedef Slot1<void, const ConfigPointer &> ConfigSlotVoid;
 *
 * The slot type to connect to the coresponding signal.
 */
typedef Slot1<void, const ConfigPointer &> ConfigSlotVoid;

/**
 * @typedef typedef Signal1<void, const ConfigPointer &> ConfigSignalVoid;
 *
 * The signal type to connect with the ConfigSlotVoid slot type.
 */
typedef Signal1<void, const ConfigPointer &> ConfigSignalVoid;

/**
 * @brief The interface class to access the configuration data.
 *
 * This is an interface class to access the configuration data.
 * All of the SCIM objects which have configuration data should 
 * use this interface to store and load them.
 */
class ConfigBase : public ReferencedObject
{
    ConfigSignalVoid m_signal_reload;

public:
    /**
     * @name Constructor and Destructor.
     * @{
     */

    /**
     * @brief Contrustor
     */
    ConfigBase ();

    /**
     * @brief Virtual destructor 
     * empty but ensures that dtor of all derived classes is virtual
     */
    virtual ~ConfigBase ();

    /**
     * @}
     */

    /**
     * @name Pure virtual methods which should be implemented in derived classes.
     * @{
     */

    /**
     * @brief Check if this Config object is valid.
     * @return true if its valid and ready to work.
     */
    virtual bool valid () const = 0;

    /**
     * @brief Return the name of this configuration module.
     *
     * This name must be same as the config module's name.
     */
    virtual String get_name () const = 0;

    /**
     * @brief Read a string from the given key.
     * @param key - the key to be read.
     * @param ret - the result will be stored into *ret.
     * @return true if the string is read successfully, otherwise return false.
     */
    virtual bool read (const String& key, String *ret) const = 0;

    /**
     * @brief Read an int value from the given key.
     * @param key - the key to be read.
     * @param ret - the result will be stored into *ret.
     * @return true if the value is read successfully, otherwise return false.
     */
    virtual bool read (const String& key, int *ret) const = 0;

    /**
     * @brief Read a double value from the given key.
     * @param key - the key to be read.
     * @param ret - the result will be stored into *ret.
     * @return true if the value is read successfully, otherwise return false.
     */
    virtual bool read (const String& key, double *ret) const = 0;

    /**
     * @brief Read a bool value from the given key.
     * @param key - the key to be read.
     * @param ret - the result will be stored into *ret.
     * @return true if the value is read successfully, otherwise return false.
     */
    virtual bool read (const String& key, bool *ret) const = 0;

    /**
     * @brief Read a string list from the given key.
     * @param key - the key to be read.
     * @param ret - the result will be stored into *ret.
     * @return true if the string list is read successfully, otherwise return false.
     */
    virtual bool read (const String& key, std::vector <String> *ret) const = 0;

    /**
     * @brief Read an int list from the given key.
     * @param key - the key to be read.
     * @param ret - the result will be stored into *ret.
     * @return true if the int list is read successfully, otherwise return false.
     */
    virtual bool read (const String& key, std::vector <int> *ret) const = 0;

    /**
     * @brief Write a string to the given key.
     * @param key   - the key to be written.
     * @param value - the string to be written to the key.
     * @return true if success, otherwise false.
     */
    virtual bool write (const String& key, const String& value) = 0;

    /**
     * @brief Write an int value to the given key.
     * @param key   - the key to be written.
     * @param value - the int value to be written to the key.
     * @return true if success, otherwise false.
     */
    virtual bool write (const String& key, int value) = 0;

    /**
     * @brief Write a double value to the given key.
     * @param key   - the key to be written.
     * @param value - the double value to be written to the key.
     * @return true if success, otherwise false.
     */
    virtual bool write (const String& key, double value) = 0;

    /**
     * @brief Write a bool value to the given key.
     * @param key   - the key to be written.
     * @param value - the bool value to be written to the key.
     * @return true if success, otherwise false.
     */
    virtual bool write (const String& key, bool value) = 0;

    /**
     * @brief Write a string list to the given key.
     * @param key   - the key to be written.
     * @param value - the string list to be written to the key.
     * @return true if success, otherwise false.
     */
    virtual bool write (const String& key, const std::vector <String>& value) = 0;

    /**
     * @brief Write an int list to the given key.
     * @param key   - the key to be written.
     * @param value - the int list to be written to the key.
     * @return true if success, otherwise false.
     */
    virtual bool write (const String& key, const std::vector <int>& value) = 0;

    /**
     * @brief Permanently writes all changes.
     * @return true if success.
     */
    virtual bool flush() = 0;

    /**
     * @brief Erase a key and its value
     * @param key - the key to be erased.
     * @return true if success.
     */
    virtual bool erase (const String& key) = 0;

    /**
     * @brief Reload the configurations from storage.
     *
     * All modified keys after the last flush maybe lost.
     *
     * The derived method should call this base method
     * after reload the configurations successfully,
     * in order to emit the reload signal.
     * 
     * The derived method should have some machanism to avoid
     * reload again if there is no update after
     * the previous reload.
     *
     * @return true if success.
     */
    virtual bool reload () = 0;

    /**
     * @}
     */

    /**
     * @name Other helper methods.
     * @{
     */

    /**
     * @brief Read a string from the given key with a default fallback value.
     *
     * If failed to read from the given key, then return the given default value.
     *
     * @param key    - the key to be read.
     * @param defVal - the default value to be return if failed to read.
     * @return The result string.
     */
    String read (const String& key, const String& defVal = String ()) const;

    /**
     * @brief Read an int value from the given key with a default fallback value.
     *
     * If failed to read from the given key, then return the given default value.
     *
     * @param key    - the key to be read.
     * @param defVal - the default value to be return if failed to read.
     * @return The result int value.
     */
    int read (const String& key, int defVal) const;

    /**
     * @brief Read a double value from the given key with a default fallback value.
     *
     * If failed to read from the given key, then return the given default value.
     *
     * @param key    - the key to be read.
     * @param defVal - the default value to be return if failed to read.
     * @return The result double value.
     */
    double read (const String& key, double defVal) const;

    /**
     * @brief Read a bool value from the given key with a default fallback value.
     *
     * If failed to read from the given key, then return the given default value.
     *
     * @param key    - the key to be read.
     * @param defVal - the default value to be return if failed to read.
     * @return The result bool value.
     */
    bool read (const String& key, bool defVal) const;

    /**
     * @brief Read a string list from the given key with a default fallback value.
     *
     * If failed to read from the given key, then return the given default value.
     *
     * @param key    - the key to be read.
     * @param defVal - the default value to be return if failed to read.
     * @return The result string list.
     */
    std::vector <String> read (const String& key, const std::vector <String>& defVal) const;

    /**
     * @brief Read an int list from the given key with a default fallback value.
     *
     * If failed to read from the given key, then return the given default value.
     *
     * @param key    - the key to be read.
     * @param defVal - the default value to be return if failed to read.
     * @return The result int list.
     */
    std::vector <int> read (const String& key, const std::vector <int>& defVal) const;

    /**
     * @brief connect the given slot to the reload signal.
     * @param slot - the given slot to be connected.
     * @return the Connection object, can be used to disconnect this slot.
     */
    Connection signal_connect_reload (ConfigSlotVoid *slot);

    /** @} */ 

public:
    /**
     * @brief Set the default global Config object.
     * 
     * There is only one global Config object in an application.
     * All other objects should use it by default.
     *
     * @param p_config - a smart pointer to the Config object.
     * @return a smart pointer to the old default Config object.
     */
    static ConfigPointer set (const ConfigPointer &p_config);

    /**
     * @brief Get the default global Config object.
     * 
     * The default global Config object can be set with function ConfigBase::set.
     * If there is no default object set, a new object can be created if needed.
     *
     * @param create_on_demand - if it's true then a new object will be created,
     *                           if there is no one available.
     * @param default_module   - the Config module should be used to create the 
     *                           default Config object. If omitted, then use the
     *                           default module defined in global config file.
     * @return a smart pointer to the default global Config object.
     */
    static ConfigPointer get (bool create_on_demand = true,
                              const String &default_module = String (""));
};

/**
 * @brief A dummy implementation of interface class scim::ConfigBase.
 *
 * The read methods will just return false and the default value (if available).
 * The write methods will do nothing.
 */
class DummyConfig : public ConfigBase
{

public:
    DummyConfig ();

    virtual ~DummyConfig ();

    virtual bool valid () const;
    virtual String get_name () const;

    virtual bool read (const String& key, String *ret) const;
    virtual bool read (const String& key, int *ret) const;
    virtual bool read (const String& key, double *ret) const;
    virtual bool read (const String& key, bool *ret) const;
    virtual bool read (const String& key, std::vector <String> *ret) const;
    virtual bool read (const String& key, std::vector <int> *ret) const;

    virtual bool write (const String& key, const String& value);
    virtual bool write (const String& key, int value);
    virtual bool write (const String& key, double value);
    virtual bool write (const String& key, bool value);
    virtual bool write (const String& key, const std::vector <String>& value);
    virtual bool write (const String& key, const std::vector <int>& value);

    virtual bool flush();

    virtual bool erase (const String& key );

    virtual bool reload ();
};

/** @} */

} // namespace scim

#endif //__SCIM_CONFIG_BASE_H
/*
vi:ts=4:nowrap:ai:expandtab
*/
