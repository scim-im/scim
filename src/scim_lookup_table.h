/** @file scim_lookup_table.h
 * @brief definition of LookupTable classes.
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
 * $Id: scim_lookup_table.h,v 1.32 2005/05/13 04:21:29 suzhe Exp $
 */

#ifndef __SCIM_LOOKUP_TABLE_H
#define __SCIM_LOOKUP_TABLE_H

namespace scim {
/**
 * @addtogroup Accessories
 * @{
 */

#define SCIM_LOOKUP_TABLE_MAX_PAGESIZE  16

/**
 * @brief The base class of lookup table.
 *
 * LookupTable is used to store the candidate phrases, it provides a easy way
 * to manage the content of candidates and flip between multiple pages.
 *
 * It also can manage the attributes for each candidate string.
 *
 * This is abstract class and cannot store data.
 * IMEngine should use its derivation class.
 * This class is the interface that uses within FrontEnd class.
 */
class LookupTable
{
    class LookupTableImpl;

    LookupTableImpl * m_impl;

    LookupTable (const LookupTable &);
    const LookupTable & operator= (const LookupTable &);

public:
    /**
     * @brief Constructor
     * @param page_size - the maximum page size, can be set by set_page_size() later.
     */
    LookupTable (int page_size = 10);

    /**
     * @brief Virtual destructor.
     */
    virtual ~LookupTable ();

    /**
     * @brief Set the strings to label the candidates in one page.
     * @param labels - the strings to label the candidates in one page.
     */
    void set_candidate_labels (const std::vector<WideString> &labels);

    /**
     * @brief Get the label string of a candidate in a page.
     * @param page_index - the index in a page, 0 to (max page size - 1).
     * @return the corresponding label of the index. 
     */
    WideString get_candidate_label (int page_index) const;

    /**
     * @brief Set the maximum page size.
     * @param page_size - the max page size of the table.
     */
    void set_page_size (int page_size); 

    /**
     * @brief Get the maximum page size.
     * @return the max page size of the table.
     */
    int get_page_size () const;

    /**
     * @brief Get current page size,
     * @return the page size of current page.It can be less than the max page size.
     */
    int get_current_page_size () const;

    /**
     * @brief Get the start index of current page.
     * @return the start item index of current page, starting from 0.
     */
    int get_current_page_start () const;

    /**
     * @brief Check if the cursor is visible.
     * @return true if the cursor should be shown.
     */
    bool is_cursor_visible () const;

    /**
     * @brief Check if the page size is fixed, aka. couldn't reduced by FrontEnd.
     * @return true if the page size shouldn't be reduced by FrontEnd.
     */
    bool is_page_size_fixed () const;

    /**
     * @brief Get current cursor position.
     * @return the cursor position in the table, starting from 0.
     */
    int get_cursor_pos () const;

    /**
     * @brief Get the cursor position in current page.
     * @return the cursor position in current page,
     *         equals to get_cursor_pos () - get_current_page_start ().
     */
    int get_cursor_pos_in_current_page () const;

    /**
     * @brief Flip to the previous page.
     * @return true if success, false if it's already in the first page.
     */
    bool page_up ();

    /**
     * @brief Flip to the next page.
     * @return true if success, false if it's already in the last page.
     */
    bool page_down ();

    /**
     * @brief Move cursor position to the previous entry.
     * @return true if success, false if it's already at the first entry.
     */
    bool cursor_up ();

    /**
     * @brief Move cursor position to the next entry.
     * @return true if success. false if it's already at the last entry.
     */
    bool cursor_down ();

    /**
     * @brief Set the cursor visibility.
     * @param show - true to show the cursor.
     */
    void show_cursor (bool show=true);

    /**
     * @brief Set the page size to be fixed, aka. prevent from being changed by FrontEnd.
     */
    void fix_page_size (bool fixed=true);

    /**
     * @brief Set the cursor position.
     * @param pos - the absolute position of the cursor.
     */
    void set_cursor_pos (int pos);

    /**
     * @brief Set the cursor position in current page.
     * @param pos - the relative position of the cursor in current page.
     */
    void set_cursor_pos_in_current_page (int pos);

    /**
     * @brief Get a candidate in current page.
     *
     * @param page_index - the candidate index in current page.
     * @return the content of this candidate.
     * 
     * @sa get_candidate
     */
    WideString get_candidate_in_current_page (int page_index) const;

    /**
     * @brief Get the display attributes of a candidate in current page.
     *
     * @param page_index - the index in current page.
     * @return the AttributeList object holding the attributes of this candidate.
     *
     * @sa get_attributes
     */
    AttributeList get_attributes_in_current_page (int page_index) const;

public:
    /**
     * @name Pure Virtual functions.
     * These functions should be implemented in derivation classes.
     *
     * @{
     */

    /**
     * @brief Get a candidate.
     * @param index - the candidate index in the lookup table.
     * @return the content of this candidate.
     */
    virtual WideString get_candidate (int index) const = 0;

    /**
     * @brief Get the attributes of a candidate.
     * @param index - the index in the lookup table.
     * @return the AttributeList object holding the attributes of this candidate.
     */
    virtual AttributeList get_attributes (int index) const = 0;

    /**
     * @brief Return the number of candidates in this table.
     * @return the number of entries currently in this table.
     */
    virtual uint32 number_of_candidates () const = 0;

    /**
     * @brief Clear the table.
     */
    virtual void clear () = 0;

    /**
     * @}
     */
};


/**
 * @brief A common lookup table class.
 *
 * This class implements the LookupTable interface in a common way.
 *
 */
class CommonLookupTable : public LookupTable
{
    class CommonLookupTableImpl;

    CommonLookupTableImpl *m_impl;

    CommonLookupTable (const CommonLookupTable &);
    const CommonLookupTable & operator= (const CommonLookupTable &);

public:
    CommonLookupTable (int page_size = 10);
 
    /**
     * @brief Constructor
     *
     * @param page_size - the maximum page size, can be set by set_page_size () later.
     * @param labels - the strings to label the candidates in one page.
     */
    CommonLookupTable (int                            page_size,
                       const std::vector<WideString> &labels);

    ~CommonLookupTable ();

    virtual WideString get_candidate (int index) const;

    virtual AttributeList get_attributes (int index) const;

    virtual uint32 number_of_candidates () const;

    virtual void clear ();

public:
    /**
     * @brief Append a candidate string into the table.
     *
     * @param cand  - a candidate string to be added into the table.
     * @param attrs - the attributes to control the display effect of this entry.
     *                It can be omitted if no attribute.
     *
     * @return true if success.
     */
    bool append_candidate (const WideString    &cand,
                           const AttributeList &attrs = AttributeList ());

    /**
     * @brief Append a candidate char into the table.
     *
     * @param cand  - a candidate char to be added into the table.
     * @param attrs - the attributes to control the display effect of this entry.
     *                It can be omitted if no attribute.
     *
     * @return true if success.
     */
    bool append_candidate (ucs4_t               cand,
                           const AttributeList &attrs = AttributeList ());
};

/** @} */

} // namespace scim

#endif //__SCIM_LOOKUP_TABLE_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
