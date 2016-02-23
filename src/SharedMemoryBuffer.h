//
// Created by fewstert on 22/02/16.
//

#ifndef SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H
#define SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H


#include <stddef.h>
#include <atomic>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

class SharedMemoryBuffer {
private:
    typedef boost::interprocess::managed_shared_memory::handle_t SharedPtrType;
    typedef boost::interprocess::allocator<SharedPtrType, boost::interprocess::managed_shared_memory::segment_manager> SmbAllocatorType;
    typedef boost::interprocess::deque<SharedPtrType, SmbAllocatorType> ObjPtrListType;
    typedef boost::interprocess::named_upgradable_mutex MutexType;

    boost::interprocess::managed_shared_memory m_segment;
    mutable MutexType m_mutex;
    ObjPtrListType *m_objectList;

    SmbAllocatorType m_allocator;

    const std::string m_name;

public:
    SharedMemoryBuffer(const char *name, const size_t initial_size = 65536) : m_segment(boost::interprocess::open_or_create, name, initial_size),
                                                                              m_mutex(boost::interprocess::open_or_create, "communication_named_mutex"),
                                                                              m_allocator(m_segment.get_segment_manager()), m_name(name) {
        m_objectList = m_segment.find_or_construct<ObjPtrListType>("ObjectList")(m_allocator);
    }

    ~SharedMemoryBuffer() {
        m_segment.destroy<ObjPtrListType>("ObjectList");
        boost::interprocess::named_mutex::remove("communication_named_mutex");
        boost::interprocess::shared_memory_object::remove(m_name.c_str());
    }

    void *allocate(const size_t size) {
        return m_segment.allocate(size);
    }

    void notify(void *ptr) {
        std::cout << "Buffer is now: " << m_objectList->size() << std::endl;
        boost::interprocess::scoped_lock<MutexType> lock(m_mutex);
        m_objectList->push_back(m_segment.get_handle_from_address(ptr));
    }

    const size_t size() const {
        boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
        return m_objectList->size();
    }

    void *try_retrieve() {
        std::cout << "Size: " << this->size() << std::endl;
        if (this->size()) {
            return this->retrieve();
        }
        return nullptr;
    }

    void *retrieve() {
        boost::interprocess::sharable_lock<MutexType> lock(m_mutex);
        SharedPtrType ptr = m_objectList->front();
        m_objectList->pop_front();
        return m_segment.get_address_from_handle(ptr);
    }

    void deallocate(void *ptr) {
        m_segment.deallocate(ptr);
    }
};


#endif //SHAREDMEMORYSENDER_SHAREDMEMORYBUFFER_H