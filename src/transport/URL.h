//
// Created by Tom Fewster on 04/03/2016.
//

#ifndef TFDCF_URL_H
#define TFDCF_URL_H

#include <string>
#include <unordered_map>

namespace DCF {
    class url {
    public:
        using query_type = std::unordered_map<std::string, std::string>;
    private:
        std::string m_protocol;
        std::string m_host;
        std::string m_port;
        std::string m_path;
        query_type m_query;
    public:
        url(const std::string& url_s);
        url(const url &);

        const url &operator=(const url &) = delete;

        const bool operator==(const url &other) const;

        const std::string &protocol() const { return m_protocol; }
        const std::string &host() const { return m_host; }
        const std::string &port() const { return m_port; }
        const std::string &path() const { return m_path; }
        const query_type &query() const { return m_query; }

        friend std::ostream &operator<<(std::ostream &out, const url &url);
    };
}

#endif //TFDCF_URL_H
