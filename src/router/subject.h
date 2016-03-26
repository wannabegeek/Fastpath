//
// Created by Tom Fewster on 14/03/2016.
//

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

        subject(const char *subject) {
            m_components.reserve(50);
            hash_elements(subject, m_components);
            m_subject_hash = hasher()(subject);
            m_is_admin = (m_components[0] == admin_identifier());
        }

        const bool is_admin() const noexcept {
            return m_is_admin;
        }

        const bool operator==(const subject<H> &other) const {
            return m_subject_hash == other.m_subject_hash;
        }

        template <typename T> friend class subscription;
    };
}

#endif //TFDCF_SUBJECT_H
