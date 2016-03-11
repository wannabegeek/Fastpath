//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_SUBSCRIPTION_H
#define TFDCF_SUBSCRIPTION_H

#include <string>
#include <vector>

namespace fp {
    class subscription {
    private:
        std::vector<std::string> m_components;
        bool m_contains_wildcard;
        std::hash<std::string>::result_type m_subject_hash;

        void split(const std::string &subject, std::vector<std::string> &out) const;
        bool contains_wildcard(const std::string &subject) const;
    public:
        static constexpr char wildcard_end = '>';
        static constexpr char wildcard_element = '*';

        subscription(const std::string &subject);
        ~subscription();

        bool matches(const std::string &subject) const;
    };
}

#endif //TFDCF_SUBSCRIPTION_H
