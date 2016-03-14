//
// Created by Tom Fewster on 14/03/2016.
//

#ifndef TFDCF_SIMPLESTRINGHASHER_H
#define TFDCF_SIMPLESTRINGHASHER_H

#include <cstddef>

namespace fp {
    struct StringHash {
        using result_type = size_t;

        result_type operator()(const char *s) const {
            size_t result = 0;
            const size_t prime = 31;

            size_t i = 0;
            while (s[i] != '\0') {
                result = s[i] + (result * prime);
                i++;
            }

            return result;
        }

        result_type operator()(const char *s, const size_t length) const {
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
