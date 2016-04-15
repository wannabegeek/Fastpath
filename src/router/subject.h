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

#ifndef TFDCF_SUBJECT_H
#define TFDCF_SUBJECT_H

#include "stringhash.h"

#include <vector>
#include <cstring>
#include <algorithm>

namespace fp {

    template <typename H = StringHash> class subject {
    private:
        using hasher = H;

        constexpr typename hasher::result_type admin_identifier() { return hasher()("_FP"); }

        std::vector<typename hasher::result_type> m_components;
        typename hasher::result_type m_subject_hash;
        bool m_is_admin = false;

        static void hash_elements(const char *subject, std::vector<typename hasher::result_type> &out) noexcept {
            const char delimiter = '.';
            const size_t len = strlen(subject);
            char *start_ptr = const_cast<char *>(&subject[0]);

            for (size_t i = 0; i < len; i++) {
                if (subject[i] == delimiter) {
                    const long l = std::distance(static_cast<const char *>(start_ptr), &subject[i]);
                    out.emplace_back(hasher()(start_ptr, l));
                    std::advance(start_ptr, l + 1);
                }
            }
            const long l = std::distance(static_cast<const char *>(start_ptr), &subject[len]);
            out.emplace_back(hasher()(start_ptr, l));
        }
    public:

        subject(const char *subject) noexcept {
            m_components.reserve(50);
            hash_elements(subject, m_components);
            m_subject_hash = hasher()(subject);
            m_is_admin = (m_components[0] == admin_identifier());
        }

        const bool is_admin() const noexcept {
            return m_is_admin;
        }

        const bool operator==(const subject<H> &other) const noexcept {
            return m_subject_hash == other.m_subject_hash;
        }

        template <typename T> friend class subscription;
    };
}

#endif //TFDCF_SUBJECT_H
