/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 26/03/2016

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

#ifndef FASTPATH_SHM_ALLOCATOR_H
#define FASTPATH_SHM_ALLOCATOR_H

#include <cstddef>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/interprocess/managed_shared_memory.hpp>

#pragma GCC diagnostic pop

namespace tf {
    template <class SegmentManager> class shm_allocator : public tf::allocator_resource {
    private:
        SegmentManager *m_segment_manager;
    public:
        shm_allocator(SegmentManager *segment_manager) noexcept : m_segment_manager(segment_manager) {}

        ~shm_allocator() noexcept = default;

        shm_allocator(const shm_allocator &other) noexcept : m_segment_manager(other.m_segment_manager) {}

        inline void *allocate(const std::size_t size) noexcept {
            return m_segment_manager->allocate(size, std::nothrow);
        }

        inline void deallocate(void* p, std::size_t size) noexcept {
            m_segment_manager->deallocate(reinterpret_cast<void *>(p));
        }
    };
};

#endif //FASTPATH_SHM_ALLOCATOR_H
