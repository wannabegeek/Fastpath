//
// Created by Tom Fewster on 24/04/2016.
//

#include "performance.h"

#include <unordered_map>
#include <vector>
#include <array>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include "fastpath/utils/stringhash.h"

int main(int argc, char *argv[]) {

    const size_t iterations = 100000;

    typedef std::array<std::string, iterations> keys_type;
    keys_type keys;

    std::srand(std::time(0));
    for (std::size_t i = 0; i < iterations; ++i) {
        std::string temp;
        int count = std::rand() % 32;
        for (int j = 0; j < count; j++) {
            char c = (std::rand() % 26) + 'A';
            temp += c;
        }
        keys[i] = std::rand();
    }

    std::cout << "Created samples now starting tests" << std::endl;

    std::unordered_map<const char *, const size_t, tf::string_hash, tf::string_comparator> m_map;
    std::vector<std::pair<std::size_t, std::size_t>> m_vect;
    m_map.reserve(512);
    m_vect.reserve(512);

    std::size_t max = 122;

    std::cout << "insert vector x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (int j = 0; j < 100; j++) {
            for (size_t i = 0; i < iterations; i++) {
                const char *key = keys[i % max].c_str();
                auto hash = tf::string_hash()(key);
                auto it = std::find_if(m_vect.begin(), m_vect.end(), [&] (const auto &v) noexcept {
                    return v.first == hash;
                });
                if (it == m_vect.end()) {
                    m_vect.emplace_back(hash, i);
                }
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::cout << "obtain vector x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (int j = 0; j < 100; j++) {
            for (size_t i = 0; i < iterations; i++) {
                const char *key = keys[i % max].c_str();
                auto hash = tf::string_hash()(key);
                auto it = std::find_if(m_vect.begin(), m_vect.end(), [&](const auto &v) noexcept {
                    return v.first == hash;
                });
                if (it != m_vect.end()) {
                    // got it
                }
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::cout << "insert map x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (int j = 0; j < 100; j++) {
            for (size_t i = 0; i < iterations; i++) {
                const char *key = keys[i % max].c_str();
                auto it = m_map.find(key);
                if (it != m_map.end()) {
                    m_map.emplace(key, i);
                }
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;

    std::cout << "obtain map x" << iterations << ": " << tf::measure<std::chrono::microseconds>::execution([&]() {
        for (int j = 0; j < 100; j++) {
            for (size_t i = 0; i < iterations; i++) {
                const char *key = keys[i % max].c_str();
                auto it = m_map.find(key);
                if (it != m_map.end()) {
                    // got it
                }
            }
        }
    }) / static_cast<float>(iterations) << "us" << std::endl;
}
