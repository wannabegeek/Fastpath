//
// Created by Tom Fewster on 13/03/2016.
//

#include <iostream>
#include "performance.h"
#include "fastpath/router/subscription.h"
#include "fastpath/router/subject.h"

int main(int argc, char *argv[])
{
    const size_t iterations = 100000000;

    fp::subscription<> s("SOME.TEST.SUBJECT");

    std::cout << "encode x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        fp::subject<> t("SOME.TEST.SUBJECT");
        for (size_t i = 0; i < iterations; i++) {
            s.matches(t);
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;
}