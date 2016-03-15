#ifndef TFException_h
#define TFException_h

#include <sstream>
#include <stdexcept>

#define ThrowException(exception, msg) { \
		std::ostringstream mess; \
		mess << msg; \
		throw exception(mess.str()); \
}

#define ThrowExceptionWithArgs(exception, msg, ...) { \
		std::ostringstream mess; \
		mess << msg; \
		throw exception(mess.str(), __VA_ARGS__); \
}

namespace fp {
    class exception : public std::logic_error {
    public:
        exception(const std::string &description) : std::logic_error(description) {}

        friend std::ostream &operator<<(std::ostream &os, const exception &exception) {
            os << exception.what();
            return os;
        }
    };
} // namespace

#endif