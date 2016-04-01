//
// Created by Tom Fewster on 31/03/2016.
//

#ifndef TFDCF_REF_COUNT_PTR_H
#define TFDCF_REF_COUNT_PTR_H

#include <functional>

namespace tf {
    template <typename T> class ref_count_ptr {
    public:
        typedef T element_type;
    private:
        element_type *m_ptr;

        std::atomic<uint32_t> m_ref_counter = ATOMIC_VAR_INIT(1);
        std::function<void(element_type *)> m_deleter;
    public:
        ref_count_ptr(T *value, std::function<void(element_type *)> deleter = std::default_delete<element_type>()) noexcept : m_ptr(value) {}

        void retain() noexcept {
            ++m_ref_counter;
        }

        void release() noexcept {
            if (m_ref_counter.fetch_sub(1) == 0) {
                m_deleter(m_ptr);
            }
        }

        element_type *get() const noexcept {
            return m_ptr;
        }

        element_type* operator->() const noexcept {
            return m_ptr;
        }
    };

}

#endif //TFDCF_REF_COUNT_PTR_H
