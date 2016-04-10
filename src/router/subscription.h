//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_SUBSCRIPTION_H
#define TFDCF_SUBSCRIPTION_H

#include <string>
#include <vector>
#include "stringhash.h"
#include "subject.h"

namespace fp {
    template <typename H = StringHash> class subscription {
    private:
        using hasher = H;

        typedef enum {
            STANDARD,
            ELEMENT,
            COMPLETION
        } component_type;

        struct component {
            typename hasher::result_type component_hash;
            component_type type;

            component(const typename hasher::result_type &h, const component_type &t) noexcept : component_hash(h), type(t) {}
            component(const component &other) noexcept : component_hash(other.component_hash), type(other.type) {}
            component(component &&other) noexcept : component_hash(other.component_hash), type(other.type) {}

            component& operator=(const component& other) noexcept {
                component_hash = other.component_hash;
                type = other.type;

                return *this;
            }
        };

        std::vector<component> m_components;
        bool m_contains_wildcard;
        typename hasher::result_type m_subject_hash;

        void split(const std::string &subject, std::vector<std::string> &out) const noexcept {
            const char delimiter = '.';
            size_t start = 0;
            size_t end = subject.find_first_of(delimiter);

            while (end <= std::string::npos) {
                out.emplace_back(subject.substr(start, end - start));

                if (end == std::string::npos) {
                    break;
                }

                start = end + 1;
                end = subject.find_first_of(delimiter, start);
            }
        }

        bool contains_wildcard(const char *subject) const noexcept {
            const size_t len = strlen(subject);
            for (size_t i = 0; i < len; ++i) {
                switch(subject[i]) {
                    case wildcard_completion:
                    case wildcard_element:
                        return true;
                    default:
                        continue;
                }
            }
            return false;
        }
    public:
        static constexpr char wildcard_completion = '>';
        static constexpr char wildcard_element = '*';

        subscription(const char *subject) noexcept {
            m_contains_wildcard = contains_wildcard(subject);
            if (m_contains_wildcard) {
                std::vector<std::string> out;
                this->split(subject, out);
                std::for_each(out.begin(), out.end(), [&](const std::string &c) noexcept {
                    typename hasher::result_type h = hasher()(c.c_str());
                    switch (c[0]) {
                        case wildcard_completion:
                            m_components.emplace_back(h, COMPLETION);
                            break;
                        case wildcard_element:
                            m_components.emplace_back(h, ELEMENT);
                            break;
                        default:
                            m_components.emplace_back(h, STANDARD);
                            break;
                    }
                });
            }

            // always calculate this, we use it in operator==
            m_subject_hash = hasher()(subject);
        }

        subscription(subscription &&other) noexcept : m_components(std::move(other.m_components)), m_contains_wildcard(other.m_contains_wildcard), m_subject_hash(other.m_subject_hash) {}

        ~subscription() noexcept {};

        subscription& operator=(const subscription& other) noexcept {
            std::copy(other.m_components.begin(), other.m_components.end(), std::back_inserter(m_components));
            m_contains_wildcard = other.m_contains_wildcard;
            m_subject_hash = other.m_subject_hash;

            return *this;
        }

        bool matches(const subject<H> &subject) const noexcept {
            bool matches = false;
            if (!m_contains_wildcard) {
                matches = (m_subject_hash == subject.m_subject_hash);
            } else {
                if (subject.m_components.size() >= m_components.size()) {
                    size_t index = 0;
                    auto it = std::find_if_not(m_components.begin(), m_components.end(), [&](const component &component) noexcept {
                           bool result = false;
                           switch (component.type) {
                               case STANDARD:
                                   result = component.component_hash == subject.m_components[index];
                                   break;
                               case ELEMENT:
                               case COMPLETION:
                                   result = true;
                                   break;
                           }
                           ++index;
                           return result;
                       });

                    matches = (it == m_components.end());
                }
            }

            return matches;
        }

        const bool operator==(const subscription &other) const noexcept {
            return m_subject_hash == other.m_subject_hash;
        }

        const bool operator==(const char *other) const noexcept {
            return m_subject_hash == StringHash()(other);
        }
    };
}

#endif //TFDCF_SUBSCRIPTION_H
