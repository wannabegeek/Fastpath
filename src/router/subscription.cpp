//
// Created by Tom Fewster on 11/03/2016.
//

#include "subscription.h"
#include <functional>
#include <algorithm>
#include <ostream>
#include <iostream>

namespace fp {
    subscription::subscription(const std::string &subject) {
        m_contains_wildcard = contains_wildcard(subject);
        if (m_contains_wildcard) {
            std::vector<std::string> out;
            this->split(subject, out);
            std::for_each(out.begin(), out.end(), [&](const std::string &c) {
                hasher::result_type h = hasher()(c);
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

    subscription::~subscription() {

    }

    bool subscription::matches(const std::string &subject) const {
        bool matches = false;
        if (!m_contains_wildcard) {
            matches = (m_subject_hash == hasher()(subject));
        } else {
            std::vector<std::string> out;
            this->split(subject, out);
            if (out.size() >= m_components.size()) {
                size_t index = 0;
                auto it = std::find_if_not(m_components.begin(), m_components.end(), [&](const component &component) {
                    bool result = false;
                    switch (component.type) {
                        case STANDARD:
                            result = component.component_hash == hasher()(out[index]);
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

    void subscription::split(const std::string &subject, std::vector<std::string> &out) const {
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

    bool subscription::contains_wildcard(const std::string &subject) const {
        return subject.find(wildcard_completion) != std::string::npos || subject.find(wildcard_element) != std::string::npos;
    }

    const bool subscription::operator==(const subscription &other) const {
        return m_subject_hash == other.m_subject_hash;
    }

    const bool subscription::operator==(const std::string &other) const {
        return m_subject_hash == hasher()(other);
    }
}