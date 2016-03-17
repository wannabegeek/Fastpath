
#ifndef TFSessionEventType_h
#define TFSessionEventType_h

#include <string>

namespace DCF {
    typedef enum {
        NONE = 0,
        READ = 1 << 0,
        WRITE = 1 << 1,
        ALL = READ | WRITE
    } EventType;

    class EventException {
    protected:
        std::string m_description;
    public:
        EventException(const std::string &description) : m_description(description) {}
        virtual const std::string &description() const {return m_description;}
    };
}

#endif