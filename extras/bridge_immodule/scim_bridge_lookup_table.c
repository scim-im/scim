/**
 * @file scim_lookup_table.c
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

#include "scim_bridge_lookup_table.h"

struct _ScimLookupTable {
    size_t page_size;
    size_t candidate_index;
    size_t candidate_count;
    size_t candidate_capacity;
    size_t *candidate_label_capacities;
    ucs4_t **candidate_labels;
    ScimAttributeList **candidate_attribute_lists;
};

ScimLookupTable *scim_alloc_lookup_table ()
{
    ScimLookupTable *lookup_table = malloc (sizeof (ScimLookupTable));
    
    lookup_table->page_size = 0;
    lookup_table->candidate_index = 0;
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

#endif /*SCIM_BRIDGE_LOOKUP_H_*/
