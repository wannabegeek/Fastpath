//
// Created by Tom Fewster on 27/02/2016.
//

#include <chrono>
#include <Messages/Message.h>
#include <Utils/tfpool.h>

template<typename T = std::chrono::milliseconds> struct measure {
    template<typename F, typename ...Args> static typename T::rep execution(F&& func, Args&&... args) {
        auto start = std::chrono::system_clock::now();
        std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
        auto duration = std::chrono::duration_cast<T>(std::chrono::system_clock::now() - start);
        return duration.count();
    }
};

int main(int argc, char *argv[])
{
    const size_t iterations = 1000000;
    typedef tf::pool<DCF::Message> PoolType;

    PoolType pool(iterations);

    std::vector<PoolType::ptr_type> encoded_messages(iterations);
    DCF::MessageBuffer buffer(1000000);

    std::cout << "encode x" << iterations << ": " << measure<std::chrono::microseconds>::execution([&]() {
        for (int i = 0; i < iterations; i++) {
            PoolType::ptr_type msg = pool.allocate_ptr();
            msg->setSubject("SOME.TEST.SUBJECT");
            float32_t t = 22.0;
            msg->addScalarField("TEST", t);
            msg->addDataField("Name", "Tom");
            msg->addDataField("Name", "Zac");

            const size_t encoded_len = msg->encode(buffer);
            encoded_messages.emplace_back(std::move(msg));
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::vector<PoolType::ptr_type> decoded_messages(iterations);
    std::cout << "decode x" << iterations << ": " << measure<std::chrono::microseconds>::execution([&]() {
        for (int i = 0; i < iterations + 1; i++) {
            PoolType::ptr_type msg = pool.allocate_ptr();

            const DCF::ByteStorage &storage = std::move(buffer.byteStorage());
            if (!msg->decode(storage)) {
                break;
            }
            buffer.erase_front(storage.bytesRead());

            decoded_messages.emplace_back(std::move(msg));
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

}
