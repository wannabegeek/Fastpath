/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef FASTPATH_STATUS_H
#define FASTPATH_STATUS_H

namespace fp {

    static constexpr const char *str_status[20] = {
            "Everything is OK and successful",
            "The event manager is not running",
            "There has been a severe internal error",
            "Message cannot be sent on selected transport",
            "Object cannot be created",
            "Object cannot be destroyed, maybe invalid",
            "Cannot connect to daemon",
            "Object not valid",
            "Service not valid for transport",
            "Daemon not valid for transport",
            "Message invalid and cannot be constructed",
            "The subject is not valid for this message",
            "The queue is invalid",
            "The transport is invalid",
            "The callack is invalid",
            "Field not found in message",
            "Requested conversion is illegal",
            "Cannot requested update",
            "Request timed out"
    };

    typedef enum {
        OK = 0,

        EVM_NOTRUNNING,
        NO_EVENTS,

        INTERNAL_ERROR,
        CANNOT_SEND,
        ALREADY_ACTIVE,
        NOT_ACTIVE,
        CANNOT_CREATE,
        CANNOT_DESTROY,
        CANNOT_CONNECT,

        NOT_VALID,

        INVALID_SERVICE,
        INVALID_DAEMON,

        INVALID_MSG,
        INVALID_SUBJECT,
        INVALID_NAME,
        INVALID_QUEUE,
        INVALID_TRANSPORT,
        INVALID_TRANSPORT_STATE,

        NOT_FOUND,
        CONVERSION_FAILED,
        UPDATE_FAILED,

        TIMEOUT = 19
    } status;

}

#endif //FASTPATH_STATUS_H
