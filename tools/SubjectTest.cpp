//
// Created by Tom Fewster on 13/03/2016.
//

#include <iostream>
#include "performance.h"
#include "router/subscription.h"

int main(int argc, char *argv[])
{
    const size_t iterations = 10000000;

    fp::subscription s("SOME.TEST.SUBJECT");

    std::cout << "encode x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (int i = 0; i < iterations; i++) {
            s.matches("SOME.TEST.SUBJECT");
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;
}