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

        EVM_NOTRUNNING = 1,

        INTERNAL_ERROR = 2,
        CANNOT_SEND = 3,
        ALREADY_ACTIVE = 4,
        CANNOT_DESTROY = 5,
        CANNOT_CONNECT = 6,
        NOT_VALID = 7,

        INVALID_SERVICE = 8,
        INVALID_DAEMON = 9,

        INVALID_MSG = 10,
        INVALID_SUBJECT = 11,
        INVALID_NAME = 12,
        INVALID_QUEUE = 13,
        INVALID_TRANSPORT = 14,
        INVALID_CALLBACK = 15,

        NOT_FOUND = 16,
        CONVERSION_FAILED = 17,
        UPDATE_FAILED = 18,

        TIMEOUT = 19
    } status;

}

#endif //TFDCF_STATUS_H
