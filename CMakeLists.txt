cmake_minimum_required(VERSION 3.17)
project(cppcon_benchmarks VERSION 1.0.0)

find_package(HPX REQUIRED)

enable_testing()

function(add_simd_test target)
    add_hpx_executable(${target}
    ESSENTIAL
    SOURCES main.cpp
    )

    target_compile_options(${target} PUBLIC "-march=native" "-fno-tree-vectorize")

    set(CORES)
    if(${SIMD_CORES})
        set(CORES "--hpx:threads=${SIMD_CORES}")
    endif()

    set(END)
    if(${SIMD_END})
        set(END "--end=${SIMD_END}")
    endif()

    add_test(NAME ${target}_test COMMAND ${target} ${CORES} ${END})
endfunction()

add_subdirectory(benchmarks)
