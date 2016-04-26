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

#ifndef SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H
#define SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H

#include "MutableByteStorage.h"

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

#include "playground/shm_allocator.h"
#include "types.h"

class SharedMemoryBuffer {
public:
    typedef tf::shm_allocator<unsigned char, boost::interprocess::managed_shared_memory::segment_manager> allocator_type;
    typedef fp::MutableByteStorage<byte, allocator_type> mutable_storage_type;
    typedef fp::ByteStorage<byte, allocator_type> storage_type;

private:
//    typedef boost::interprocess::managed_shared_memory::handle_t SharedPtrType;
    // This is only used to create the deque in shared memory

    typedef std::pair<ptrdiff_t, std::size_t> txfr_type;

    typedef boost::interprocess::allocator<txfr_type, boost::interprocess::managed_shared_memory::segment_manager> SmbAllocatorType;
    typedef boost::interprocess::deque<txfr_type, SmbAllocatorType> ObjPtrListType;

    typedef boost::interprocess::named_upgradable_mutex MutexType;


    boost::interprocess::managed_shared_memory m_segment;
    mutable MutexType m_mutex;
    ObjPtrListType *m_objectList;

    SmbAllocatorType m_deque_allocator;
    allocator_type m_allocator;

    const std::string m_name;

public:
    SharedMemoryBuffer(const char *name, const size_t initial_size = 65536) : m_segment(boost::interprocess::open_or_create, name, initial_size),
                                                                              m_mutex(boost::interprocess::open_or_create, "communication_named_mutex"),
                                                                              m_deque_allocator(m_segment.get_segment_manager()),
                                                                              m_allocator(m_segment.get_segment_manager()),
                                                                              m_name(name) {
        m_objectList = m_segment.find_or_construct<ObjPtrListType>("ObjectList")(m_deque_allocator);
    }

    ~SharedMemoryBuffer() {
        m_segment.destroy<ObjPtrListType>("ObjectList");
        boost::interprocess::named_mutex::remove("communication_named_mutex");
        boost::interprocess::shared_memory_object::remove(m_name.c_str());
    }

    allocator_type allocator() const noexcept {
        return m_allocator;
    }

    void notify(storage_type *buffer) {
        std::cout << "Buffer is now: " << m_objectList->size() << std::endl;
//        boost::interprocess::scoped_lock<MutexType> lock(m_mutex);
        const byte *d = nullptr;
        const std::size_t length = buffer->bytes(&d);
        const void *ptr = d;
        txfr_type data = std::make_pair(m_segment.get_handle_from_address(ptr), length);
        std::cout << "Sending " << data.second << " bytes of data in buffer at " << data.first << std::endl;
        m_objectList->push_back(data);
    }

    const size_t size() const {
        boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
        return m_objectList->size();
    }

    fp::ByteStorage<byte> try_retrieve() {
        std::cout << "Size: " << this->size() << std::endl;
        if (this->size()) {
            return this->retrieve();
        }
        return fp::ByteStorage<byte>(nullptr, 0);
    }

    fp::ByteStorage<byte> retrieve() {
//        boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
        txfr_type ptr = m_objectList->front();
        byte *data = reinterpret_cast<byte *>(m_segment.get_address_from_handle(ptr.first));
        std::size_t len = ptr.second;
        std::cout << "Received " << len << " bytes of data in buffer at " << ptr.first << std::endl;
        m_objectList->pop_front();
        return fp::ByteStorage<byte>(data, len); //m_segment.get_address_from_handle(ptr);
    }
};


#endif //SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H
