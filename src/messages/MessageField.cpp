//
// Created by Tom Fewster on 25/02/2016.
//

#include "MessageField.h"
#include "Message.h"

namespace DCF {
    void MessageField::set(const char *identifier, const BaseMessage *msg) {
        setIdentifier(identifier);
        m_msg = msg;
    }

    const BaseMessage *MessageField::get() const {
        return m_msg;
    }


    std::ostream &MessageField::output(std::ostream &out) const {
        return out << m_identifier << ":message=" << *m_msg;
    }
}

