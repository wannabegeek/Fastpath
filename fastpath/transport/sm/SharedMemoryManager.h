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

#ifndef FASTPATH_SHAREDMEMORYMANAGER_H
#define FASTPATH_SHAREDMEMORYMANAGER_H

#include <stddef.h>
#include <atomic>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#pragma GCC diagnostic pop

#include "shm_allocator.h"
#include "fastpath/types.h"
#include "fastpath/utils/allocator/generic_allocator.h"

namespace fp {
    class SharedMemoryManager {
    public:
        typedef tf::shm_allocator<boost::interprocess::managed_shared_memory::segment_manager> inner_allocator_type;
        typedef tf::generic_allocator<unsigned char> allocator_type;

    private:
        boost::interprocess::managed_shared_memory m_segment;

        inner_allocator_type m_inner_allocator;
        allocator_type m_allocator;

        const std::string m_name;

    public:
        SharedMemoryManager(const char *name, const size_t initial_size = 65536)
                : m_segment(boost::interprocess::open_or_create, name, initial_size),
                  m_inner_allocator(m_segment.get_segment_manager()),
                  m_allocator(&m_inner_allocator),
                  m_name(name) {
        }

        ~SharedMemoryManager() {
            boost::interprocess::shared_memory_object::remove(m_name.c_str());
        }

        inline allocator_type &allocator() noexcept {
            return m_allocator;
        }

        inline boost::interprocess::managed_shared_memory &segment() noexcept {
            return m_segment;
        }
    };
}

#endif //FASTPATH_SHAREDMEMORYMANAGER_H
