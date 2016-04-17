//
// Created by Tom Fewster on 27/02/2016.
//

#include "performance.h"
#include "fastpath/messages/Message.h"
#include "fastpath/utils/tfpool.h"

int main(int argc, char *argv[])
{

    LOG_LEVEL(tf::logger::debug);

    const size_t iterations = 1000000;
    typedef tf::pool<DCF::Message> PoolType;

    PoolType pool(3);

    std::vector<PoolType::shared_ptr_type> encoded_messages(iterations);
    DCF::MessageBuffer buffer(1000000);

    std::cout << "encode x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (size_t i = 0; i < iterations; i++) {
            auto msg = pool.allocate_unique_ptr();
            msg->setSubject("SOME.TEST.SUBJECT");
            float32_t t = 22.0;
            msg->addScalarField("TEST", t);
            msg->addDataField("Name", "Tom");
            msg->addDataField("Name2", "Zac");

            /* const size_t encoded_len = */msg->encode(buffer);
            //encoded_messages.emplace_back(std::move(msg));
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::vector<PoolType::shared_ptr_type> decoded_messages(iterations);
    std::cout << "decode x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (size_t i = 0; i < iterations + 1; i++) {
            auto msg = pool.allocate_unique_ptr();

            const DCF::MessageBuffer::ByteStorageType &storage = buffer.byteStorage();
            if (!msg->decode(storage)) {
                break;
            }
            buffer.erase_front(storage.bytesRead());

            //decoded_messages.emplace_back(std::move(msg));
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

}
