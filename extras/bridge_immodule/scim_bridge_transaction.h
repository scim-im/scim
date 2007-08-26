/**
 * @file scim_bridge_transaction.h
 * @brief Bridged socket interfaces.
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
#ifndef SCIM_BRIDGE_TRANSACTION_H_
#define SCIM_BRIDGE_TRANSACTION_H_

#include "scim_bridge_attribute.h"
#include "scim_bridge_key_event.h"

/**
 * @brief This struct is used to pack up many data and commands into one package
 *        and send them via socket.
 */
typedef struct _ScimTransaction ScimTransaction;

/**
 * @brief Allocate a new transaction.
 * 
 * @return The new transaction.
 */
ScimTransaction *scim_alloc_transaction ();

/**
 * @breif Free a transaction.
 * 
 * @param trans The transaction to free.
 */
void scim_free_transaction (ScimTransaction *trans);

/**
 * @brief Write the transaction into a socket.
 * 
 * @param trans The transaction.
 * @param socket_fd The file descriptor for the socket to write into.
 * @return true if success.
 */
bool scim_transaction_write_to_socket (const ScimTransaction *trans, int socket_fd);

/**
 * @brief Read a transaction from a socket.
 *
 * @param trans The transaction.
 * @param socket_fd the file descriptor for socket to be read from.
 * @param timeout time out in millisecond (1/1000 second), -1 means infinity.
 * @return true if success.
 */
bool scim_transaction_read_from_socket (ScimTransaction *trans, int socket_fd, int timeout = -1);

/**
 * @brief Store a command into the transaction.
 *
 * @param trans The transaction.
 * @param cmd the command code, like SCIM_TRANS_CMD_FOCUS_IN etc.
 */
void scim_transaction_put_command (ScimTransaction *trans, int cmd);

/**
 * @brief Get a command from current read position.
 * 
 * @param trans The transaction.
 * @param cmd The buffer to write in.
 * @return true if it succeeded.
 */
bool scim_transaction_get_command (ScimTransaction *trans, int *cmd);

/**
 * @brief Store a uint32 into the transaction.
 * 
 * @param trans The transaction.
 * @param value The value to store.
 */
void scim_transaction_put_uint32 (ScimTransaction *trans, uint32 value);

/**
 * @brief Get an uint32 value from current read position.
 * 
 * @param trans The transaction.
 * @param value The buffer to write in.
 * @return true if it succeeded.
 */
bool scim_transaction_get_uint32 (ScimTransaction *trans, uint32 *value);

/**
 * @brief Store a string into the transaction.
 * 
 * @param trans The transaction.
 * @param str The value to store.
 */
void scim_transaction_put_string (ScimTransaction *trans, const char* str);

/**
 * @brief Get a string from current read position.
 * 
 * @param trans The transaction.
 * @param str The buffer to write in.
 * @return true if it succeeded.
 */
bool scim_transaction_get_string (ScimTransaction *trans, char **str);

/**
 * @brief Store a wide string into the transaction.
 * 
 * @param trans The transaction.
 * @param wstr The value to store.
 */
void scim_transaction_put_wstring (ScimTransaction *trans, const wchar* wstr);

/**
 * @brief Get a wide string from current read position.
 * 
 * @param trans The transaction.
 * @param str The buffer to write in.
 * @return true if it succeeded.
 */
bool scim_transaction_get_wstring (ScimTransaction *trans, ucs4_t **wstr);

/**
 * @brief Store a key event into the transaction.
 * 
 * @param trans The transaction.
 * @param key_event The value to store.
 */
void scim_transaction_put_key_event (ScimTransaction *trans, const ScimKeyEvent key_event);

/**
 * @brief Get a key event from the transaction.
 * 
 * @param trans The transaction.
 * @param key_event The buffer to write in.
 * @return true if it succeeded.
 */
bool scim_transaction_get_key_event (ScimTransaction *trans, ScimKeyEvent *key_event);

/**
 * @brief Store a set of string attributes into the transaction.
 * 
 * @param trans The transaction.
 * @param attribute_list The value to store.
 */
void scim_transaction_put_attribute_list (ScimTransaction *trans, const ScimAttributeList *attribute_list);

/**
 * @brief Get a string attributes from the transaction.
 * 
 * @param trans The transaction.
 * @param attribute_list The destination for the attribute list.
 * @return true if it succeeded.
 */
bool scim_transaction_get_attribute_list (ScimTransaction *trans, ScimAttributeList *attribute_list);

#endif /*SCIM_BRIDGE_TRANSACTION_H_*/
