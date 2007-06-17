/** @file scim_properties.h
 *  @brief Definition of scim::Property and scim::PropertyList
 *
 *  Provide class scim::Property to hold of a property 
 *  of a IMEngineInstance or a Panel GUI client.
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
 * $Id: scim_property.h,v 1.7 2005/01/10 08:30:54 suzhe Exp $
 */

#ifndef __SCIM_PROPERTY_H
#define __SCIM_PROPERTY_H

namespace scim {

#define SCIM_PROPERTY_ACTIVE     0x01
#define SCIM_PROPERTY_VISIBLE    0x02

/**
 * @addtogroup Accessories
 * @{
 */

/**
 * @brief Class to hold a property of a IMEngineInstance object or a Panel GUI client.
 *
 * A property has four elements:
 *   - key
 *     An unique identify key of the property, for example:
 *     - /TableInstance
 *     - /TableInstance/FullWidthLetter
 *     In this case, the second property will be a leaf (maybe a submenu item) of the first property.
 *   - label
 *     A label of the property which should be displayed.
 *   - icon
 *     An icon file of the property which should be displayed along with label.
 *   - tip
 *     A string to descript what the property means.
 *
 * With path like keys, the properties can form a cascade structure, which may be displayed
 * like a cascading menu.
 * 
 * But only the leaf properties can act as trigger commands
 * and give feedback to IMEngineInstance.
 *
 * All strings should be encoded into UTF-8.
 */
class Property
{
    String m_key;
    String m_label;
    String m_icon;
    String m_tip;
    uint16 m_state;

public:
    /**
     * @brief Default constructor
     */
    Property () : m_state (0) { }

    /**
     * @brief Constructor
     */
    Property (const String &key,
              const String &label,
              const String &icon = String (""),
              const String &tip = String (""))
        : m_key (key), m_label (label), m_icon (icon),
          m_tip (tip), m_state (SCIM_PROPERTY_VISIBLE | SCIM_PROPERTY_ACTIVE) { }

    /**
     * @brief Test if this property is valid.
     *
     * @return true if this property is valid.
     */
    bool valid () const { return m_key.length (); }

    /**
     * @brief If this property is visible.
     *
     * @return true if this property is visible.
     */
    bool visible () const { return m_state & SCIM_PROPERTY_VISIBLE; }

    /**
     * @brief If this property is active.
     *
     * A active property can be clicked by users.
     *
     * @return true if this property is active.
     */
    bool active () const { return m_state & SCIM_PROPERTY_ACTIVE; }

    /**
     * @brief Get the key of this property.
     */
    const String & get_key   () const { return m_key; }

    /**
     * @brief Get the label of this property.
     */
    const String & get_label () const { return m_label; }

    /**
     * @brief Get the icon file of this property.
     */
    const String & get_icon  () const { return m_icon; }

    /**
     * @brief Get the tip of this property.
     */
    const String & get_tip  () const { return m_tip; }

    /**
     * @brief Set a new key for this property.
     */
    void set_key   (const String & key)   { m_key = key; }

    /**
     * @brief Set a new label for this property.
     */
    void set_label (const String & label) { m_label = label; }

    /**
     * @brief Set a new icon file for this property.
     */
    void set_icon  (const String & icon)  { m_icon = icon; }

    /**
     * @brief Set a new tip for this property.
     */
    void set_tip  (const String & tip)  { m_tip = tip; }

    /**
     * @brief Set if this property is active.
     *
     * @param active If this property is active.
     */
    void set_active (bool active) {
        if (active) m_state |= SCIM_PROPERTY_ACTIVE;
        else m_state &= (~ SCIM_PROPERTY_ACTIVE);
    }

    void show (bool visible = true) {
        if (visible) m_state |= SCIM_PROPERTY_VISIBLE;
        else m_state &= ~SCIM_PROPERTY_VISIBLE;
    }

    void hide (bool hidden = true) { show (!hidden); }

    /**
     * @brief Test if this property is a leaf of another one.
     *
     * @return true if this property is a leaf of the node.
     */
    bool is_a_leaf_of (const Property &node) const {
        if (m_key.length () > node.m_key.length () &&
            m_key.substr (0, node.m_key.length ()) == node.m_key &&
            m_key [node.m_key.length ()] == '/')
            return true;
        return false;
    }
};

inline bool
operator < (const Property &lhs, const Property &rhs) {
    return lhs.get_key () < rhs.get_key ();
}

inline bool
operator < (const Property &lhs, const String &rhs) {
    return lhs.get_key () < rhs;
}

inline bool
operator < (const String &lhs, const Property &rhs) {
    return lhs < rhs.get_key ();
}

inline bool
operator == (const Property &lhs, const Property &rhs) {
    return lhs.get_key () == rhs.get_key ();
}

inline bool
operator == (const Property &lhs, const String &rhs) {
    return lhs.get_key () == rhs;
}

inline bool
operator == (const String &lhs, const Property &rhs) {
    return lhs == rhs.get_key ();
}

inline bool
operator != (const Property &lhs, const Property &rhs) {
    return lhs.get_key () != rhs.get_key ();
}

inline bool
operator != (const Property &lhs, const String &rhs) {
    return lhs.get_key () != rhs;
}

inline bool
operator != (const String &lhs, const Property &rhs) {
    return lhs != rhs.get_key ();
}

/**
 * @typedef std::vector<Property>  PropertyList;
 * @brief The container to store a set of Properties.
 *
 * You should use the STL container methods to manipulate its objects.
 */
typedef std::vector<Property>  PropertyList;

/** @} */

} // namespace scim

#endif //__SCIM_PROPERTY_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
