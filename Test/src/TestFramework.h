#pragma once

#ifdef HIT_RELEASE
#define HIT_FORCE_RELEASE_LOG
#endif

#include "Core/Log.h"
#include "Core/Memory.h"

#include <vector>
#include <utility>
#include <iostream>
#include <ctime>
#include <cstdlib>

#define test_check(condition) { if(!(condition)) { return hit::TEST_ERROR; } else { hit_info("Test({}) success.", #condition); } }
#define test_expect(condition, expct) { if((condition) != (expct)) { return hit::TEST_ERROR; } else { hit_info("Test({}) success.", #condition); } }

#define test_silent_check(condition) { if(!(condition)) { return hit::TEST_ERROR; } }
#define test_silent_expect(condition, expct) { if(!(condition)) { return hit::TEST_ERROR; } }

#define test_random_number(min, max) (rand() % (max) + (min))

#define test_success() return hit::TEST_SUCCESS;
#define test_failure() return hit::TEST_ERROR;

#define get_test(test) #test, &test

namespace hit
{
    using test_val = int;

    constexpr test_val TEST_ERROR = -1;
    constexpr test_val TEST_SUCCESS = 0;

    struct Test
    {
        virtual ~Test() = default;
        virtual test_val test() = 0;
    };

    using TestFuntion = test_val(*)();

    struct TestSystem
    {
        std::vector<std::pair<const char*, TestFuntion>> tests;

        void initialize()
        {
            Log::initialize_log_system();
            Memory::initialize_memory_system();

            srand((unsigned)time(NULL));
        }

        void shutdown()
        {
            Memory::shutdown_memory_system();
            Log::shutdown_log_system();
        }

        void add_test(const char* name, TestFuntion test)
        {
            tests.push_back(std::make_pair(name, test));
        }

        void run_all()
        {
            for(auto& [name, test] : tests)
            {
                hit_info("Start test: {}", name);
                auto result = test();
                hit_error_if(result == TEST_ERROR, "Test '{}' failed.", name);
                hit_info_if(result == TEST_SUCCESS, "Test '{}' success.", name);
                std::cout << "\n\n";
            }
        }
    };
}