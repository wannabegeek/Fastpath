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
