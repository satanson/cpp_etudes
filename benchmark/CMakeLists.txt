message(STATUS "link_libs=${link_libs}")
set(BENCHMARKS
        basic_usage.cc
        vectorized_bm.cc
        doris_benchmark.cc
        benchmark_string_functions.cc
        benchmark_string_functions_substr.cc
        benchmark_int_arithmetic.cc
        benchmark_int_arithmetic_single.cc
        benchmark_string_functions_case.cc
        benchmark_vector.cc
        benchmark_function_args_more_or_less.cc
        benchmark_repeat.cc
        benchmark_concat.cc
        benchmark_memequal.cc
        benchmark_memequal_shuffle0.cc
        benchmark_memequal_shuffle1.cc
        benchmark_raw_allocator.cc
        benchmark_interpreters.cc
        )
foreach (src ${BENCHMARKS})
    get_filename_component(exe ${src} NAME_WE)
    add_executable(${exe} ${src})
    target_link_libraries(${exe} ${link_libs} benchmark pthread)
    list(APPEND benchmarks_deps ${exe})
endforeach ()
add_custom_target(benchmarks DEPENDS ${benchmarks_deps})
