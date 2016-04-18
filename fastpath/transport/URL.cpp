/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "fastpath/transport/URL.h"

#include <string>
#include <algorithm>
#include <cctype>
#include <ostream>
#include <functional>
#include <stdexcept>

namespace fp {
    auto find_separator = [](std::string::const_iterator begin, std::string::const_iterator end) {
        return std::find_if(begin, end, [](const char &v) {
            return v == '/' || v == ':' || v == '?';
        });
    };

    url::url(const std::string& url_s) {

        const std::string prot_end("://");
        std::string::const_iterator prot_i = std::search(url_s.begin(), url_s.end(), prot_end.begin(), prot_end.end());
        m_protocol.reserve(std::distance(url_s.begin(), prot_i));
        std::transform(url_s.begin(), prot_i, std::back_inserter(m_protocol), std::ptr_fun<int,int>(tolower)); // protocol is icase
        if (prot_i == url_s.end()) {
            throw std::invalid_argument("Invalid URL - no protocol");
        }
        std::advance(prot_i, prot_end.length());

        std::string::const_iterator host_end = find_separator(prot_i, url_s.end());
        m_host.reserve(std::distance(prot_i, host_end));
        std::transform(prot_i, host_end, std::back_inserter(m_host), std::ptr_fun<int,int>(tolower)); // host is icase

        std::string::const_iterator pos = host_end;

        while (pos != url_s.end()) {
            if (*pos == ':') {
                // we have a port
                std::advance(pos, 1);
                std::string::const_iterator port_end = find_separator(pos, url_s.end());
                std::copy(pos, port_end, std::back_inserter(m_port));
                pos = port_end;
            } else if (*pos == '/') {
                std::advance(pos, 1);
                std::string::const_iterator path_end = std::find(pos, url_s.end(), '?');
                std::copy(pos, path_end, std::back_inserter(m_path));
                pos = path_end;
            } else if (*pos == '?') {
                while (pos != url_s.end()) {
                    std::advance(pos, 1);
                    auto new_query = std::find(pos, url_s.end(), '&');
                    auto key_end = std::find(pos + 1, new_query, '=');
                    std::string value;
                    if (key_end != new_query) {
                        value = std::string(key_end + 1, new_query);
                    }
                    m_query.emplace(std::string(pos, key_end), value);
                    pos = new_query;
                }
                pos = url_s.end();
            } else {
                throw std::invalid_argument("Invalid URL");
                break;
            }
        }
    }

    url::url(const url &other) : m_protocol(other.m_protocol),
                                     m_host(other.m_host),
                                     m_port(other.m_port),
                                     m_path(other.m_path),
                                     m_query(other.m_query) {

    }

    const bool url::operator==(const url &other) const {
        return m_protocol == other.m_protocol
                && m_host == other.m_host
                && m_port == other.m_port
                && m_path == other.m_path
                && m_query == other.m_query;
    }

    std::ostream &operator<<(std::ostream &out, const url &url) {
        out << "Protocol: '" << url.m_protocol << "' Host: '" << url.m_host << "' Port: '" << url.m_port << "' Path: '" << url.m_path << "' Query: [";
        std::for_each(url.m_query.begin(), url.m_query.end(), [&](const url::query_type::value_type &pair) noexcept {
            out << pair.first << " => " << pair.second;
        });
        out << "]";

        return out;
    }
}