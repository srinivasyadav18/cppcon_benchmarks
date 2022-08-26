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

template <typename ExPolicy, typename T, typename Gen>
auto test(ExPolicy policy, std::size_t n, Gen gen)
{  
    std::vector<T> nums(n, 0.0), nums2(n, 0.0);
    if constexpr (hpx::is_parallel_execution_policy_v<ExPolicy>){
        hpx::generate(hpx::execution::par, nums.begin(), nums.end(), gen);
    }
    else
    {
        hpx::generate(hpx::execution::seq, nums.begin(), nums.end(), gen);
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
        hpx::adjacent_difference(policy, 
                nums.begin(), nums.end(), 
                nums2.begin()
            );
    auto t2 = std::chrono::high_resolution_clock::now();

    std::cout << hpx::count(hpx::execution::par_simd, nums2.begin(), nums2.end(), gen());

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}


// INCLUDE MAIN FUNCTION
#include "../../main.hpp"