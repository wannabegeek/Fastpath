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

#ifndef TFDCF_STORAGETYPES_H
#define TFDCF_STORAGETYPES_H

#include <cstdint>

namespace DCF {
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
    } StorageType;

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
        Default = 0,
        Administration = 1 << 0,
        Garuanted = 1 << 1
    } MessageFlags;

    class Message;
}

#endif //TFDCF_STORAGETYPES_H
