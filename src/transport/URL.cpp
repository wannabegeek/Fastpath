//
// Created by Tom Fewster on 04/03/2016.
//

#include "URL.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <functional>

namespace DCF {
    url::url(const std::string& url_s) {
        const std::string prot_end("://");
        std::string::const_iterator prot_i = std::search(url_s.begin(), url_s.end(), prot_end.begin(), prot_end.end());
        m_protocol.reserve(std::distance(url_s.begin(), prot_i));
        std::transform(url_s.begin(), prot_i, std::back_inserter(m_protocol), std::ptr_fun<int,int>(tolower)); // protocol is icase
        if (prot_i == url_s.end()) {
            return;
        }
        std::advance(prot_i, prot_end.length());
        std::string::const_iterator path_i = std::find(prot_i, url_s.end(), '/');
        m_host.reserve(std::distance(prot_i, path_i));
        std::transform(prot_i, path_i, std::back_inserter(m_host), std::ptr_fun<int,int>(tolower)); // host is icase
        std::string::const_iterator query_i = std::find(path_i, url_s.end(), '?');
        m_path.assign(path_i, query_i);
        if (query_i != url_s.end()) {
            ++query_i;
        }
        m_query.assign(query_i, url_s.end());
    }

    const bool url::operator==(const url &other) const {
        return m_protocol == other.m_protocol
                && m_host == other.m_host
                && m_port == other.m_port
                && m_path == other.m_path
                && m_query == other.m_query;
    }

}