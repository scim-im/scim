/**
 * @file scim_filter.h
 * @brief Defines scim::FilterFactoryBase and scim::FilterInstanceBase interfaces.
 *
 * scim::FilterFactoryBase and scim::FilterInstanceBase are used to implement
 * filter IMEngines, such as Simplified Chinese <-> Traditional Chinese converter etc.
 *
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
 * $Id: scim_filter.h,v 1.5 2005/05/28 13:54:59 suzhe Exp $
 */

#ifndef __SCIM_FILTER_H
#define __SCIM_FILTER_H

namespace scim {
/**
 * @addtogroup IMEngine
 * The base classes for filter input method engine modules.
 * @{
 */

/**
 * @brief An exception class to hold Filter related errors.
 *
 * scim::FilterFactoryBase, scim::FilterInstanceBase
 * and their derived classes must throw scim::FilterError object when error.
 */
class FilterError: public Exception
{
public:
    FilterError (const String& what_arg)
        : Exception (String("scim::Filter: ") + what_arg) { }
};

/**
 * @brief Structure to hold information for a Filter.
 */
struct FilterInfo
{
    String uuid;    ///< The UUID.
    String name;    ///< The localized name, in UTF-8 encoding.
    String langs;   ///< The supported languages, separated by comma.
    String icon;    ///< The icon file path.
    String desc;    ///< The description.

    FilterInfo () { }
    FilterInfo (const String &u, const String &n = String (""), const String &l = String (""), const String &i = String (""), const String &d = String (""))
        : uuid (u), name (n), langs (l), icon (i), desc (d) { }
};

/**
 * @brief The base class to implement FilterFactory classes.
 *
 * All FilterFactory classes should derive from this class and some base methods
 * should be overrided in the derived classes.
 *
 * If a FilterFactory object need large amount of data (eg. mapping table etc.),
 * it'd better to share these data among all other objects of the same class.
 * Because multiple objects of one FilterFactory class maybe used at the same time 
 * to filter several real IMEngineFactory objects.
 */
class FilterFactoryBase : public IMEngineFactoryBase
{
    IMEngineFactoryPointer m_orig;

public:
    /**
     * @brief Default Constructor.
     */
    FilterFactoryBase ();

    virtual ~FilterFactoryBase ();

    /**
     * @brief Attach an IMEngineFactory object to this filter.
     *
     * The attached object could also be a filter.
     *
     * This method will set the supported locales of this filter factory
     * to the locales supported by the original IMEngineFactory object.
     *
     * This method could be overrided in derived class, in which some special
     * tasks could be done, for example, set additional supported locales.
     * But this method of base class must be invoked to attach the factory correctly.
     *
     * The IMEngineFactoryPointer orig may not be kept and used by derived class directly.
     * All tasks related to the original IMEngineFactory object should be done by
     * calling the corresponding methods of FilterFactoryBase class.
     *
     * @param orig The original IMEngineFactory object to be filtered.
     */
    virtual void attach_imengine_factory (const IMEngineFactoryPointer &orig);

    /**
     * @brief Return the name of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class should override this method to return
     * the name of the filter itself when it returns an empty string.
     *
     * The derived method should look like:
     *
     * <PRE>
     * WideString
     * XXXFilterFactory::get_name ()
     * {
     *     WideString name = FilterFactoryBase::get_name ();
     *     return name.length () ? name : _("XXX");
     * }
     * </PRE>
     */
    virtual WideString  get_name () const;

    /**
     * @brief Return the uuid of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class should override this method to return
     * the uuid of the filter itself when it returns an empty string.
     */
    virtual String      get_uuid () const;

    /**
     * @brief Return the icon file path of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class should override this method to return
     * the icon file path of the filter itself when it returns an empty string.
     */
    virtual String      get_icon_file () const;

    /**
     * @brief Return the authors information of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class should override this method to return
     * the authors information of the filter itself when it returns an empty string.
     */
    virtual WideString  get_authors () const;

    /**
     * @brief Return the credits information of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class should override this method to return
     * the credits information of the filter itself when it returns an empty string.
     */
    virtual WideString  get_credits () const;

    /**
     * @brief Return the help information of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class should override this method to return the combined
     * help information of the filter itself and the original IMEngineFactory to be filtered.
     */
    virtual WideString  get_help () const;

    /**
     * @brief Return the supported language of the original IMEngineFactory object
     * specified in contructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet,
     * then an empty string would be returned.
     *
     * The derived class may override this method to return
     * a different language which is supported by the filter itself.
     */
    virtual String      get_language () const;

    /**
     * @brief Check if an encoding is supported by the original IMEngineFactory object
     * specified in constructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet, then false would be returned.
     *
     * The derived class may override this method to provide its own validate routing.
     */
    virtual bool        validate_encoding (const String& encoding) const; 

    /**
     * @brief Check if an locale is supported by the original IMEngineFactory object
     * specified in constructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet, then false would be returned.
     *
     * The derived class may override this method to provide its own validate routing.
     */
    virtual bool        validate_locale (const String& locale) const;

    /**
     * @brief Get the original key string of a composed string by calling the same method of
     * the original IMEngineFactory object specified in constructor or by attach_imengine_factory() method.
     *
     * If there is no IMEngineFactory attached yet, then an empty string would be returned.
     */
    virtual WideString  inverse_query (const WideString &str);

    /**
     * @brief Create an IMEngineInstance object of the original IMEngineFactory object
     * specified in constructor or by attach_imengine_factory() method.
     * 
     * The derived class should override this method and create its own instance object
     * from the result of this base method.
     *
     * The code may look like:
     * 
     * <PRE>
     * IMEngineInstancePointer
     * XXXFilterFactory::create_instance (const String& encoding, int id)
     * {
     *     return new XXXFilterInstance (this, FilterFactoryBase::create_instance (encoding, id));
     * }
     * </PRE>
     */
    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

/**
 * @typedef typedef Pointer <FilterFactoryBase> FilterFactoryPointer;
 *
 * A smart pointer for scim::FilterFactoryBase and its derived classes.
 */
typedef Pointer <FilterFactoryBase> FilterFactoryPointer;

/**
 * @brief The base class to implement FilterInstance classes.
 *
 * All FilterInstance classes should derive from this base class,
 * and some base methods should be override in the derived classes.
 */
class FilterInstanceBase : public IMEngineInstanceBase
{
    class FilterInstanceBaseImpl;

    FilterInstanceBaseImpl *m_impl;

    friend class FilterInstanceBaseImpl;
public:
    /**
     * @brief Constructor.
     *
     * The Constructor of derived class should call this Contructor with the following
     * two parameters.
     *
     * The orig_inst should be created by invoking FilterFactoryBase::create_instance()
     * within the same method of derived FilterFactory classes.
     *
     * @param factory the FilterFactory which creates this instance.
     * @param orig_inst the original IMEngineInstance to be filtered.
     */
    FilterInstanceBase (FilterFactoryBase     *factory,
                        const IMEngineInstancePointer &orig_inst);

    virtual ~FilterInstanceBase ();

    /**
     * @brief Set the working encoding for this filter instance
     *        as well as the original instance which is currently filtered.
     *
     * This method could be overrided in derived class to do some extra job. But
     * the method of this base class must be invoked within the new method.
     *
     * After invoking this method, reset() should be invoked to
     * let the new encoding take effect.
     *
     * If you want to use different encodings in this filter instance and
     * the original instance which is currently filtered, this method should
     * be overrided in the derived class and two different encodings should be
     * set respectively by calling FilterInstanceBase::set_encoding() and
     * IMEngineInstanceBase::set_encoding (); The code may look like:
     *
     * bool
     * XXXFilterInstance::set_encoding (const String &encoding)
     * {
     *     FilterInstanceBase::set_encoding ("Other Encoding");  // Set the encoding of the original instance to another one.
     *     return IMEngineInstanceBase::set_encoding (encoding);         // Set the encoding of this filter instance to the desired one.
     * }
     *
     * @return true if the encoding is supported, otherwise false.
     */
    virtual bool set_encoding (const String &encoding);
public:
    /**
     * @name Action functions.
     *
     * These functions will be called by FrontEnds to send events to
     * this FilterInstance.
     *
     * These methods can be overrided, if the derived class wants to filter these events.
     *
     * The default implementation of these methods are just to call the corresponding methods
     * of the original IMEngineInstance object which is filtered by this filter.
     *
     * @{
     */
    virtual bool process_key_event (const KeyEvent &key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int index);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    virtual void trigger_property (const String &property);
    virtual void process_helper_event (const String &helper_uuid, const Transaction &trans);
    virtual void update_client_capabilities (unsigned int cap);
    /** @ */

protected:
    /**
     * @name Signal activation functions.
     *
     * These methods will be called by FilterInstanceBase class when the corresponding signals are
     * emitted by the original IMEngineInstance object which is filtered by this filter. 
     *
     * These methods can be overrided, if the derived class wants to filter these signals.
     * The default implementation of these methods are just to deliver the signals to FrontEnd directly.
     *
     * @{
     */
    virtual void filter_show_preedit_string ();
    virtual void filter_show_aux_string ();
    virtual void filter_show_lookup_table ();
    virtual void filter_hide_preedit_string ();
    virtual void filter_hide_aux_string ();
    virtual void filter_hide_lookup_table ();
    virtual void filter_update_preedit_caret (int caret);
    virtual void filter_update_preedit_string (const WideString    &str,
                                               const AttributeList &attrs = AttributeList ());
    virtual void filter_update_aux_string (const WideString    &str,
                                           const AttributeList &attrs = AttributeList ());
    virtual void filter_update_lookup_table (const LookupTable &table);
    virtual void filter_commit_string (const WideString &str);
    virtual void filter_forward_key_event (const KeyEvent &key);
    virtual void filter_register_properties (const PropertyList &properties);
    virtual void filter_update_property (const Property &property);
    virtual void filter_beep ();
    virtual void filter_start_helper (const String &helper_uuid);
    virtual void filter_stop_helper (const String &helper_uuid);
    virtual void filter_send_helper_event (const String &helper_uuid, const Transaction &trans);
    virtual bool filter_get_surrounding_text (WideString &text, int &cursor, int maxlen_before = -1, int maxlen_after = -1);
    virtual bool filter_delete_surrounding_text (int offset, int len);
    /** @} */
};

/**  @} */

} // namespace scim

#endif //__SCIM_FILTER_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
