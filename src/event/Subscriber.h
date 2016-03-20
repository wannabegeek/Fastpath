//
// Created by Tom Fewster on 20/03/2016.
//

#ifndef TFDCF_SUBSCRIBER_H
#define TFDCF_SUBSCRIBER_H

namespace DCF {
    class Transport;

    class Subscriber {
    public:
        Subscriber(Transport *transport);
        ~Subscriber();

        void addSubcription(const char *subject);
    };
}

#endif //TFDCF_SUBSCRIBER_H
