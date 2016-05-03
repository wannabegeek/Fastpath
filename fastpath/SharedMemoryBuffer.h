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
#include <fastpath/utils/allocator/generic_allocator.h>
#include <fastpath/transport/sm/SharedMemoryManager.h>

#pragma GCC diagnostic pop

#include "fastpath/transport/sm/shm_allocator.h"
#include "types.h"

namespace fp {
    class SharedMemoryBuffer {
    public:
        typedef MutableByteStorage<byte, SharedMemoryManager::allocator_type> mutable_storage_type;
        typedef ByteStorage<byte, SharedMemoryManager::allocator_type> storage_type;

    private:
        //    typedef boost::interprocess::managed_shared_memory::handle_t SharedPtrType;
        // This is only used to create the deque in shared memory

        typedef std::pair<ptrdiff_t, std::size_t> txfr_type;

        typedef boost::interprocess::allocator<txfr_type, boost::interprocess::managed_shared_memory::segment_manager> SmbAllocatorType;
        typedef boost::interprocess::deque<txfr_type, SmbAllocatorType> ObjPtrListType;

        typedef boost::interprocess::named_upgradable_mutex MutexType;

        const std::string m_name;

        mutable MutexType m_mutex;
        SmbAllocatorType m_deque_allocator;
        ObjPtrListType *m_objectList;

        SharedMemoryManager &m_sharedMemoryManager;

    public:
        SharedMemoryBuffer(const char *name, SharedMemoryManager &sharedMemoryManager) :
                m_name(name),
                m_mutex(boost::interprocess::open_or_create, (m_name + "_mutex").c_str()),
                m_deque_allocator(sharedMemoryManager.segment().get_segment_manager()),
                m_sharedMemoryManager(sharedMemoryManager) {
            m_objectList = m_sharedMemoryManager.segment().find_or_construct<ObjPtrListType>("InboundList")(m_deque_allocator);
        }

        ~SharedMemoryBuffer() {
            m_sharedMemoryManager.segment().destroy<ObjPtrListType>(m_name.c_str());
            boost::interprocess::named_mutex::remove((m_name + "_mutex").c_str());
        }

        void notify(storage_type *buffer) {
            //        boost::interprocess::scoped_lock<MutexType> lock(m_mutex);
            const byte *d = nullptr;
            const std::size_t length = buffer->bytes(&d);
            const void *ptr = d;
            txfr_type data = std::make_pair(m_sharedMemoryManager.segment().get_handle_from_address(ptr), length);
            buffer->release_ownership();
            m_objectList->push_back(data);
        }

        const size_t size() const {
            //        boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
            return m_objectList->size();
        }

//        storage_type try_retrieve() {
//            std::cout << "Size: " << this->size() << std::endl;
//            if (this->size()) {
//                return this->retrieve();
//            }
//            return storage_type(nullptr, 0, ByteStorage<byte>::TAKE_OWNERSHIP, m_sharedMemoryManager.allocator());
//        }

        void retrieve(std::function<void(const storage_type &)> callback) {
            //        boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
            while (!m_objectList->empty()) {
                txfr_type ptr = m_objectList->front();
                byte *data = reinterpret_cast<byte *>(m_sharedMemoryManager.segment().get_address_from_handle(ptr.first));
                std::size_t len = ptr.second;
                m_objectList->pop_front();

                callback(storage_type(data, len, fp::ByteStorage<byte>::TAKE_OWNERSHIP, m_sharedMemoryManager.allocator()));
            }
        }
    };
}

#endif //SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H
