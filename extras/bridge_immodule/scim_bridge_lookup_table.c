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

#include "scim_bridge_lookup_table->h"

struct _ScimLookupTable {
    size_t page_start;
    size_t page_size;
    bool cursor_visible;
    size_t cursor_index;
    
    size_t candidate_count;
    size_t candidate_capacity;
    size_t *candidate_label_capacities;
    ucs4_t **candidate_labels;
    
    ScimAttributeList **candidate_attribute_lists;
};

ScimLookupTable *scim_alloc_lookup_table ()
{
    ScimLookupTable *lookup_table = malloc (sizeof (ScimLookupTable));
    
    lookup_table->page_start = 0;
    lookup_table->page_size = 0;
    lookup_table->cursor_index = 0;
    lookup_table->cursor_visible = false;
    
    lookup_table->candidate_count = 0;
    lookup_table->candidate_capacity = 10;
    
    lookup_table->candidate_label_capacities = malloc (sizeof (size_t) * lookup_table->candidate_capacity);
    lookup_table->candidate_labels = malloc (sizeof (ucs4_t*) * lookup_table->candidate_capacity);
    lookup_table->candidate_attribute_lists = malloc (sizeof (ScimAttribute*) * lookup_table->candidate_capacity);
    
    size_t i;
    for (i = 0; i < lookup_table->candidate_count; ++i) {
        lookup_table->candidate_label_capacities[i] = 10;
        lookup_table->candidate_labels[i] = malloc (sizeof (ucs4_t) * (lookup_table->candidate_label_capacities[i] + 1));
        lookup_table->candidate_attribute_lists[i] = scim_alloc_attribute_list ();
    }
    
    return lookup_table;
}

void scim_free_lookup_table (ScimLookupTable *lookup_table)
{
    size_t i;
    for (i = 0; i < lookup_table->candidate_count; ++i) {
        free (lookup_table->candidate_label_capacities[i]);
        free (lookup_table->candidate_labels[i]);
        scim_free_attribute_list (lookup_table->candidate_attribute_lists[i]);
    }
    
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
    lookup_table->page_start = 0;
}

size_t scim_lookup_table_get_page_size (const ScimLookupTable *lookup_table, size_t page_size)
{
    return lookup_table->page_size;
}

void scim_lookup_table_set_page_size (ScimLookupTable *lookup_table, size_t page_size)
{
    lookup_table->page_size = page_size;
    
    lookup_table->cursor_index = 0;
    lookup_table->page_start = 0;
}

bool scim_lookup_table_cursor_up (ScimLookupTable *lookup_table)
{
    if (lookup_table->cursor_index <= 0)
        return false;
    
    lookup_table->cursor_visible = true;
    lookup_table->cursor_index -= 1;
    if (lookup_table->cursor_index <= lookup_table->page_start) {
        scim_lookup_table_page_up (lookup_table);
        lookup_table->cursor_index = lookup_table->page_start + lookup_table->page_size - 1;
    }
    return true;
}

bool scim_lookup_table_cursor_down (ScimLookupTable *lookup_table)
{
    if (lookup_table->cursor_index >= lookup_table->candidate_count)
        return false;
    
    lookup_table->cursor_visible = true;
    lookup_table->cursor_index += 1;
    if (lookup_table->cursor_index >= lookup_table->page_start + lookup_table->page_size) {
        scim_lookup_table_page_down (lookup_table);
        lookup_table->cursor_index = lookup_table->page_start;
    }
    return true;
}

bool scim_lookup_table_page_down (ScimLookupTable *lookup_table)
{
    if (lookup_table->page_start + lookup_table->page_size >= lookup_table->candidate_count)
        return false;

    lookup_table->page_start += lookup_table->page_size;
    lookup_table->cursor_index += lookup_table->page_size;
    
    if (lookup_table->cursor_index < lookup_table->page_start) {
        lookup_table->cursor_index = lookup_table->page_start;
    } else if (lookup_table->cursor_index >= lookup_table->page_start + lookup_table->page_size) {
        lookup_table->cursor_index = lookup_table->page_start + lookup_table->page_size - 1;
    }

    return true;
}

bool scim_lookup_table_page_up (ScimLookupTable *lookup_table)
{
    if (lookup_table->page_start - lookup_table->page_size < 0)
        return false;

    lookup_table->page_start -= lookup_table->page_size;
    lookup_table->cursor_index -= lookup_table->page_size;
        
    if (lookup_table->cursor_index < lookup_table->page_start) {
        lookup_table->cursor_index = lookup_table->page_start;
    } else if (lookup_table->cursor_index >= lookup_table->page_start + lookup_table->page_size) {
        lookup_table->cursor_index = lookup_table->page_start + lookup_table->page_size - 1;
    }

    return true;
}

#endif /*SCIM_BRIDGE_LOOKUP_H_*/
