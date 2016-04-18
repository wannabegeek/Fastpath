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

#ifndef FASTPATH_BLOCKING_RINGBUFFER_H
#define FASTPATH_BLOCKING_RINGBUFFER_H

#include "tfringbuffer.h"
#include "semaphore_light.h"
#include <type_traits>

namespace tf {
    template <typename T, size_t SIZE = 1024> class blocking_ringbuffer final : public ringbuffer<T, SIZE> {
        LightweightSemaphore m_semaphore;

    public:
        void pop_wait(T &object) {
            while (!this->pop(object)) {
                m_semaphore.wait();
            }
        }

        virtual bool push(T &&object) override {
            bool v = ringbuffer<T, SIZE>::push(std::forward<T>(object));
            m_semaphore.signal();
            return v;
        }

        virtual bool push(const T &object) override {
            bool v = ringbuffer<T, SIZE>::push(object);
            m_semaphore.signal();
            return v;
        }
    };
}

#endif //FASTPATH_BLOCKING_RINGBUFFER_H
