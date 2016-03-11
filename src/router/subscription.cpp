//
// Created by Tom Fewster on 11/03/2016.
//

#include "subscription.h"
#include <functional>

namespace fp {
    subscription::subscription(const std::string &subject) {
        if (contains_wildcard(subject)) {
            this->split(subject, m_components);
        } else {
            m_subject_hash = std::hash<std::string>()(subject);
        }
    }

    subscription::~subscription() {

    }

    bool subscription::matches(const std::string &subject) const {
        if (!m_contains_wildcard) {
            return m_subject_hash == std::hash<std::string>()(subject);
        } else {
            std::vector<std::string> out;
            this->split(subject, out);
            std::for_each(out.begin(), out.end(), [&](const std::string &component) {

            });
        }

        return false;
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
        return subject.find(wildcard_end) || subject.find(wildcard_element);
    }
}