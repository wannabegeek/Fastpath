//
// Created by Tom Fewster on 24/04/2016.
//

#include "performance.h"

#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>

int main(int argc, char *argv[]) {

    const size_t iterations = 100000;

    typedef std::array<std::size_t, iterations> keys_type;
    keys_type keys;

    std::srand(std::time(0));
    for (std::size_t i = 0; i < iterations; ++i) {
        keys[i] = std::rand();
    }

    std::cout << "Created samples now starting tests" << std::endl;

    std::unordered_map<std::size_t, std::size_t> m_map;
    std::vector<std::pair<std::size_t, std::size_t>> m_vect;
    m_map.reserve(512);
    m_vect.reserve(512);

    std::size_t max = 64;

    std::cout << "insert vector x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (int j = 0; j < 100; j++ {
            for (size_t i = 0; i < iterations; i++) {
                auto it = std::find_if(m_vect.begin(), m_vect.end(), [&] (const auto &v) noexcept {
                        return v.first == keys[i] % max;
                });
                if (it == m_vect.end()) {
                    m_vect.push_back(std::make_pair(keys[i] % max, i));
                }
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::cout << "obtain vector x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (size_t i = 0; i < iterations; i++) {
            auto it = std::find_if(m_vect.begin(), m_vect.end(), [&] (const auto &v) noexcept {
                    return v.first == keys[i] % max;
            });
            if (it != m_vect.end()) {
                // got it
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::cout << "insert map x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (size_t i = 0; i < iterations; i++) {
            auto it = m_map.find(keys[i] % max);
            if (it != m_map.end()) {
                m_map[keys[i] % max] = i;
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::cout << "obtain map x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (size_t i = 0; i < iterations; i++) {
            auto it = m_map.find(keys[i] % max);
            if (it != m_map.end()) {
                // got it
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;
}
