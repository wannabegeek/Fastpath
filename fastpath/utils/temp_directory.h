/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/05/2016

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


#ifndef FASTPATH_TEMP_DIRECTORY_H
#define FASTPATH_TEMP_DIRECTORY_H

#include <cstdlib>
#include <array>

namespace tf {
    const char *get_temp_directory() noexcept {
        static const std::array<const char *, 4> env_locations{
                "TMPDIR",
                "TMP",
                "TEMP",
                "TEMPDIR"
        };

        char *location = nullptr;
        for (auto &env: env_locations) {
            if ((location = ::getenv(env)) != nullptr) {
                return location;
            }
        }

        return "/tmp/";
    }
}

#endif //FASTPATH_TEMP_DIRECTORY_H
