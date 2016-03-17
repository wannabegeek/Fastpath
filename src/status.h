//
// Created by Tom Fewster on 01/03/2016.
//

#ifndef TFDCF_STATUS_H
#define TFDCF_STATUS_H

namespace DCF {

//    const char *descriptions[20] = {
//            "Everything is OK and successful",
//            "The event manager is not running",
//            "There has been a severe internal error",
//            "Message cannot be sent on selected transport",
//            "Object cannot be created",
//            "Object cannot be destroyed, maybe invalid",
//            "Cannot connect to daemon",
//            "Object not valid",
//            "Service not valid for transport",
//            "Daemon not valid for transport",
//            "Message invalid and cannot be constructed",
//            "The subject is not valid for this message",
//            "The queue is invalid",
//            "The transport is invalid",
//            "The callack is invalid",
//            "Field not found in message",
//            "Requested conversion is illegal",
//            "Cannot requested update",
//            "Request timed out"
//    };

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

        NOT_FOUND,
        CONVERSION_FAILED,
        UPDATE_FAILED,

        TIMEOUT = 19
    } status;

}

#endif //TFDCF_STATUS_H
