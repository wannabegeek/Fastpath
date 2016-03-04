//
// Created by Tom Fewster on 04/03/2016.
//

#ifndef TFDCF_URL_H
#define TFDCF_URL_H

#include <string>

namespace DCF {
    class url {
    private:
        std::string m_protocol;
        std::string m_host;
        std::string m_port;
        std::string m_path;
        std::string m_query;
    public:
        url(const std::string& url_s); // omitted copy, ==, accessors, ...

        url(const url &) = delete;
        const url &operator=(const url &) = delete;

        const bool operator==(const url &other) const;

        const std::string &protocol() const { return m_protocol; }
        const std::string &host() const { return m_host; }
        const std::string &port() const { return m_port; }
        const std::string &path() const { return m_path; }
        const std::string &query() const { return m_query; }
    };
}

#endif //TFDCF_URL_H
