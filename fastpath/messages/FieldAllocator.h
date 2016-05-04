//
// Created by Tom Fewster on 22/04/2016.
//

#ifndef FASTPATH_FIELDALLOCATOR_H
#define FASTPATH_FIELDALLOCATOR_H

#include <utility>

#include "fastpath/messages/Field.h"
#include "fastpath/messages/ScalarField.h"
#include "fastpath/messages/DataField.h"
#include "fastpath/messages/DateTimeField.h"
#include "fastpath/messages/SmallDataField.h"
#include "fastpath/messages/LargeDataField.h"
#include "fastpath/messages/MessageField.h"
#include "fastpath/utils/allocate_polymorphic.h"

namespace fp {

    template<class ...Args, typename Allocator>
    inline ScalarField *createScalarField(Allocator &allocator, Args &&...args) {
        return tf::allocate_polymorphic::allocate<ScalarField, Allocator>(allocator, std::forward<Args>(args)...);
    }

    template<class ...Args, typename Allocator>
    inline DataField *createDataField(Allocator &allocator, std::size_t size, Args &&...args) {
        if (tf::likely(size <= SmallDataField::max_size)) {
            return tf::allocate_polymorphic::allocate<SmallDataField, Allocator>(allocator, std::forward<Args>(args)...);
        } else {
            return tf::allocate_polymorphic::allocate<LargeDataField<Allocator>, Allocator>(allocator, std::forward<Args>(args)..., allocator);
        }
    }

    template<class ...Args, typename Allocator>
    inline DateTimeField *createDateTimeField(Allocator &allocator, Args &&...args) {
        return tf::allocate_polymorphic::allocate<DateTimeField, Allocator>(allocator, std::forward<Args>(args)...);
    }

    template<class ...Args, typename Allocator>
    inline MessageField *createMessageField(Allocator &allocator, Args &&...args) {
        return tf::allocate_polymorphic::allocate<MessageField, Allocator>(allocator, std::forward<Args>(args)...);
    }

    template<typename T, typename Allocator>
    inline void destroyField(Allocator &allocator, T *field) noexcept {
        return tf::allocate_polymorphic::deallocate<T, Allocator>(allocator, field);
    }
}

#endif //FASTPATH_FIELDALLOCATOR_H
