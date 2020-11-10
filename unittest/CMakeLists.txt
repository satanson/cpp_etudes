message(STATUS "link_libs=${link_libs}")
set(TESTS
        hello_world.cc
        gtest_demo.cc
        mark_ptr_type_test.cc
        list_test.cc
        util_test.cc
        hash_test.cc
        misc_test.cc
        decimal_microbm_test.cc
        test_c++17.cc
        test_decimal.cc
        test_default_init_allocator.cc
        test_string_functions.cc
        test_trim.cc
        test_reverse.cc
        test_repeat.cc
        )
foreach (src ${TESTS})
    get_filename_component(exe ${src} NAME_WE)
    add_executable(${exe} ${src})
    target_link_libraries(${exe} ${link_libs} gtest pthread atomic)
    list(APPEND unittest_deps ${exe})
endforeach ()
add_custom_target(unittest DEPENDS ${unittest_deps})
