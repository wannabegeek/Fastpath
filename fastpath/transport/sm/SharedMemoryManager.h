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

#include "fastpath/SharedMemoryBuffer.h"

namespace fp {

    template<class T, class SegmentManager> class deleter {
    public:
        typedef typename boost::intrusive::pointer_traits<typename SegmentManager::void_pointer>::template rebind_pointer<T>::type pointer;

    private:
        typedef typename boost::intrusive::pointer_traits<pointer>::template rebind_pointer<SegmentManager>::type segment_manager_pointer;

        segment_manager_pointer mp_mngr;

    public:
        deleter(segment_manager_pointer pmngr) : mp_mngr(pmngr) {}

        void operator()(const pointer &p) {
            T *raw = boost::interprocess::ipcdetail::to_raw_pointer(p);
            mp_mngr->deallocate(reinterpret_cast<void *>(const_cast<byte *>(raw->_ptr.get())));
            mp_mngr->destroy_ptr(raw);
        }
    };

    struct BufferContainer {

        boost::interprocess::offset_ptr<const byte> _ptr;
        std::size_t _length;

        BufferContainer(boost::interprocess::offset_ptr<const byte> &&ptr, const std::size_t length) noexcept : _ptr(std::move(ptr)), _length(length) {}

        ~BufferContainer() {
        }
    };

    class SharedMemoryManager {
    public:
        typedef deleter<BufferContainer, boost::interprocess::managed_shared_memory::segment_manager> deleter_type;
        typedef boost::interprocess::allocator<BufferContainer, boost::interprocess::managed_shared_memory::segment_manager> buffer_allocator_type;
        typedef boost::interprocess::shared_ptr<BufferContainer, buffer_allocator_type, deleter_type> shared_ptr_type;

        typedef tf::shm_allocator<boost::interprocess::managed_shared_memory::segment_manager> inner_allocator_type;
        typedef tf::generic_allocator<unsigned char> allocator_type;

    private:
        boost::interprocess::managed_shared_memory m_segment;

        inner_allocator_type m_inner_allocator;
        allocator_type m_allocator;

        const std::string m_name;

    public:
        SharedMemoryManager(const char *name, const size_t initial_size = 1024 * 1024 * 2)
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

        shared_ptr_type getInterprocessBuffer(ByteStorage<byte, SharedMemoryManager::allocator_type> *buffer) noexcept {
            const byte *d = nullptr;
            const std::size_t length = buffer->bytes(&d);

            BufferContainer *ptr_container = m_segment.construct<BufferContainer>(boost::interprocess::anonymous_instance)(boost::interprocess::offset_ptr<const byte>(d), length);

            shared_ptr_type data = shared_ptr_type(ptr_container, buffer_allocator_type(m_segment.get_segment_manager()), deleter_type(m_segment.get_segment_manager()));
            assert(data.use_count() == 1);

            buffer->release_ownership();

            return data;
        }

    };
}

#endif //FASTPATH_SHAREDMEMORYMANAGER_H
