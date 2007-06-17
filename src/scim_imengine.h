/**
 * @file scim_imengine.h
 * @brief Defines scim::IMEngineFactoryBase and scim::IMEngineInstanceBase interfaces.
 *
 * scim::IMEngineFactoryBase and scim::IMEngineInstanceBase are the most important
 * part of SCIM platform.
 *
 * These interfaces are used to write input method engine modules.
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
 * $Id: scim_imengine.h,v 1.19 2005/08/15 12:45:46 suzhe Exp $
 */

#ifndef __SCIM_IMENGINE_H
#define __SCIM_IMENGINE_H

namespace scim {
/**
 * @addtogroup IMEngine
 * The base classes for input method engine modules.
 * @{
 */

/**
 * @brief Enum values of all Client Capabilities bitmask.
 *
 * These capabilities are not always supported by all kinds of clients.
 * So if an IMEngine requires some of them to realize some features,
 * it should make sure that they are supported by client by checking the cap value
 * sent by update_client_capabilities() action.
 */
enum ClientCapability
{
    SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT     = (1 << 0),   /**< The client support OnTheSpot preedit (embed preedit string into client window) */
    SCIM_CLIENT_CAP_SINGLE_LEVEL_PROPERTY = (1 << 1),   /**< The client support displaying single level property, property tree may not be supported*/ 
    SCIM_CLIENT_CAP_MULTI_LEVEL_PROPERTY  = (1 << 2),   /**< The client support displaying multiple level property, aka. property tree */
    SCIM_CLIENT_CAP_TRIGGER_PROPERTY      = (1 << 3),   /**< The client is capabile to trigger the IMEngine property. */
    SCIM_CLIENT_CAP_HELPER_MODULE         = (1 << 4),   /**< The client support helper module */
    SCIM_CLIENT_CAP_SURROUNDING_TEXT      = (1 << 5),   /**< The client support get/delete surrounding text operations */
    SCIM_CLIENT_CAP_ALL_CAPABILITIES      = 0x3F
};

/**
 * @brief An exception class to hold IMEngine related errors.
 *
 * scim::IMEngineBase and its derived classes must throw
 * scim::IMEngineError object when error.
 */
class IMEngineError: public Exception
{
public:
    IMEngineError (const String& what_arg)
        : Exception (String("scim::IMEngine: ") + what_arg) { }
};

class IMEngineFactoryBase;
class IMEngineInstanceBase;

/**
 * @typedef typedef Pointer <IMEngineFactoryBase> IMEngineFactoryPointer;
 *
 * A smart pointer for scim::IMEngineFactoryBase and its derived classes.
 */
typedef Pointer <IMEngineFactoryBase>  IMEngineFactoryPointer;

/**
 * @typedef typedef Pointer <IMEngineInstanceBase> IMEngineInstancePointer;
 *
 * A smart pointer for scim::IMEngineInstanceBase and its derived classes.
 */
typedef Pointer <IMEngineInstanceBase> IMEngineInstancePointer;

typedef Slot1<void, IMEngineInstanceBase*>
        IMEngineSlotVoid;

typedef Slot2<void, IMEngineInstanceBase*,int>
        IMEngineSlotInt;

typedef Slot2<void, IMEngineInstanceBase*,bool>
        IMEngineSlotBool;

typedef Slot2<void, IMEngineInstanceBase*,const String&>
        IMEngineSlotString;

typedef Slot2<void, IMEngineInstanceBase*,const WideString&>
        IMEngineSlotWideString;

typedef Slot2<void, IMEngineInstanceBase*,const KeyEvent&>
        IMEngineSlotKeyEvent;

typedef Slot2<void, IMEngineInstanceBase*,const LookupTable&>
        IMEngineSlotLookupTable;

typedef Slot2<void, IMEngineInstanceBase*,const Property&>
        IMEngineSlotProperty;

typedef Slot2<void, IMEngineInstanceBase*,const PropertyList&>
        IMEngineSlotPropertyList;

typedef Slot3<void, IMEngineInstanceBase*,const String&,const Transaction&>
        IMEngineSlotStringTransaction;

typedef Slot3<void, IMEngineInstanceBase*,const WideString&,const AttributeList&>
        IMEngineSlotWideStringAttributeList;

typedef Slot5<bool, IMEngineInstanceBase*,WideString&,int&,int,int>
        IMEngineSlotGetSurroundingText;

typedef Slot3<bool, IMEngineInstanceBase*,int,int>
        IMEngineSlotDeleteSurroundingText;

/**
 * @brief The base class of the real input methods' IMEngineFactory classes.
 *
 * Each input method should implement a class derived from scim::IMEngineFactoryBase,
 * which takes charge of holding shared data, creating IMEngineInstances etc.
 */
class IMEngineFactoryBase : public ReferencedObject
{
    class IMEngineFactoryBaseImpl;

    IMEngineFactoryBaseImpl *m_impl;

public:
    IMEngineFactoryBase ();

    /**
     * @brief Virtual destructor.
     */
    virtual ~IMEngineFactoryBase ();

    /**
     * @name Pure virtual members.
     *
     * These member functions must be implemented in derived classes.
     *
     * @{
     */

    /**
     * @brief Get the name of this input method engine.
     *
     * This name should be a localized string.
     *
     * @return A WideString containing the name.
     */
    virtual WideString  get_name () const = 0;

    /**
     * @brief Get the UUID of this input method engine.
     *
     * Each input method engine has an unique UUID to
     * distinguish itself from other engines.
     *
     * You may use uuidgen command shipped with e2fsprogs package to generate this UUID.
     *
     * @return A String containing an unique UUID.
     */
    virtual String      get_uuid () const = 0;

    /**
     * @brief Get the icon file path of this input method engine.
     *
     * @return A String containing the icon file path on the local filesystem.
     */
    virtual String      get_icon_file () const = 0;

    /**
     * @brief Get the authors information of this input method engine.
     *
     * This string should be a localized string.
     *
     * @return A WideString containing a list of the authors' name.
     */
    virtual WideString  get_authors () const = 0;

    /**
     * @brief Get the credits information of this input method engine.
     *
     * This string should be a localized string.
     *
     * @return A WideString containing the credits information.
     */
    virtual WideString  get_credits () const = 0;

    /**
     * @brief Get the help information of this input method engine.
     *
     * This string should be a localized string.
     *
     * @return A WideString containing the help information.
     */
    virtual WideString  get_help () const = 0;

    /**
     * @brief Create a new IMEngineInstance object.
     *
     * This method creates a new scim::IMEngineInstanceBase object with the given encoding and id.
     *
     * @param encoding - the encoding supported by the client.
     * @param id - the instance id, should be unique.
     * @return A smart pointer points to this new IMEngineInstance object.
     */
    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1) = 0;
    /**
     * @}
     */

    /**
     * @brief Check if an encoding is supported by this IMEngineFactory.
     *
     * The default implementation of this virtual function validates the
     * encoding against the locale list set by method set_locales.
     * 
     * It should be enough in most case.
     *
     * @param encoding - the encoding name to be checked.
     * @return true if the encoding is supported, otherwise false.
     */
    virtual bool validate_encoding (const String& encoding) const; 

    /**
     * @brief Check if a locale is supported by this IMEngineFactory.
     *
     * The default implementation of this virtual function validates the
     * locale against the locale list set by method set_locales.
     * 
     * It should be enough in most case.
     *
     * @param locale - the locale name to be checked.
     * @return true if the locale is supported, otherwise false.
     */
    virtual bool validate_locale (const String& locale) const;

    /**
     * @brief Get the supported language of this input method engine.
     *
     * The language name conforms to glibc locale naming standard, like:
     * zh_CN  Simplified Chinese
     * zh_TW  Traditional Chinese
     * ja_JP  Japanese
     * ru_RU  for Russian
     *
     * The second part of the name (territory id) can be omitted.
     *
     * The default implementation of this method will get the language name
     * according to the return value of get_default_locale () method.
     *
     * This method maybe overwrited to return another language name,
     * for example returning "~other" means other uncategorized languages.
     */ 
    virtual String get_language () const;

    /**
     * @brief Get the original key string of a composed string.
     *
     * For example, in the pinyin input method of Simplified Chinese:
     * the key string of composed string "中国" can be "zhongguo".
     *
     * The default implementation just returns a empty string.
     *
     * @param str The composed string to be queried.
     *
     * @return the original key string of the given composed string.
     */
    virtual WideString inverse_query (const WideString &str);

    /**
     * @brief Get the default locale of this input method engine.
     *
     * The default locale is the first locale in the locale list,
     * which is set by method set_locales.
     *
     * @return The default locale name.
     */
    String get_default_locale () const;

    /**
     * @brief Get the default encoding of this input method engine.
     *
     * The default encoding is the first locale's encoding in the locale list,
     * which is set by method set_locales.
     *
     * @return The default encoding name.
     */
    String get_default_encoding () const;

    /**
     * @brief Get a list of all supported locales, separated by comma.
     * 
     * @return A comma separated locale list.
     */
    String get_locales () const;

    /**
     * @brief Get a list of all supported encodings, separated by comma.
     *
     * @return A comma separated encoding list.
     */
    String get_encodings () const;

protected:
    /**
     * @brief Set the locales supported by this input method engine.
     *
     * This method should be called within the constructors of the derived classes.
     *
     * set_locales () and set_languages () are exclusive with each other. Only one
     * method should be used for one Factory object.
     *
     * @param locales - a comma separated list containing all valid locales
     *                  should be supported by this input method engine.
     *                  The first locale is the default one.
     */
    void set_locales (const String &locales);

    /**
     * @brief Set the languages supported by this input method engine.
     *
     * This method should be called within the constructors of the derived classes.
     *
     * set_locales () and set_languages () are exclusive with each other. Only one
     * method should be used for one Factory object.
     *
     * @param languages - a comma separated list containing all valid languages
     *                    should be supported by this input method engine.
     *                    The first language is the default one.
     */
    void set_languages (const String &languages);
};

/**
 * @brief The base class of the real input methods' IMEngineInstance classes.
 * 
 * Each input method should implement a class derived from scim::IMEngineInstanceBase,
 * which takes charge of recording Input Context status and processing user input events.
 */
class IMEngineInstanceBase : public ReferencedObject
{
    class IMEngineInstanceBaseImpl;

    IMEngineInstanceBaseImpl *m_impl;

public:
    /**
     * @brief Constructor.
     *
     * @param factory - the factory which creates this instance.
     * @param encoding - the working encoding.
     * @param id - the unique id of this instance.
     */
    IMEngineInstanceBase (IMEngineFactoryBase *factory,
                          const String        &encoding,
                          int                  id = -1);

    /**
     * @brief Virtual destructor.
     */
    virtual ~IMEngineInstanceBase ();

    /**
     * @brief Set the working encoding for this instance.
     *
     * One engine instance can only support one client encoding at the same time.
     * This encoding must be supported by the IMEngineFactory as well.
     *
     * This method could be overrided in derived class to do some extra job. But
     * the method of this base class must be invoked within the new method.
     *
     * After invoking this method, reset() should be invoked to
     * let the new encoding take effect.
     * 
     * @return true if the encoding is supported, otherwise false.
     */
    virtual bool set_encoding (const String &encoding);

    /**
     * @brief Get the working encoding of this instance.
     *
     * This method returns the encoding passed to the
     * constructor when constructing this object.
     *
     * @return The working encoding.
     */
    String get_encoding () const;

    /**
     * @brief Get the unique id of this instance.
     *
     * @return The id of this instance.
     */
    int get_id () const;

    /**
     * @brief Get the UUID of the engine factory.
     *
     * @return The UUID string of the engine factory.
     */
    String get_factory_uuid () const;

    /**
     * @brief Attach a pointer to this IMEngineInstance, which is pointed to corresponding FrontEnd data.
     *
     * @param data The pointer to corresponding FrontEnd data, eg. input context object.
     */
    void   set_frontend_data (void *data);

    /**
     * @brief Retrieve the pointer previously attached by set_frontend_data();
     *
     * @return The pointer previously attached by set_frontend_data();
     */
    void * get_frontend_data (void);

public:
    /**
     * @name Signal connection functions.
     *
     * These functions are used by FrontEnds to connect their corresponding slots to
     * this IMEngineInstance's signals.
     *
     * @{
     */
    Connection signal_connect_show_preedit_string     (IMEngineSlotVoid *slot);
    Connection signal_connect_show_aux_string         (IMEngineSlotVoid *slot);
    Connection signal_connect_show_lookup_table       (IMEngineSlotVoid *slot);
    Connection signal_connect_hide_preedit_string     (IMEngineSlotVoid *slot);
    Connection signal_connect_hide_aux_string         (IMEngineSlotVoid *slot);
    Connection signal_connect_hide_lookup_table       (IMEngineSlotVoid *slot);
    Connection signal_connect_update_preedit_caret    (IMEngineSlotInt *slot);
    Connection signal_connect_update_preedit_string   (IMEngineSlotWideStringAttributeList *slot);
    Connection signal_connect_update_aux_string       (IMEngineSlotWideStringAttributeList *slot);
    Connection signal_connect_update_lookup_table     (IMEngineSlotLookupTable *slot);
    Connection signal_connect_commit_string           (IMEngineSlotWideString *slot);
    Connection signal_connect_forward_key_event       (IMEngineSlotKeyEvent *slot);
    Connection signal_connect_register_properties     (IMEngineSlotPropertyList *slot);
    Connection signal_connect_update_property         (IMEngineSlotProperty *slot);
    Connection signal_connect_beep                    (IMEngineSlotVoid *slot);
    Connection signal_connect_start_helper            (IMEngineSlotString *slot);
    Connection signal_connect_stop_helper             (IMEngineSlotString *slot);
    Connection signal_connect_send_helper_event       (IMEngineSlotStringTransaction *slot);

    Connection signal_connect_get_surrounding_text    (IMEngineSlotGetSurroundingText *slot);
    Connection signal_connect_delete_surrounding_text (IMEngineSlotDeleteSurroundingText *slot);
    /** @} */

public:
    /**
     * @name Action functions.
     *
     * These functions will be called by FrontEnds to send events to
     * this IMEngineInstance.
     *
     * @{
     */

    /**
     * @brief Process a key event.
     *
     * @param key - the key event to be processed.
     * @return true if the event is processed, otherwise the event
     *         is not processed and should be forward to client application.
     */
    virtual bool process_key_event (const KeyEvent &key) = 0;

    /**
     * @brief Move the preedit caret in the preedit string.
     *
     * @param pos - the new position that user requested.
     */
    virtual void move_preedit_caret (unsigned int pos);

    /**
     * @brief Select a candidate in current lookup table.
     *
     * When user click a candidate directly,
     * this method will be invoked by FrontEnd.
     *
     * @param index - the index in current page of the selected candidate.
     */
    virtual void select_candidate (unsigned int index);

    /**
     * @brief Update the page size of current lookup table.
     *
     * In the next time, the lookup table should page down by
     * this size.
     *
     * @param page_size - the new size of current page.
     */
    virtual void update_lookup_table_page_size (unsigned int page_size);

    /**
     * @brief Flip the lookup table to the previous page.
     *
     * The method will be invoked by FrontEnd when user click
     * the lookup table page up button.
     */
    virtual void lookup_table_page_up ();

    /**
     * @brief Flip the lookup table to the next page.
     *
     * The method will be invoked by FrontEnd when user click
     * the lookup table page down button.
     */
    virtual void lookup_table_page_down ();

    /**
     * @brief Reset this engine instance.
     *
     * All status of this engine instance should be reset,
     * including the working encoding.
     *
     * The client encoding may have been changed before calling
     * this method, so if the IMEngine makes use of the client's encoding
     * information, it should check whether the encoding has been changed.
     * IMEngineInstance could call the get_encoding () method of base class
     * to get the client encoding.
     */
    virtual void reset ();

    /**
     * @brief Focus in this engine instance.
     *
     * This function should update/show/hide the status area,
     * preedit area and lookup table, and update the
     * full width punctuation/letter state.
     */
    virtual void focus_in ();

    /**
     * @brief Focus out this engine instance.
     */
    virtual void focus_out ();

    /**
     * @brief Trigger a property.
     *
     * This function should do some action according
     * to the triggered property.
     * For example toggle the input mode, etc.
     *
     * @param property the key of the triggered property.
     */
    virtual void trigger_property (const String &property);

    /**
     * @brief Process the events sent from a Client Helper process.
     *
     * @param helper_uuid The UUID of the Helper process which sent the events.
     * @param trans The transaction which contains the events.
     */
    virtual void process_helper_event (const String &helper_uuid, const Transaction &trans);

    /**
     * @brief Update the capabilities of current client application which is attached to this IMEngineInstance.
     *
     * Some client may not support all capabilities provided by the IMEngine API. For example:
     *
     * - OnTheSpot preedit string display (Embedded into client window).
     * - Property display
     * - Helper module
     * - etc.
     *
     * This method will be called to inform this IMEngineInstance object which capabilities are supported by the
     * client application. It may be called multiple times, if the capabilities was changed.
     *
     * @param cap A bitmask to indicate which client capabilities are supported by the client application.
     *
     * @sa scim::ClientCapability
     */
    virtual void update_client_capabilities (unsigned int cap);
    /** @} */

protected:
    /**
     * @name Signal activation functions
     * 
     * These functions should be called by derived classes
     * to fire the corresponding signals. The FrontEnd
     * connected to those signals will receive and process them.
     *
     * @{
     */

    /**
     * @brief Show the preedit string area.
     *
     * The preedit string should be updated by calling
     * update_preedit_string before or right after this call.
     */
    void show_preedit_string ();

    /**
     * @brief Show the aux string area.
     *
     * The aux string should be updated by calling
     * update_aux_string before or right after this call.
     *
     * The aux string can contain any additional information whatever
     * the input method engine want.
     */
    void show_aux_string ();

    /**
     * @brief Show the lookup table area.
     *
     * The lookup table should be updated by calling
     * update_lookup_table before or right after this call.
     */
    void show_lookup_table ();

    /**
     * @brief Hide the preedit string area.
     */
    void hide_preedit_string ();

    /**
     * @brief Hide the aux string area.
     */
    void hide_aux_string ();

    /**
     * @brief Hide the lookup table area.
     */
    void hide_lookup_table ();

    /**
     * @brief Update the preedit caret position in the preedit string.
     *
     * @param caret - the new position of the preedit caret.
     */
    void update_preedit_caret (int caret);

    /**
     * @brief Update the content of the preedit string,
     * 
     * @param str - the string content
     * @param attrs - the string attributes
     */
    void update_preedit_string (const WideString    &str,
                                const AttributeList &attrs = AttributeList ());

    /**
     * @brief Update the content of the aux string,
     * 
     * @param str - the string content
     * @param attrs - the string attribute
     */
    void update_aux_string (const WideString    &str,
                            const AttributeList &attrs = AttributeList ());

    /**
     * @brief Update the content of the lookup table,
     *
     * FrontEnd may reduce the page size of the table
     * according to screen resolution. If the page size
     * is changed, FrontEnd will inform this engine instance
     * by calling update_lookup_table_page_size method.
     *
     * @param table - the new LookupTable
     */
    void update_lookup_table (const LookupTable &table);

    /**
     * @brief Commit a string to the client application.
     *
     * The preedit string should be hid before calling
     * this method. Otherwise the clients which use
     * OnTheSpot input mode will flicker annoyingly.
     *
     * @param str - the string to be committed.
     */
    void commit_string (const WideString &str);

    /**
     * @brief Forward a key event to the client application.
     *
     * @param key - the key event to be forwarded.
     */
    void forward_key_event (const KeyEvent &key);

    /**
     * @brief Register all properties of this IMEngineInstance into the FrontEnd.
     *
     * The old properties previously registered by other IMEngineInstance will be discarded,
     * so for each time focus_in() is called, all properties should be registered
     * no matter whether they had been registered before.
     *
     * @param properties the PropertyList contains all of the properties.
     */
    void register_properties (const PropertyList &properties);

    /**
     * @brief Update a registered property.
     *
     * Update a property which already registered by register_properties () method.
     *
     * @param property the property to be updated.
     */
    void update_property (const Property &property);

    /**
     * @brief Generate a short beep.
     */
    void beep ();

    /**
     * @brief Start a Client Helper process.
     *
     * @param helper_uuid The UUID of the Helper object.
     */
    void start_helper (const String &helper_uuid);

    /**
     * @brief Stop a Client Helper process which was started by start_helper.
     *
     * @param helper_uuid The UUID of the Helper object.
     */
    void stop_helper (const String &helper_uuid);

    /**
     * @brief Send an events transaction to a client helper process.
     * 
     * @param helper_uuid The UUID of the Helper object.
     * @param trans The transaction which contains events.
     */
    void send_helper_event (const String &helper_uuid, const Transaction &trans);

    /**
     * @brief Retrieves context around the insertion point.
     * 
     * Input methods typically want context in order to constrain
     * input text based on existing text;
     * this is important for languages such as Thai where
     * only some sequences of characters are allowed.
     *
     * Unlike other signal activation actions, this action will return the result
     * immediately.
     *
     * @param text          location to store the context string around the insertion point.
     * @param cursor        location to store index of the insertion cursor within @text.
     * @param maxlen_before the maxmium length of context string to be retrieved
     *                      before the cursor; -1 means unlimited.
     * @param maxlen_after  the maxmium length of context string to be retrieved
     *                      after the cursor; -1 means unlimited.
     *
     * @return true if surrounding text was provided.
     */
    bool get_surrounding_text (WideString &text, int &cursor, int maxlen_before = -1, int maxlen_after = -1);

    /**
     * @brief Ask the client to delete characters around the cursor position.
     * 
     * In order to use this function, you should first call
     * get_surrounding_text () to get the current context, and
     * call this function immediately afterwards to make sure that you
     * know what you are deleting. You should also account for the fact
     * that even if the signal was handled, the input context might not
     * have deleted all the characters that were requested to be deleted.
     *
     * @param offset offset from cursor position in chars;
     *               a negative value means start before the cursor.
     * @param len number of characters to delete.
     *
     * @return true if the signal was handled.
     */
    bool delete_surrounding_text (int offset, int len);
    /** @} */
};

/**
 * @brief A trivial IMEngine that do nothing.
 */
class DummyIMEngineFactory : public IMEngineFactoryBase
{
public:
    DummyIMEngineFactory ();
    virtual ~DummyIMEngineFactory ();

    virtual WideString  get_name () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;

    virtual bool validate_encoding (const String& encoding) const;
    virtual bool validate_locale (const String& locale) const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class DummyIMEngineInstance : public IMEngineInstanceBase
{
public:
    DummyIMEngineInstance (DummyIMEngineFactory *factory,
                           const String         &encoding,
                           int                   id = -1);

    virtual ~DummyIMEngineInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void focus_in ();
};

/**  @} */

} // namespace scim

#endif //__SCIM_IMENGINE_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

