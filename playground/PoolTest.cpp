//
// Created by Tom Fewster on 24/04/2016.
//

#include "performance.h"
#include "fastpath/messages/MutableMessage.h"
#include "tfpool_new.h"

class test {
    int t[5];
    double d[4];
};

int main(int argc, char *argv[])
{

    LOG_LEVEL(tf::logger::debug);

    const size_t iterations = 100000000;
    typedef tf::experimental::pool<test> PoolType;

    PoolType pool;

    test *msg = nullptr;
    std::cout << "obtain x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (size_t i = 0; i < iterations; i++) {
            msg = pool.allocate();
            pool.release(msg);
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

}