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

#ifndef TFDCF_SIMPLESTRINGHASHER_H
#define TFDCF_SIMPLESTRINGHASHER_H

#include <cstddef>

namespace tf {
    struct string_comparator {
        bool operator()(const char *s1, const char *s2) const {
            return strcmp(s1, s2) == 0;
        }
    };

    struct string_hash {
        using result_type = size_t;

        result_type operator()(const char *s) const noexcept {
            size_t result = 0;
            const size_t prime = 31;

            size_t i = 0;
            while (s[i] != '\0') {
                result = s[i] + (result * prime);
                i++;
            }

            return result;
        }

        result_type operator()(const char *s, const size_t length) const noexcept {
            size_t result = 0;
            const size_t prime = 31;

            size_t i = 0;
            while (i < length) {
                result = s[i] + (result * prime);
                i++;
            }

            return result;
        }
    };
}

#endif //TFDCF_SIMPLESTRINGHASHER_H
