/**
 * @file scim_bridge_transaction.c
 * implementation of bridged transaction
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
#include <stdlib.h>
#include <string.h>

#include "scim_bridge_types.h"
#include "scim_bridge_utility.h"
#include "scim_bridge_transaction.h"

/**
 * @brief Signature of all valid data types which can be store into transaction.
 */
enum ScimTransactionDataType
{
    SCIM_TRANS_DATA_UNKNOWN = 0,            //!< Unknown transaction data type.
    SCIM_TRANS_DATA_COMMAND = 1,         //!< Send/Receive command.
    SCIM_TRANS_DATA_RAW = 2,             //!< Send/Receive raw buffer.
    SCIM_TRANS_DATA_UINT32 = 3,          //!< Send/Receive uint32.
    SCIM_TRANS_DATA_STRING = 4,          //!< Send/Receive String.
    SCIM_TRANS_DATA_WSTRING = 5,         //!< Send/Receive WideString.
    SCIM_TRANS_DATA_KEYEVENT = 6,        //!< Send/Receive KeyEvent.
    SCIM_TRANS_DATA_ATTRIBUTE_LIST = 7,    //!< Send/Receive AttributeList.
    SCIM_TRANS_DATA_LOOKUP_TABLE = 8,    //!< Send/Receive LookupTable.
    SCIM_TRANS_DATA_PROPERTY = 9,        //!< Send/Receive Property.
    SCIM_TRANS_DATA_PROPERTY_LIST = 10,  //!< Send/Receive PropertyList.
    SCIM_TRANS_DATA_VECTOR_UINT32 = 11,  //!< Send/Receive vector<uint32>.
    SCIM_TRANS_DATA_VECTOR_STRING = 12,  //!< Send/Receive vector<String>.
    SCIM_TRANS_DATA_VECTOR_WSTRING = 13, //!< Send/Receive vector<WideString>.
    SCIM_TRANS_DATA_TRANSACTION = 14     //!< Send/Receive another Transaction.
};

struct _ScimTransaction
{
    size_t buffer_size;
    size_t write_position;
    size_t read_position;
    unsigned char *buffer;
};

ScimTransaction *scim_alloc_transaction ()
{
    ScimTransaction *transaction = malloc (sizeof (ScimTransaction));
    transaction->buffer_size = 256;
    transaction->buffer = malloc (transaction->buffer_size);
    transaction->write_position = 0;
    transaction->read_position = 0;
}

void scim_free_transaction (ScimTransaction *transaction)
{
    free (transaction->buffer);
    free (transaction);
}

/**
 * @brief Check the remaining buffer size of the transaction, and reserve more space if needed.
 * 
 * @param trans The transaction.
 * @param buffer_size The size to reserve.
 */
static inline void scim_transaction_request_buffer_size (ScimTransaction *trans, size_t buffer_size)
{
    const size_t remaining_size = trans->buffer_size - write_position;
    if (remaining_size < buffer_size)
        transaction->buffer = realloc (transaction->buffer, transaction->buffer_size + buffer_size * 2);
}

/**
 * @brief Put a data into the transaction.
 * 
 * @param trans The transaction.
 * @param data The data to write in.
 * @param size The size of the data.
 */

static inline void scim_transaction_put_data (ScimTransaction *trans, void *data, size_t size)
{
    scim_transaction_request_buffer_size (trans, size);
    memcpy (bridge_transaction->buffer + bridge_transaction->write_position, data, size);
    scim_transaction->write_position += size;    
}

/**
 * @brief Get a data from the transaction.
 * 
 * @param trans The transaction.
 * @param data The buffer to write in.
 * @param size The size of the data.
 * @return true if it succeeded.
 */
static inline bool scim_transaction_get_data (ScimTransaction *trans, void *data, size_t size)
{
    if (trans->read_position + size > trans->write_position) {
        return false;
    } else {
        memcpy (data, trans->buffer + trans->read_position, size);
        trans->read_position += size;
        return true;
    }
}

void scim_transaction_put_command (ScimTransaction *trans, int cmd)
{
    const uint32 header = SCIM_TRANS_DATA_COMMAND;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    scim_transaction_put_data (trans, &cmd, sizeof (uint32));
}

bool scim_transaction_get_command (ScimTransaction *trans, int *cmd)
{
    uint32 header;
    return (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_COMMAND && 
            scim_transaction_get_data (trans, cmd, sizeof (uint32)));
}

void scim_transaction_put_uint32 (ScimTransaction *trans, uint32 value)
{
    const uint32 header = SCIM_BRIDGE_TRANS_DATA_UINT32;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    scim_transaction_put_data (trans, &cmd, sizeof (uint32));
}

bool scim_transaction_get_uint32 (ScimTransaction *trans, uint32 *value)
{
    const uint32 header;
    return (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_DATA_UINT32 && 
            scim_transaction_get_data (trans, value, sizeof (uint32)));
}

void scim_transaction_put_string (ScimTransaction *trans, const char* str)
{
    const uint32 header = SCIM_TRANS_DATA_STRING;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    
    const uint32 str_length = strlen (str);
    scim_transaction_put_data (trans, &str_length, sizeof (str_length));
    
    scim_transaction_put_data (trans, str, str_length);
}

bool scim_transaction_get_string (ScimTransaction *trans, char **str)
{
    uint32 header;
    uint32 str_length;
    if (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_DATA_STRING && 
            scim_transaction_get_data (trans, &str_length, sizeof (uint32))) {
        char *str_buffer = malloc (sizeof (char) * (str_length + 1));
        str_buffer[str_length] = '\0';
        *str = str_buffer;
        return scim_transaction_get_data (trans, str_buffer, str_length);
    } else {
         return false;
    }
}

void scim_transaction_put_wstring (ScimTransaction *trans, const wchar* wstr)
{
    const uint32 header = SCIM_TRANS_DATA_WSTRING;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    
    const size_t wstr_length = bridge_utf8_wcslen (wstr);
    if (wstr_length >= 0) {
        const char *str;
        const size_t mbs_length = s
        cim_utf8_wcstombs (&str, wstr, wstr_len);
        assert (mbs_length >= 0);
        const uint32 str_length = mbs_length;
        scim_transaction_put_data (trans, &str_length, sizeof (str_length));
        scim_transaction_put_data (trans, str, str_length);
        free (str);
    } else {
        const uint32 str_length = 0;
        scim_transaction_put_data (trans, &str_length, sizeof (str_length));
        
        const char str[0] = {'\0'};
        scim_transaction_put_data (trans, str, str_length);
    }
}

bool scim_transaction_get_wstring (ScimTransaction *trans, ucs4_t **wstr)
{
    uint32 header;
    uint32 str_length;
    if (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_DATA_WSTRING && 
            scim_transaction_get_data (trans, &str_length, sizeof (uint32))) {
        char *str_buffer = malloc (sizeof (char) * (str_length + 1));
        str_buffer[str_length] = '\0';
        *str = str_buffer;
        
        bool retval;
        if (scim_transaction_get_data (trans, str_buffer, str_length)) {
            if (scim_utf8_mbstowcs (wstr, str, str_length) >= 0) {
                return true;
            } else {
                return false;
            }
        } else {
            retval = false;
        }
        free (str_buffer);
        return retval;
    } else {
         return false;
    }    
}

void scim_transaction_put_key_event (ScimTransaction *trans, const ScimKeyEvent key_event)
{
    const uint32 header = SCIM_TRANS_DATA_KEYEVENT;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    scim_transaction_put_data (trans, &key_event->code, sizeof (uint32));
	scim_transaction_put_data (trans, &key_event->modifier, sizeof (uint16));
    scim_transaction_put_data (trans, &key_event->layout, sizeof (uint16));
}

bool scim_transaction_get_key_event (ScimTransaction *trans, ScimKeyEvent *key_event)
{
	uint32 header;
	if (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_DATA_KEYEVENT) {
        uint32 code;
        uint16 modifiers;
        uint16 layout;
        if (scim_transaction_get_data (trans, &code, sizeof (uint32)) && 
                scim_transaction_get_data (trans, &modifier, sizeof (uint16)) && 
                scim_transaction_get_data (trans, &layout, sizeof (uint16))) {
            key_event->code = code;
            key_event->modifiers = modifiers;
            key_event->layout = layout;
            return true;
        }
    }
    
    return false;
}

void scim_transaction_put_attributes (ScimTransaction *trans, const ScimAttributeList *attribute_list)
{
    const uint32 header = SCIM_TRANS_DATA_ATTRIBUTE_LIST;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    
    const uint32 attribute_count = scim_attribute_get_size (attribute_list);
    scim_transaction_put_data (trans, &attribute_count, sizeof (uint32));
    
    size_t i;
    for (i = 0; i < attribute_count; ++i) {
        const ScimAttribute *attribute = scim_attribute_list_get ((ScimAttributeList*) attribute_list, i);
        const uint32 type = attribute->type;
        scim_transaction_put_data (trans, &type, sizeof (uint32);
        
        const uint32 value = attribute->value;
        scim_transaction_put_data (trans, &value, sizeof (uint32);
        
        const uint32 start = attribute->start;
        scim_transaction_put_data (trans, &start, sizeof (uint32);
        
        const uint32 length = attribute->length;
        scim_transaction_put_data (trans, &length, sizeof (uint32);
    }
}

bool scim_transaction_get_attribute_list (ScimTransaction *trans, ScimAttributeList *attribute_list)
{
    uint32 type;
    if (scim_transaction_get_data (trans, &type, sizeof (uint32)) && type == SCIM_TRANS_DATA_ATTRIBUTE_LIST) {
        uint32 attribute_count;
        if (!scim_transaction_get_data (trans, &attribute_count, sizeof (uint32))
            return false;
        
        scim_attribute_list_set_size (attribute_list, attribute_count);
        
        size_t i;
        for (i = 0; i < attribute_count; ++i) {
            uint32 type;
            uint32 value;
            uint32 start;
            uint32 length;
            
            if (scim_transaction_get_data (trans, &type, sizeof (uint32)) && 
                    scim_transaction_get_data (trans, &value, sizeof (uint32)) && 
                    scim_transaction_get_data (trans, &start, sizeof (uint32)) && 
                    scim_transaction_get_data (trans, &length, sizeof (uint32))) {
                        
                ScimAttribute *attribute = scim_attribute_list_get (attribute_list, i);
                attribute->type = type;
                attribute->value = value;
                attribute->start = start;
                attribute->length = length;
            } else {
                return false;
            }
        }
    }
    
    return true;
}

void scim_transaction_put_property (ScimTransaction *trans, const ScimProperty *property)
{
    const uint32 header = SCIM_TRANS_DATA_PROPERTY;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    
    scim_transaction_put_string (trans, property->key);
    scim_transaction_put_string (trans, property->label);
    scim_transaction_put_string (trans, property->icon);
    scim_transaction_put_string (trans, property->tip);
    
    unsigned const char visible = property->status & SCIM_PROPERTY_VISIBLE;
    unsigned const char active = property->status & SCIM_PROPERTY_ACTIVE;
    
    scim_transaction_put_data (trans, &visible, sizeof (unsigned char));
    scim_transaction_put_data (trans, &active, sizeof (unsigned char));   
}

bool scim_transaction_get_property (ScimTransaction *trans, ScimProperty *property)
{
    uint32 header;
    if (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_DATA_PROPERTY) {
        char *key = NULL;
        char *label = NULL;
        char *icon = NULL;
        char *tip = NULL;
        unsigned char visible;
        unsigned char active;
        if (scim_transaction_get_string (trans, key) && scim_transaction_get_string (trans, label) && 
                scim_transaction_get_string (trans, icon) && scim_transaction_get_string (trans, tip) && 
                scim_transaction_get_data (trans, &visible, sizeof (unsigned char)) && scim_transaction_get_data (trans, &active, sizeof (unsigned char))) {
            
            status = 0;
            if (visible)
                status |= SCIM_PROPERTY_VISIBLE;
            if (active)
                status |= SCIM_PROPERTY_ACTIVE;
            
            property->key = key;
            property->label = label;
            property->icon = icon;
            property->tip = tip;
            property->status = status;
            return true;
        } else {
            if (key != NULL)
                free (key);
            if (label != NULL)
                free (label);
            if (icon != NULL)
                free (icon);
            if (tip != NULL)
                free (tip);
        }
    }
    
    return false;
}

void scim_transaction_put_property_list (ScimTransaction *trans, const ScimPropertyList *property_list)
{
    const uint32 header = SCIM_TRANS_DATA_PROPERTY_LIST;
    scim_transaction_put_data (trans, &header, sizeof (uint32));
    
    const uint32 size = scim_property_list_get_size (property_list);
    scim_transaction_put_data (trans, &size, sizeof (uint32));
    
    size_t i;
    for (i = 0; i < size; ++i) {
        const ScimProperty *property = scim_property_list_get ((ScimPropertyList*) property_list, i);
        scim_transaction_put_property (trans, property);
    }
}

bool scim_transaction_get_property_list (ScimTransaction *trans, ScimPropertyList *property_list)
{
    uint32 header;
    uint32 size;
    if (scim_transaction_get_data (trans, &header, sizeof (uint32)) && header == SCIM_TRANS_DATA_PROPERTY_LIST && 
            scim_transaction_get_data (trans, &size, sizeof (uint32))) {
        
        scim_property_list_set_size (property_list, size);
        scim_transaction_put_data (trans, &size, sizeof (uint32));
        
        size_t i;
        for (i = 0; i < size; ++i) {
            ScimProperty *property = scim_property_list_get (property_list, i);
            if (!scim_transaction_get_property (trans, property))
                return false;
        }
        
        return true;
    }
    
    return false;
}