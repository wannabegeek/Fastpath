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

#ifndef FASTPATH_STORAGETYPES_H
#define FASTPATH_STORAGETYPES_H

#include <cstdint>

namespace fp {
    typedef enum {
        unknown,
        string,
        date_time,
        boolean,
        uint8,
        uint16,
        uint32,
        uint64,
        int8,
        int16,
        int32,
        int64,
//        float16,
        float32,
        float64,
        data,
        message
    } storage_type;

    static constexpr const char *StorageTypeDescription[16] = {
            "unknown",
            "string",
            "date_time",
            "boolean",
            "uint8",
            "uint16",
            "uint32",
            "uint64",
            "int8",
            "int16",
            "int32",
            "int64",
//        float16,
            "float32",
            "float64",
            "data",
            "message"
    };

    typedef enum {
        standard = 0,
        administration = 1 << 0,
        guaranteed = 1 << 1
    } message_flags;

    class Message;
}

#endif //FASTPATH_STORAGETYPES_H
