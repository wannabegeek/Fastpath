//
// Created by Tom Fewster on 02/04/2016.
//

#include <iostream>
#include "transport/UnixSocketClient.h"
#include <thread>
#include <utils/logger.h>
#include <mach/mach.h>
#include <cassert>

typedef struct {
    mach_msg_header_t header;
    int data;
} msg_format_send_t;

int main(int argc, char *argv[]) {

    LOG_LEVEL(tf::logger::debug);

    mach_port_t port;
    mach_port_t receive;
    int err;

    /* Allocate a port.  */
    err = mach_port_allocate (mach_task_self (), MACH_PORT_RIGHT_RECEIVE, &port);

    if (err) {
        ERROR_LOG("mach_port_allocate failed: " << err);
    }

    err = mach_port_allocate (mach_task_self (), MACH_PORT_RIGHT_RECEIVE, &receive);
    if (err) {
        ERROR_LOG("mach_port_allocate failed: " << err);
    }

    /* Send a message down the port */
//    while (1) {
        struct message message;

        message.data = "Blah blah blah";

        message.header.msgh_bits = MACH_MSGH_BITS (MACH_MSG_TYPE_MAKE_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
        message.header.msgh_remote_port = port;		/* Request port */
        message.header.msgh_local_port = receive;		/* Reply port */
        message.header.msgh_id = 0;			/* Message id */
        message.header.msgh_size = sizeof (message);	/* Message size */

        message.type.msgt_name = MACH_MSG_TYPE_STRING;	/* Parameter type */
        message.type.msgt_size = 8 * (strlen (message.data) + 1); /* # Bits */
        message.type.msgt_number = 1;			/* Number of elements */
        message.type.msgt_inline = TRUE;			/* Inlined */
        message.type.msgt_longform = FALSE;		/* Shortform */
        message.type.msgt_deallocate = FALSE;		/* Do not deallocate */
        message.type.msgt_unused = 0;			/* = 0 */

        /* Send the message on its way and wait for a reply.  */
        err = mach_msg (&message.header,			/* The header */
            MACH_SEND_MSG | MACH_RCV_MSG,	/* Flags */
            sizeof (message),			/* Send size */
            sizeof (message),			/* Max receive Size */
            receive,				/* Receive port */
            MACH_MSG_TIMEOUT_NONE,		/* No timeout */
            MACH_PORT_NULL);			/* No notification */
        if (err) {
            ERROR_LOG("mach_msg failed: " << err);
        }

        /* Verify the type tag */
        assert(message.type.msgt_name == MACH_MSG_TYPE_STRING);

        printf ("Server said: %s", message.data);
//    }

    return 0;
}