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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "scim-bridge-messenger.h"
#include "scim-bridge-output.h"

/* Type definition */
struct _ScimBridgeMessenger
{
    int socket_fd;

    char *sending_buffer;
    size_t sending_buffer_offset;
    size_t sending_buffer_size;
    size_t sending_buffer_capacity;

    char *receiving_buffer;
    size_t receiving_buffer_offset;
    size_t receiving_buffer_size;
    size_t receiving_buffer_capacity;

    boolean has_received_message;
};

/* Implementations */
ScimBridgeMessenger *scim_bridge_alloc_messenger (int socket_fd)
{
    scim_bridge_pdebugln (4, "scim_bridge_alloc_messenger ()");

    if (socket_fd < 0) {
        scim_bridge_perrorln ("An invalid file descriptor is given at scim_bridge_alloc_messenger ()");
        return NULL;
    }

    const int socket_flags = fcntl (socket_fd, F_GETFL);
    if (socket_flags < 0) {
        scim_bridge_perrorln ("Failed to get the flags of the socket");
        return NULL;
    }
    if (fcntl (socket_fd, F_SETFL, socket_flags | O_NONBLOCK)) {
        scim_bridge_perrorln ("Failed to set the flags of the socket");
        return NULL;
    }

    ScimBridgeMessenger *messenger = malloc (sizeof (ScimBridgeMessenger));
    messenger->socket_fd = socket_fd;

    messenger->sending_buffer_capacity = 20;
    messenger->sending_buffer = malloc (sizeof (char) * messenger->sending_buffer_capacity);

    messenger->sending_buffer_offset = 0;
    messenger->sending_buffer_size = 0;

    messenger->receiving_buffer_capacity = 20;
    messenger->receiving_buffer = malloc (sizeof (char) * messenger->receiving_buffer_capacity);

    messenger->receiving_buffer_offset = 0;
    messenger->receiving_buffer_size = 0;

    messenger->has_received_message = FALSE;

    return messenger;
}


void scim_bridge_free_messenger (ScimBridgeMessenger *messenger)
{
    scim_bridge_pdebugln (4, "scim_bridge_free_messenger ()");
    if (messenger == NULL) return;

    scim_bridge_close_messenger (messenger);

    free (messenger->sending_buffer);
    free (messenger->receiving_buffer);

    free (messenger);
}


retval_t scim_bridge_close_messenger (ScimBridgeMessenger *messenger)
{
    scim_bridge_pdebugln (4, "scim_bridge_close_messenger ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return RETVAL_FAILED;
    }

    if (messenger->socket_fd < 0) return RETVAL_SUCCEEDED;

    shutdown (messenger->socket_fd, SHUT_RDWR);
    close (messenger->socket_fd);
    messenger->socket_fd = -1;

    return RETVAL_SUCCEEDED;
}


boolean scim_bridge_messenger_is_closed (const ScimBridgeMessenger *messenger)
{
    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return FALSE;
    }

    return messenger->socket_fd < 0;
}


int scim_bridge_messenger_get_socket_fd (const ScimBridgeMessenger *messenger)
{
    scim_bridge_pdebugln (4, "scim_bridge_messenger_get_socket_fd ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return RETVAL_FAILED;
    }

    return messenger->socket_fd;
}


retval_t scim_bridge_messenger_push_message (ScimBridgeMessenger *messenger, const ScimBridgeMessage *message)
{
    scim_bridge_pdebugln (4, "scim_bridge_messenger_push_message ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return RETVAL_FAILED;
    }

    if (message == NULL) {
        scim_bridge_perrorln ("The pointer given as a message is NULL");
        return RETVAL_FAILED;
    }

    const ssize_t arg_count = (ssize_t) scim_bridge_message_get_argument_count (message);

    scim_bridge_pdebug (4, "message:");
    
    int i;
    for (i = -1; i < arg_count; ++i) {
        const char *str;
        if (i == -1) {
            str = scim_bridge_message_get_header (message);
        } else {
            str = scim_bridge_message_get_argument (message, i);
        }

        scim_bridge_pdebug (4, " %s", str);

        const size_t str_length = strlen (str);

        int j;
        for (j = 0; j <= str_length; ++j) {
            const size_t buffer_size = messenger->sending_buffer_size;

            size_t buffer_capacity = messenger->sending_buffer_capacity;
            size_t buffer_offset = messenger->sending_buffer_offset;

            if (buffer_size + 2 >= buffer_capacity) {
                const size_t new_buffer_capacity = buffer_capacity + 20;
                char *new_buffer = malloc (sizeof (char) * new_buffer_capacity);

                memcpy (new_buffer, messenger->sending_buffer + buffer_offset, sizeof (char) * (buffer_capacity - buffer_offset));
                memcpy (new_buffer + buffer_capacity - buffer_offset, messenger->sending_buffer, sizeof (char) * buffer_offset);
                free (messenger->sending_buffer);
                messenger->sending_buffer = new_buffer;

                buffer_capacity = new_buffer_capacity;
                messenger->sending_buffer_capacity = buffer_capacity;

                buffer_offset = 0;
                messenger->sending_buffer_offset = buffer_offset;
            }

            if (j < str_length) {
                switch (str[j]) {
                    case '\n':
                        messenger->sending_buffer[(buffer_offset + buffer_size) % buffer_capacity] = '\\';
                        messenger->sending_buffer[(buffer_offset + buffer_size + 1) % buffer_capacity] = 'n';
                        messenger->sending_buffer_size += 2;
                        break;
                    case ' ':
                        messenger->sending_buffer[(buffer_offset + buffer_size) % buffer_capacity] = '\\';
                        messenger->sending_buffer[(buffer_offset + buffer_size + 1) % buffer_capacity] = 's';
                        messenger->sending_buffer_size += 2;
                        break;
                    case '\\':
                        messenger->sending_buffer[(buffer_offset + buffer_size) % buffer_capacity] = '\\';
                        messenger->sending_buffer[(buffer_offset + buffer_size + 1) % buffer_capacity] = '\\';
                        messenger->sending_buffer_size += 2;
                        break;
                    default:
                        messenger->sending_buffer[(buffer_offset + buffer_size) % buffer_capacity] = str[j];
                        messenger->sending_buffer_size += 1;
                }
            } else {
                if (i + 1 == arg_count) {
                    messenger->sending_buffer[(buffer_offset + buffer_size) % buffer_capacity] = '\n';
                } else {
                    messenger->sending_buffer[(buffer_offset + buffer_size) % buffer_capacity] = ' ';
                }
                messenger->sending_buffer_size += 1;
            }
        }
    }
    scim_bridge_pdebug (4, "\n");

    return RETVAL_SUCCEEDED;
}


retval_t scim_bridge_messenger_poll_message (ScimBridgeMessenger *messenger, ScimBridgeMessage **message)
{
    scim_bridge_pdebugln (3, "scim_bridge_messenger_poll_message ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return RETVAL_FAILED;
    }

    if (message == NULL) {
        scim_bridge_perrorln ("The pointer given as a destination for a message is NULL");
        return RETVAL_FAILED;
    }

    if (!messenger->has_received_message) {
        scim_bridge_pdebugln (2, "No message to poll");
        return RETVAL_FAILED;
    }

    const size_t buffer_capacity = messenger->receiving_buffer_capacity;
    const size_t buffer_offset = messenger->receiving_buffer_offset;
    const size_t buffer_size = messenger->receiving_buffer_size;

    boolean escaped = FALSE;

    char *tmp_buffer = alloca (sizeof (char) * (buffer_size + 1));
    size_t tmp_buffer_size = 0;
    int arg_count = -1;
    int arg_capacity = 10;
    char **args = alloca (sizeof (char*) * arg_capacity);
    args[0] = tmp_buffer;

    int i;
    for (i = 0; i < buffer_size; ++i) {
        if (arg_count + 2 >= arg_capacity) {
            const int new_arg_capacity = arg_capacity + 10;
            char **new_args = alloca (sizeof (char*) * new_arg_capacity);
            memcpy (new_args, args, sizeof (char*) * arg_capacity);
            args = new_args;
            arg_capacity = new_arg_capacity;
        }

        const char c = messenger->receiving_buffer[(buffer_offset + i) % buffer_capacity];
        switch (c) {
            case '\\':
                if (escaped) {
                    tmp_buffer[tmp_buffer_size] = c;
                    ++tmp_buffer_size;
                    escaped = FALSE;
                } else {
                    escaped = TRUE;
                }
                break;
            case ' ':
            case '\n':
                escaped = FALSE;
                tmp_buffer[tmp_buffer_size] = '\0';
                ++tmp_buffer_size;
                ++arg_count;
                args[arg_count + 1] = tmp_buffer + i + 1;

                if (c == '\n') {
                    *message = scim_bridge_alloc_message (args[0], arg_count);

                    scim_bridge_pdebug (5, "message: %s", args[0]);
                    int j;
                    for (j = 0; j < arg_count; ++j) {
                        scim_bridge_pdebug (5, " %s", args[j + 1]);
                        scim_bridge_message_set_argument (*message, j, args[j + 1]);
                    }
                    scim_bridge_pdebug (5, "\n");

                    messenger->receiving_buffer_offset = (buffer_offset + i + 1) % buffer_capacity;
                    messenger->receiving_buffer_size -= i + 1;

                    return RETVAL_SUCCEEDED;
                }

                break;
            default:
                if (escaped) {
                    if (c == 'n') {
                        tmp_buffer[tmp_buffer_size] = '\n';
                    } else if (c == 's') {
                        tmp_buffer[tmp_buffer_size] = ' ';
                    } else {
                        tmp_buffer[tmp_buffer_size] = c;
                    }
                } else {
                    tmp_buffer[tmp_buffer_size] = c;
                }
                ++tmp_buffer_size;
                escaped = FALSE;
        }
    }


    scim_bridge_pdebugln (2, "The message is not completed");
    messenger->has_received_message = FALSE;
    return RETVAL_FAILED;
}


retval_t scim_bridge_messenger_send_message (ScimBridgeMessenger *messenger, const struct timeval *timeout)
{
    scim_bridge_pdebugln (3, "scim_bridge_messenger_send_message ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return RETVAL_FAILED;
    }

    const size_t buffer_capacity = messenger->sending_buffer_capacity;
    const size_t buffer_size = messenger->sending_buffer_size;
    const size_t buffer_offset = messenger->sending_buffer_offset;

    if (buffer_size == 0) return RETVAL_SUCCEEDED;

    size_t write_size;
    if (buffer_offset + buffer_size > buffer_capacity) {
        write_size = buffer_capacity - buffer_offset;
    } else {
        write_size = buffer_size;
    }

    const int fd = messenger->socket_fd;
    if (fd < 0) {
        scim_bridge_perrorln ("The socket is broken at scim_bridge_messenger_send_message ()");
        return RETVAL_FAILED;
    }

    fd_set select_set;
    FD_ZERO (&select_set);
    FD_SET (fd, &select_set);

    if (timeout != NULL) {
        struct timeval polling_timeout;
        polling_timeout.tv_sec = timeout->tv_sec;
        polling_timeout.tv_usec = timeout->tv_usec;

        if (select (fd + 1, NULL, &select_set, &select_set, &polling_timeout) < 0) {
            if (errno == EINTR) {
                scim_bridge_pdebugln (2, "An interruption occurred at scim_bridge_messenger_send_message ()");
                return RETVAL_SUCCEEDED;
            } else {
                scim_bridge_perrorln ("An IOException occurred at scim_bridge_messenger_send_message ()");
                return RETVAL_FAILED;
            }
        }
    } else {
        if (select (fd + 1, NULL, &select_set, &select_set, NULL) < 0) {
            if (errno == EINTR) {
                scim_bridge_pdebugln (2, "An interruption occurred at scim_bridge_messenger_send_message ()");
                return RETVAL_SUCCEEDED;
            } else {
                scim_bridge_perrorln ("An IOException occurred at scim_bridge_messenger_send_message ()");
                return RETVAL_FAILED;
            }
        }
    }

    ssize_t written_bytes = send (fd, messenger->sending_buffer + buffer_offset, sizeof (char) * write_size, MSG_NOSIGNAL);

    if (written_bytes < 0) {

        if (errno == EINTR || errno == EAGAIN) {
            scim_bridge_pdebugln (2, "Cannot send for now at scim_bridge_messenger_send_message ()");
            return RETVAL_SUCCEEDED;
        } else {
            scim_bridge_perrorln ("An IOException at scim_bridge_messenger_send_message (): %s", errno != 0 ? strerror (errno):"Unknown reason");
            return RETVAL_FAILED;
        }

    } else {

        scim_bridge_pdebugln (1, "offset = %d, size = %d + %d (%d), capacity = %d", buffer_offset, buffer_size, written_bytes / sizeof (char), write_size, buffer_capacity);

        char *tmp_str = alloca (written_bytes + sizeof (char));
        memcpy (tmp_str, messenger->sending_buffer + buffer_offset, written_bytes);
        tmp_str[written_bytes / sizeof (char)] = '\0';
        scim_bridge_pdebugln (1, "<- %s", tmp_str);

        messenger->sending_buffer_size -= written_bytes / sizeof (char);
        messenger->sending_buffer_offset = (buffer_offset + written_bytes / sizeof (char)) % buffer_capacity;

        return RETVAL_SUCCEEDED;
    }
}


retval_t scim_bridge_messenger_receive_message (ScimBridgeMessenger *messenger, const struct timeval *timeout)
{
    scim_bridge_pdebugln (4, "scim_bridge_messenger_receive_message ()");

    const size_t buffer_size = messenger->receiving_buffer_size;
    size_t buffer_capacity = messenger->receiving_buffer_capacity;
    size_t buffer_offset = messenger->receiving_buffer_offset;

    if (buffer_size + 20 >= buffer_capacity) {
        const size_t new_buffer_capacity = buffer_capacity + 40;
        char *new_buffer = malloc (sizeof (char) * new_buffer_capacity);

        memcpy (new_buffer, messenger->receiving_buffer + buffer_offset, sizeof (char) * (buffer_capacity - buffer_offset));
        memcpy (new_buffer + buffer_capacity - buffer_offset, messenger->receiving_buffer, sizeof (char) * buffer_offset);
        free (messenger->receiving_buffer);
        messenger->receiving_buffer = new_buffer;

        buffer_capacity = new_buffer_capacity;
        messenger->receiving_buffer_capacity = buffer_capacity;

        buffer_offset = 0;
        messenger->receiving_buffer_offset = buffer_offset;
    }

    size_t read_size;
    if (buffer_offset + buffer_size < buffer_capacity) {
        read_size = buffer_capacity - (buffer_offset + buffer_size);
    } else {
        read_size = buffer_offset - (buffer_offset + buffer_size) % buffer_capacity;
    }

    const int fd = messenger->socket_fd;
    if (fd < 0) {
        scim_bridge_perrorln ("The socket is broken at scim_bridge_messenger_receive_message ()");
        return RETVAL_FAILED;
    }

    fd_set select_set;
    FD_ZERO (&select_set);
    FD_SET (fd, &select_set);

    if (timeout != NULL) {
        struct timeval polling_timeout;
        polling_timeout.tv_sec = timeout->tv_sec;
        polling_timeout.tv_usec = timeout->tv_usec;

        if (select (fd + 1, &select_set, NULL, &select_set, &polling_timeout) < 0) {
            if (errno == EINTR) {
                scim_bridge_pdebugln (2, "An interruption occurred at scim_bridge_messenger_receive_message ()");
                return RETVAL_SUCCEEDED;
            } else {
                scim_bridge_perrorln ("An IOException occurred at scim_bridge_messenger_receive_message ()");
                return RETVAL_FAILED;
            }
        }
    } else {
        if (select (fd + 1, &select_set, NULL, &select_set, NULL) < 0) {
            if (errno == EINTR) {
                scim_bridge_pdebugln (2, "An interruption occurred at scim_bridge_messenger_receive_message ()");
                return RETVAL_SUCCEEDED;
            } else {
                scim_bridge_perrorln ("An IOException occurred at scim_bridge_messenger_receive_message ()");
                return RETVAL_FAILED;
            }
        }
    }

    assert (read_size > 0);
    const ssize_t read_bytes = recv (fd, messenger->receiving_buffer + (buffer_offset + buffer_size) % buffer_capacity, sizeof (char) * read_size, 0);

    if (read_bytes == 0) {
        scim_bridge_pdebugln (9, "The socket is closed at scim_bridge_messenger_receive_message ()");
        return RETVAL_FAILED;
    } else if (read_bytes < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            scim_bridge_pdebugln (2, "Cannot read for now at scim_bridge_messenger_receive_message ()");
            return RETVAL_SUCCEEDED;
        } else {
            scim_bridge_perrorln ("An IOException at scim_bridge_messenger_receive_message (): %s", errno != 0 ? strerror (errno):"Unknown reason");
            return RETVAL_FAILED;
        }

    } else {

        scim_bridge_pdebugln (1, "offset = %d, size = %d + %d (%d), capacity = %d", buffer_offset, buffer_size, read_bytes / sizeof (char), read_size, buffer_capacity);

        char *tmp_str = alloca (read_bytes + sizeof (char));
        memcpy (tmp_str, messenger->receiving_buffer + (buffer_offset + buffer_size) % buffer_capacity, read_bytes);
        tmp_str[read_bytes / sizeof (char)] = '\0';
        scim_bridge_pdebugln (1, "-> %s", tmp_str);
        
        if (!messenger->has_received_message) {
            int i;
            for (i = 0; i < read_bytes / sizeof (char); ++i) {
                char c = messenger->receiving_buffer[(buffer_offset + buffer_size + i) % buffer_capacity];
                if (c == '\n') {
                    scim_bridge_pdebugln (3, "A message has arrived");
                    messenger->has_received_message = TRUE;
                    break;
                }
            }
        }

        messenger->receiving_buffer_size += read_bytes / sizeof (char);
        return RETVAL_SUCCEEDED;
    }
}


ssize_t scim_bridge_messenger_get_sending_buffer_size (const ScimBridgeMessenger *messenger)
{
    scim_bridge_pdebugln (3, "scim_bridge_messenger_get_sending_buffer_size ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return -1;
    }

    scim_bridge_pdebugln (2, "The sending buffer size: %d", messenger->sending_buffer_size);
    return messenger->sending_buffer_size;
}


ssize_t scim_bridge_messenger_get_receiving_buffer_size (const ScimBridgeMessenger *messenger)
{
    scim_bridge_pdebugln (3, "scim_bridge_messenger_get_receiving_buffer_size ()");

    if (messenger == NULL) {
        scim_bridge_perrorln ("The pointer given as a messenger is NULL");
        return -1;
    }

    scim_bridge_pdebugln (2, "The receiving buffer size: %d", messenger->receiving_buffer_size);
    return messenger->receiving_buffer_size;
}
