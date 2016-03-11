//
// Created by Tom Fewster on 11/03/2016.
//

#ifndef TFDCF_BOOTSTRAP_H
#define TFDCF_BOOTSTRAP_H

#include <iosfwd>

namespace fp{
    class bootstrap {
    private:
    public:
        bootstrap(const std::string &service);
        ~bootstrap();

        void run();
    };
}

#endif //TFDCF_BOOTSTRAP_H
