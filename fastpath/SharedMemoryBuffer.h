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
#include <boost/interprocess/sync/upgradable_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <fastpath/utils/allocator/generic_allocator.h>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#pragma GCC diagnostic pop

#include "fastpath/transport/sm/SharedMemoryManager.h"
#include "fastpath/transport/sm/shm_allocator.h"
#include "types.h"

namespace fp {

    class SharedMemoryBuffer {
    public:
        typedef MutableByteStorage<byte, SharedMemoryManager::allocator_type> mutable_storage_type;
        typedef ByteStorage<byte, SharedMemoryManager::allocator_type> storage_type;

    private:

        typedef boost::interprocess::allocator<SharedMemoryManager::shared_ptr_type, boost::interprocess::managed_shared_memory::segment_manager> SmbAllocatorType;
        typedef boost::interprocess::deque<SharedMemoryManager::shared_ptr_type, SmbAllocatorType> ObjPtrListType;

        typedef boost::interprocess::named_upgradable_mutex MutexType;

        const std::string m_name;

        mutable MutexType m_mutex;
        SmbAllocatorType m_deque_allocator;
        ObjPtrListType *m_objectList;

        SharedMemoryManager &m_sharedMemoryManager;

        bool m_shared_queue;

    public:
        SharedMemoryBuffer(const char *name, SharedMemoryManager &sharedMemoryManager, bool shared_queue = false) :
                m_name(name),
                m_mutex(boost::interprocess::open_or_create, (m_name + "_mutex").c_str()),
                m_deque_allocator(sharedMemoryManager.segment().get_segment_manager()),
                m_sharedMemoryManager(sharedMemoryManager),
                m_shared_queue(shared_queue) {

            m_objectList = m_sharedMemoryManager.segment().find_or_construct<ObjPtrListType>(m_name.c_str())(m_deque_allocator);
        }

        ~SharedMemoryBuffer() {
            if (!m_shared_queue) {
                DEBUG_LOG("Removing mutex and queue for " << m_name);
                boost::interprocess::named_mutex::remove((m_name + "_mutex").c_str());
                m_sharedMemoryManager.segment().destroy_ptr(m_objectList);
            }
        }

        void notify(SharedMemoryManager::shared_ptr_type &ptr) {
            boost::interprocess::scoped_lock<MutexType> lock(m_mutex);
            DEBUG_LOG("Forwarding " << ptr->_ptr << " count: " << ptr.use_count() << " to: " << m_name);
            m_objectList->push_back(ptr);
        }

        void notify(SharedMemoryManager::shared_ptr_type &&ptr) {
            boost::interprocess::scoped_lock<MutexType> lock(m_mutex);
            DEBUG_LOG("Forwarding move " << ptr->_ptr << " count: " << ptr.use_count() << " to: " << m_name);
            m_objectList->push_back(std::move(ptr));
        }

        const size_t size() const {
            boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
            return m_objectList->size();
        }

        std::size_t retrieve(std::function<void(const storage_type &, SharedMemoryManager::shared_ptr_type &)> callback) {
            std::size_t count = 0;
            boost::interprocess::upgradable_lock<MutexType> lock(m_mutex);
            while (!m_objectList->empty()) {
                SharedMemoryManager::shared_ptr_type &ptr = m_objectList->front();
                DEBUG_LOG("Recieved: " << ptr->_ptr << " count: " << ptr.use_count() << " from: " << m_name);
                const byte *data = ptr->_ptr.get();
                std::size_t len = ptr->_length;

                callback(storage_type(data, len, fp::ByteStorage<byte>::TRANSIENT, m_sharedMemoryManager.allocator()), ptr);

                boost::interprocess::scoped_lock<MutexType> slock(std::move(lock));
                m_objectList->pop_front();
                count++;
            }

            return count;
        }
    };
}

#endif //SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H
