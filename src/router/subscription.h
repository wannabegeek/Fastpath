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
        using hasher = std::hash<std::string>;

        typedef enum {
            STANDARD,
            ELEMENT,
            COMPLETION
        } component_type;

        struct component {
            hasher::result_type component_hash;
            component_type type;

            component(const hasher::result_type &h, const component_type &t) : component_hash(h), type(t) {}
            component(component &&other) : component_hash(other.component_hash), type(other.type) {}
        };

        std::vector<component> m_components;
        bool m_contains_wildcard;
        hasher::result_type m_subject_hash;

        void split(const std::string &subject, std::vector<std::string> &out) const;
        bool contains_wildcard(const std::string &subject) const;
    public:
        static constexpr char wildcard_completion = '>';
        static constexpr char wildcard_element = '*';

        subscription(const std::string &subject);
        ~subscription();

        bool matches(const std::string &subject) const;
    };
}

#endif //TFDCF_SUBSCRIPTION_H
