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

#include <hpx/config.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/datapar.hpp>

#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>
#include <random>
#include <filesystem>
#include <limits>

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
        x = 5.0f + x;
    }
} test_{};

template <typename ExPolicy, typename T, typename Gen, typename Flag>
auto test(ExPolicy policy, std::size_t n, Gen gen, Flag f)
{  
    std::vector<T> nums(n);

    auto t1 = std::chrono::high_resolution_clock::now();
        if constexpr (std::is_same_v<Flag, std::true_type>){
            hpx::for_each(policy, nums.begin(), nums.end(), test_) |
                hpx::this_thread::experimental::sync_wait();
        }
        else
        {
            hpx::for_each(policy, nums.begin(), nums.end(), test_); 
        }
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = t2 - t1;
    return diff.count();
}

template <typename ExPolicy, typename T, typename Gen>
auto test3(ExPolicy policy, std::size_t iterations, std::size_t n, Gen gen)
{
    double avg_time = 0.0;
    for (std::size_t i = 0; i < iterations; i++)
    {
        avg_time += test<ExPolicy, T, Gen>(policy, n, Gen{});
    }
    avg_time /= (double) iterations;
    return avg_time;
}

template <typename T, typename Gen>
void test4(std::string type, 
        std::size_t start, std::size_t end,
        std::size_t iterations, Gen gen)
{
    hpx::execution::experimental::explicit_scheduler_executor<
        hpx::execution::experimental::thread_pool_scheduler>
        p;

    auto exec = hpx::execution::experimental::with_priority(
        p, hpx::threads::thread_priority::bound);

    std::string file_name = std::string("plots/") +
                            type + 
                            std::string(".csv");
    std::ofstream fout(file_name.c_str());

    using V = typename hpx::parallel::traits::vector_pack_type<T>::type;
    static constexpr size_t lane = hpx::parallel::traits::vector_pack_size<V>::value;

    auto& seq_pol = hpx::execution::seq;
    auto& simd_pol = hpx::execution::simd;
    auto& par_pol = hpx::execution::par;
    auto& par_simd_pol = hpx::execution::par_simd;
    auto par_sr_pol = hpx::execution::par.on(exec);
    auto par_simd_sr_pol = hpx::execution::par_simd.on(exec);

    fout << "n,lane,threads,seq,simd,par,par_simd,par_sr,par_simd_sr\n";
    for (std::size_t i = start; i <= end; i++)
    {
        double offset = 0.0;
        for (std::size_t chunk = std::pow(2, i); chunk < std::pow(2, i+1); chunk += (chunk/4))
        {
            for (std::size_t iter = 0; iter < iterations; iter++)
            {
                fout << double(double(i) + offset)
                    << ","
                    << lane
                    << ","
                    << threads
                    << ","
                    << test<decltype(seq_pol), T, Gen, std::false_type>(
                        seq_pol, chunk, gen, std::false_type{}) 
                    << ","
                #if defined (TEST_WITH_VECTORIZATION)
                    << test<decltype(simd_pol), T, Gen, std::false_type>(
                        simd_pol, chunk, gen, std::false_type{})
                #else
                    << std::numeric_limits<double>::max()
                #endif
                    << ","
                    << test<decltype(par_pol), T, Gen, std::false_type>(
                        par_pol, chunk, gen, std::false_type{})
                    << ","
                #if defined (TEST_WITH_VECTORIZATION)
                    << test<decltype(par_simd_pol), T, Gen, std::false_type>(
                        par_simd_pol, chunk, gen, std::false_type{})
                #else
                    << std::numeric_limits<double>::max()
                #endif
                    << ","
                    << test<decltype(par_sr_pol), T, Gen, std::true_type>(
                        par_sr_pol, chunk, gen, std::true_type{}) 
                    << ","
                #if defined (TEST_WITH_VECTORIZATION)
                    << test<decltype(par_simd_sr_pol), T, Gen, std::true_type>(
                        par_simd_sr_pol, chunk, gen, std::true_type{}) 
                #else
                    << std::numeric_limits<double>::max()
                #endif
                    << "\n";
                fout.flush();
            }
            offset += 0.25;
        }
    }
    fout.close();
}

struct gen_int_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_int_distribution<int> dist_int {1, 1024};
    auto operator()()
    {
        return dist_int(mersenne_engine);
    }
} gen_int{};

struct gen_float_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_real_distribution<float> dist_float {1, 1024};
    auto operator()()
    {
        return dist_float(mersenne_engine);
   }
} gen_float{};

struct gen_double_t{
    std::mt19937 mersenne_engine {42};
    std::uniform_real_distribution<float> dist_double {1, 1024};
    auto operator()()
    {
        return dist_double(mersenne_engine);
    }
} gen_double{};

int hpx_main(hpx::program_options::variables_map& vm)
{
    std::filesystem::create_directory("plots");
    threads = hpx::get_os_thread_count();
    std::uint64_t iterations = vm["iterations"].as<std::uint64_t>();
    std::uint64_t start = vm["start"].as<std::uint64_t>();
    std::uint64_t end = vm["end"].as<std::uint64_t>();

    #if defined (SIMD_TEST_WITH_INT)
        test4<int, gen_int_t>("int", start, end, iterations, gen_int);
    #endif

    #if defined (SIMD_TEST_WITH_FLOAT)
        test4<float, gen_float_t>("float", start, end, iterations, gen_float);
    #endif

    #if defined (SIMD_TEST_WITH_DOUBLE)
        test4<double, gen_double_t>("double", start, end, iterations, gen_double);
    #endif

    return hpx::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) 
{ 
    namespace po = hpx::program_options;

    po::options_description desc_commandline;
    desc_commandline.add_options()
        ("iterations", po::value<std::uint64_t>()->default_value(5),
         "number of repititions")
        ("start", po::value<std::uint64_t>()->default_value(5),
         "start of number of elements in 2^x")
        ("end", po::value<std::uint64_t>()->default_value(22),
         "end of number of elements in 2^x")
    ;

    // Initialize and run HPX
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::init(argc, argv, init_args);    
}
