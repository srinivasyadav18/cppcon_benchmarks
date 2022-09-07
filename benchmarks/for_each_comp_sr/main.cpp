#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>
#include <hpx/include/compute.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>

std::size_t threads;

// #define SIMD_TEST_WITH_INT
#define SIMD_TEST_WITH_FLOAT
// #define SIMD_TEST_WITH_DOUBLE

#define TEST_WITH_VECTORIZATION

struct test_t
{   
    template <typename T>
    void operator()(T &x)
    {
        using namespace std;
        using namespace std::experimental;

        for (int i = 0; i < 100; i++)
        {
            x = 5.0f * sin(x) + 6.0f * cos(x);
        }
    }
} test_{};

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    hpx::execution::experimental::explicit_scheduler_executor<
        hpx::execution::experimental::thread_pool_scheduler>
        p;

    auto exec = hpx::execution::experimental::with_priority(
        p, hpx::threads::thread_priority::bound);

    std::vector<T> nums(n);

    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
            hpx::for_each(policy.on(exec), nums.begin(), nums.end(), test_) |
                hpx::this_thread::experimental::sync_wait();
        }
        else
        {
            hpx::for_each(policy, nums.begin(), nums.end(), test_); 
        }
    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << hpx::count(hpx::execution::par_simd, nums.begin(), nums.end(), gen());

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"