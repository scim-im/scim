/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "scim-bridge-message.h"
#include "scim-bridge-output.h"

/* Type definition */
struct _ScimBridgeMessage
{
    char *header;

    char **arguments;
    size_t *argument_capacities;
    size_t argument_count;
};

/* Implementations */
ScimBridgeMessage *scim_bridge_alloc_message (const char *header, size_t argument_count)
{
    if (header == NULL) {
        scim_bridge_perrorln ("The given header of a message is NULL");
        return NULL;
    }

    ScimBridgeMessage *message = malloc (sizeof (ScimBridgeMessage));

    message->header = malloc (sizeof (char) * (strlen (header) + 1));
    strcpy (message->header, header);

    message->argument_count = argument_count;
    if (argument_count > 0) {
        message->arguments = malloc (sizeof (char*) * message->argument_count);
        message->argument_capacities = malloc (sizeof (size_t) * message->argument_count);
    } else {
        message->arguments = NULL;
        message->argument_capacities = NULL;
    }
    int i;
    for (i = 0; i < message->argument_count; ++i) {
        message->argument_capacities[i] = 10;
        message->arguments[i] = malloc (sizeof (char) * (message->argument_capacities[i] + 1));
        message->arguments[i][0] = '\0';
    }

    return message;
}


void scim_bridge_free_message (ScimBridgeMessage *message)
{
    if (message == NULL) return;
    
    free (message->header);

    int i;
    for (i = 0; i < message->argument_count; ++i) {
        free (message->arguments[i]);
    }

    if (message->argument_capacities) free (message->argument_capacities);
    if (message->arguments) free (message->arguments);
    free (message);
}


const char *scim_bridge_message_get_header (const ScimBridgeMessage *message)
{
    return message->header;
}


size_t scim_bridge_message_get_argument_count (const ScimBridgeMessage *message)
{
    return message->argument_count;
}


const char *scim_bridge_message_get_argument (const ScimBridgeMessage *message, size_t index)
{
    if (message == NULL) {
        scim_bridge_perrorln ("The pointer given as a message is NULL");
        return NULL;
    }

    if (index >= message->argument_count) {
        scim_bridge_perrorln ("An out of bounds exception occurred at scim_bridge_message_get_argument ()");
        return NULL;
    } else {
        return message->arguments[index];
    }
}


retval_t scim_bridge_message_set_argument (ScimBridgeMessage *message, size_t index, const char *argument)
{
    if (argument == NULL) {
        scim_bridge_perrorln ("The pointer given as an argument is NULL");
        return RETVAL_FAILED;
    }

    if (message == NULL) {
        scim_bridge_perrorln ("The pointer given as a message is NULL");
        return RETVAL_FAILED;
    }

    if (index >= message->argument_count) {
        scim_bridge_perrorln ("An out of bounds exception occurred at scim_bridge_message_set_argument ()");
        return RETVAL_FAILED;
    } else {
        const size_t argument_length = strlen (argument);
        if (argument_length > message->argument_capacities[index]) {
            const size_t new_argument_capacity = argument_length;
            free (message->arguments[index]);
            message->arguments[index] = malloc (sizeof (char) * (new_argument_capacity + 1));
            message->argument_capacities[index] = new_argument_capacity;
        }
        strcpy (message->arguments[index], argument);
        return RETVAL_SUCCEEDED;
    }
}
