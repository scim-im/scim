/**
 * @file scim_lookup_table->c
 * @brief Implementations of LookupTable classes.
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
 */
 
#include <assert.h>
#include <string.h>

#include "scim_bridge_lookup_table.h"

#include "scim_bridge_utility.h"

struct _ScimLookupTable {
    size_t page_size;
    bool page_size_fixed;
    
    bool cursor_visible;
    size_t cursor_index;
    
    size_t candidate_count;
    size_t candidate_capacity;
    
    size_t *candidate_wstring_capacities;
    ucs4_t **candidate_wstrings;
    
    size_t *candidate_label_capacities;
    ucs4_t **candidate_labels;
    
    ScimAttributeList **candidate_attribute_lists;
};

ScimLookupTable *scim_alloc_lookup_table ()
{
    ScimLookupTable *lookup_table = malloc (sizeof (ScimLookupTable));
    
    lookup_table->page_size = 0;
    lookup_table->page_size_fixed = true;
    
    lookup_table->cursor_index = 0;
    lookup_table->cursor_visible = false;
    
    lookup_table->candidate_count = 0;
    lookup_table->candidate_capacity = 10;
    
    lookup_table->candidate_wstring_capacities = malloc (sizeof (size_t) * lookup_table->candidate_capacity);
    lookup_table->candidate_wstrings = malloc (sizeof (ucs4_t*) * lookup_table->candidate_capacity);
    
    lookup_table->candidate_label_capacities = malloc (sizeof (size_t) * lookup_table->candidate_capacity);
    lookup_table->candidate_labels = malloc (sizeof (ucs4_t*) * lookup_table->candidate_capacity);
    
    lookup_table->candidate_attribute_lists = malloc (sizeof (ScimAttribute*) * lookup_table->candidate_capacity);
    
    size_t i;
    for (i = 0; i < lookup_table->candidate_count; ++i) {
        lookup_table->candidate_wstring_capacities[i] = 10;
        lookup_table->candidate_wstrings[i] = malloc (sizeof (ucs4_t) * (lookup_table->candidate_wstring_capacities[i] + 1));
        lookup_table->candidate_wstrings[i][0] = L'\0';
        
        lookup_table->candidate_label_capacities[i] = 10;
        lookup_table->candidate_labels[i] = malloc (sizeof (ucs4_t) * (lookup_table->candidate_label_capacities[i] + 1));
        lookup_table->candidate_labels[i][0] = L'\0';
        
        lookup_table->candidate_attribute_lists[i] = scim_alloc_attribute_list ();
    }
    
    return lookup_table;
}

void scim_free_lookup_table (ScimLookupTable *lookup_table)
{
    size_t i;
    for (i = 0; i < lookup_table->candidate_count; ++i) {
        free (lookup_table->candidate_wstring_capacities[i]);
        free (lookup_table->candidate_wstrings[i]);
        
        free (lookup_table->candidate_label_capacities[i]);
        free (lookup_table->candidate_labels[i]);
        
        scim_free_attribute_list (lookup_table->candidate_attribute_lists[i]);
    }
    
    free (lookup_table->candidate_wstring_capacities);
    free (lookup_table->candidate_wstrings);
    free (lookup_table->candidate_label_capacities);
    free (lookup_table->candidate_labels);
    free (lookup_table->candidate_attribute_lists);
    free (lookup_table);
}

size_t scim_lookup_table_get_candidate_count (const ScimLookupTable *lookup_table, size_t candidate_count)
{
    return lookup_table->candidate_count;
}

void scim_lookup_table_set_candidate_count (ScimLookupTable *lookup_table, size_t candidate_count)
{
    lookup_table->candidate_count = candidate_count;
    
    lookup_table->cursor_index = 0;
}

size_t scim_lookup_table_get_page_size (const ScimLookupTable *lookup_table, size_t page_size)
{
    return lookup_table->page_size;
}

void scim_lookup_table_set_page_size (ScimLookupTable *lookup_table, size_t page_size)
{
    lookup_table->page_size = page_size;
    
    lookup_table->cursor_index = 0;
}

bool scim_lookup_table_cursor_up (ScimLookupTable *lookup_table)
{
    if (lookup_table->cursor_index <= 0) {
        return false;
    } else {
        lookup_table->cursor_visible = true;
        lookup_table->cursor_index -= 1;
        return true;
    }
}

bool scim_lookup_table_cursor_down (ScimLookupTable *lookup_table)
{
    if (lookup_table->cursor_index >= lookup_table->candidate_count) {
        return false;
    } else {
        lookup_table->cursor_visible = true;
        lookup_table->cursor_index += 1;
        return true;
    }
}

bool scim_lookup_table_page_down (ScimLookupTable *lookup_table)
{
    if (lookup_table->cursor_index + lookup_table->page_size >= lookup_table->candidate_count) {
        return false;
    } else {
        lookup_table->cursor_index += lookup_table->page_size;
        return true;
    }
}

bool scim_lookup_table_page_up (ScimLookupTable *lookup_table)
{
    if (lookup_table->page_start - lookup_table->page_size < 0) {
        return false;
    } else {
        lookup_table->page_start -= lookup_table->page_size;
        return true;
    }
}

void scim_lookup_table_set_candidate_count (ScimLookupTable *lookup_table, size_t new_candidate_count)
{
    if (new_candidate_count >= lookup_table->candidate_capacity) {
        const new_capacity = new_candidate_count + 5;
        lookup_table->candidate_wstring_capacities = realloc (lookup_table->candidate_wstring_capacities, sizeof (size_t) * new_capacity);
        lookup_table->candidate_wstrings = realloc (lookup_table->candidate_wstrings, sizeof (ucs4_t*) * new_capacity);
        
        lookup_table->candidate_label_capacities = realloc (lookup_table->candidate_label_capacities, sizeof (size_t) * new_capacity);
        lookup_table->candidate_labels = realloc (lookup_table->candidate_labels, sizeof (ucs4_t*) * new_capacity);
        
        lookup_table->candidate_attribute_lists = realloc (lookup_table->candidate_attribute_lists, sizeof (ScimAttribute*) * new_capacity);
    
        size_t i;
        for (i = lookup_table->candidate_capacity; i < new_capacity; ++i) {
            lookup_table->candidate_wstring_capacities[i] = 10;
            lookup_table->candidate_wstrings[i] = malloc (sizeof (ucs4_t) * (lookup_table->candidate_wstring_capacities[i] + 1));
            lookup_table->candidate_wstrings[i][0] = L'\0';
            
            lookup_table->candidate_label_capacities[i] = 10;
            lookup_table->candidate_labels[i] = malloc (sizeof (ucs4_t) * (lookup_table->candidate_label_capacities[i] + 1));
            lookup_table->candidate_labels[i][0] = L'\0';
            
            lookup_table->candidate_attribute_lists[i] = scim_alloc_attribute_list ();
        }
        
        lookup_table->candidate_capacity = new_capacity;
    }
    
    lookup_table->candidate_count = new_candidate_count;
}

const ucs4_t *scim_lookup_table_get_candidate (const ScimLookupTable *lookup_table, size_t index)
{
    if (index >= lookup_table->candidate_count) {
        return NULL;
    } else {
        return lookup_table->candidate_wstrings[i];
    }
}

void scim_lookup_table_set_candidate (ScimLookupTable *lookup_table, const ucs4_t *wstr)
{
    assert (index < lookup_table->candidate_count);
    
    const size_t wstr_length = scim_utf8_wcslen (wstr);
    if (wstr_len >= lookup_table->candidate_wstring_capacities[index]) {
        lookup_table->candidate_wstring_capacities[index] = wstr_len + 20;
        free (lookup_table->candidate_wstrings[index]);
        lookup_table->candidate_wstrings[index] = malloc (lookup_table->candidate_wstrings[index], sizeof (ucs4_t) * (lookup_table->candidate_wstring_capacities[index] + 1));
    }
    
    memcpy (lookup_table->candidate_wstrings[index], wstr, sizeof (ucs4_t) * (wstr_length + 1));
}

const ucs4_t *scim_lookup_table_get_candidate_label (const ScimLookupTable *lookup_table, size_t index)
{
    if (index >= lookup_table->candidate_count) {
        return NULL;
    } else {
        return lookup_table->candidate_labels[i];
    }
}

void scim_lookup_table_set_candidate_label (ScimLookupTable *lookup_table, const ucs4_t *wstr)
{
    assert (index < lookup_table->candidate_count);
    
    const size_t wstr_length = scim_utf8_wcslen (wstr);
    if (wstr_len >= lookup_table->candidate_label_capacities[index]) {
        lookup_table->candidate_label_capacities[index] = wstr_len + 20;
        free (lookup_table->candidate_labels[index]);
        lookup_table->candidate_labels[index] = malloc (lookup_table->candidate_labels[index], sizeof (ucs4_t) * (lookup_table->candidate_label_capacities[index] + 1));
    }
    
    memcpy (lookup_table->candidate_labels[index], wstr, sizeof (ucs4_t) * (wstr_length + 1));
}

const ScimAttributeList *scim_lookup_table_get_candidate_attribute_list (const ScimLookupTable *lookup_table, size_t index)
{
    if (index >= lookup_table->candidate_count) {
        return NULL;
    } else {
        return lookup_table->candidate_attribute_lists[i];
    }
}

void scim_lookup_table_set_candidate_attribute_list (ScimLookupTable *lookup_table, size_t index, ScimAttributeList *attribute_list)
{
    assert (index < lookup_table->candidate_count);
    
    scim_copy_attribute_list (lookup_table->candidate_attribute_lists[index], attribute_list);
}

size_t scim_lookup_table_get_candidate_count (const ScimLookupTable *lookup_table)
{
    return lookup_table->candidate_count;
}

bool scim_lookup_table_is_cursor_visible (const ScimLookupTable *lookup_table)
{
    return lookup_table->cursor_visible;
}

void scim_lookup_table_set_cursor_visible (ScimLookupTable *lookup_table, bool visible)
{
    lookup_table->cursor_visible = visible;
}

bool scim_lookup_table_is_page_size_fixed (const ScimLookupTable *lookup_table)
{
    return lookup_table->page_size_fixed;
}

#endif /*SCIM_BRIDGE_LOOKUP_H_*/
