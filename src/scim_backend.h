/** @file scim_backend.h
 *  @brief definition of scim::BackEnd class.
 *
 *  Class scim::BackEnd is used to load and manage IMEngine
 *  modules and IMEngineFactories.
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
 * $Id: scim_backend.h,v 1.26 2005/10/06 18:02:06 liuspider Exp $
 */

#ifndef __SCIM_BACKEND_H
#define __SCIM_BACKEND_H

namespace scim {

/**
 * @brief An exception class to hold BackEnd related errors.
 *
 * scim::BackEndBase and its derived classes must throw
 * scim::BackEndError object when error.
 */
class BackEndError: public Exception
{
public:
    BackEndError (const String& what_arg)
        : Exception (String("scim::BackEnd: ") + what_arg) { }
};

/**
 * @brief The interface class to manage a set of IMEngineFactory
 * and IMEngineInstance objects.
 *
 * This is mainly an accessory interface class used by scim::FrontEndBase.
 * Its responsibility is to hold a set of IMEngineFactory instances
 * and manage the locales list supported by them. 
 * 
 * Most developer should just use the default implementation
 * scim::CommonBackEnd.
 */
class BackEndBase : public ReferencedObject
{
    class BackEndBaseImpl;

    BackEndBaseImpl *m_impl;

protected:
    /**
     * @brief Default constructor.
     *
     * @param config  Config object to be used.
     */
    BackEndBase (const ConfigPointer &config);

    virtual ~BackEndBase ();

public:
    /**
     * @brief Get a list of all locales supported by all IMEngineFactories.
     * @return A comma separated locales list.
     */
    String get_all_locales () const;

    /**
     * @return Return the pointer of a Factory.
     *
     * @param uuid The uuid of the IMEngineFactory.
     */
    IMEngineFactoryPointer get_factory (const String &uuid) const;

public:
    /**
     * @name Methods to manipulate IMEngine Factories.
     *
     * @{
     */

    /**
     * @brief Get the IMEngine factories list for specific encoding
     *
     * @param factories the vector to store the factories which
     *                  support the encoding.
     * @param encoding  the encoding to be queried. If empty,
     *                  all IMEngine factories will be returned.
     *
     * @return the number of IMEngine factories found.
     */
    uint32 get_factories_for_encoding (std::vector<IMEngineFactoryPointer> &factories, const String &encoding = String ("")) const;

    /**
     * @brief Get the IMEngine factories list for specific language
     *
     * @param factories the vector to store the factories which
     *                  support the encoding.
     * @param language  the language to be queried. If empty,
     *                  all IMEngine factories will be returned.
     *
     * @return the number of IMEngine factories found.
     */
    uint32 get_factories_for_language (std::vector<IMEngineFactoryPointer> &factories, const String &language = String ("")) const;

    /**
     * @brief Get the default IMEngineFactory for a specific language and encoding.
     *
     * @param language the language to be queried.
     * @param encoding the encoding to be queried, if empty then don't match encoding.
     *
     * @return the pointer of the default IMEngineFactory for this language.
     */
    IMEngineFactoryPointer get_default_factory (const String &language, const String &encoding) const;

    /**
     * @brief Set the default IMEngineFactory for a specific language.
     *
     * @param language the language to be set.
     * @param uuid the uuid of the default IMEngineFactory for this language.
     */
    void set_default_factory (const String &language, const String &uuid);

    /**
     * @brief Get the next IMEngineFactory for a specific language and encoding.
     *
     * @param language the language to be queried, if empty then don't match language.
     * @param encoding the encoding to be queried, if empty then don't match encoding.
     * @param cur_uuid the UUID of current IMEngineFactory.
     *
     * @return the pointer of the next IMEngineFactory for this language and encoding
     *         corresponding to the current IMEngineFactory.
     */
    IMEngineFactoryPointer get_next_factory (const String &language, const String &encoding, const String &cur_uuid) const;

    /**
     * @brief Get the previous IMEngineFactory for a specific language and encoding.
     *
     * @param language the language to be queried, if empty then don't match language.
     * @param encoding the encoding to be queried, if empty then don't match encoding.
     * @param cur_uuid the UUID of current IMEngineFactory.
     *
     * @return the pointer of the previous IMEngineFactory for this language and encoding
     *         corresponding to the current IMEngineFactory.
     */
    IMEngineFactoryPointer get_previous_factory (const String &language, const String &encoding, const String &cur_uuid) const;

    /**
     * @}
     */

protected:

    bool add_factory (const IMEngineFactoryPointer &factory);

    void clear ();
};

/**
 * @typedef typedef Pointer <BackEnd> BackEndPointer;
 *
 * A smart pointer for scim::BackEndBase and its derived classes.
 */
typedef Pointer <BackEndBase> BackEndPointer;

/**
 * @brief The default implementation of scim::BackEndBase interface.
 */
class CommonBackEnd : public BackEndBase
{
    class CommonBackEndImpl;
    CommonBackEndImpl *m_impl;

public:
    /**
     * @brief Constructor
     *
     * @param config The pointer to the Config object.
     * @param modules The list of the IMEngine modules to be loaded.
     */
    CommonBackEnd (const ConfigPointer       &config,
                   const std::vector<String> &modules);

    virtual ~CommonBackEnd ();
};

} // namespace scim

#endif //__SCIM_BACKEND_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
