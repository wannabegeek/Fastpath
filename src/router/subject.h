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

        std::vector<typename hasher::result_type> m_components;
        typename hasher::result_type m_subject_hash;

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
            hash_elements(subject, m_components);
            m_subject_hash = hasher()(subject);
        }

        template <typename T> friend class subscription;
    };
}

#endif //TFDCF_SUBJECT_H
