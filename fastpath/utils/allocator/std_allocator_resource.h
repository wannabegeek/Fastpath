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

#ifndef FASTPATH_STD_ALLOCATOR_RESOURCE_H
#define FASTPATH_STD_ALLOCATOR_RESOURCE_H

#include <memory>
#include <cstddef>

#include "fastpath/utils/allocator/generic_allocator.h"

namespace tf {
    class std_allocator_resource final : public allocator_resource {
    private:
        typedef char value_type;
        typedef value_type* pointer;

        std::allocator<value_type> m_allocator;

    public:
        void *allocate(const std::size_t size) noexcept {
            return std::allocator_traits<std::allocator<value_type>>::allocate(m_allocator, size);
        }

        void deallocate(void *p, const std::size_t size) noexcept {
            std::allocator_traits<std::allocator<value_type>>::deallocate(m_allocator, reinterpret_cast<pointer>(p), size);
        }

    };

    ////////////////
    static allocator_resource *get_default_allocator() noexcept {
        static std::unique_ptr<allocator_resource> s_resource = std::make_unique<std_allocator_resource>();
        return s_resource.get();
    }
}
#endif //FASTPATH_STD_ALLOCATOR_RESOURCE_H
