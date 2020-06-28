set(TESTS
        hello_world.cc
        gtest_demo.cc
        mark_ptr_type_test.cc
        list_test.cc
        )
foreach (src ${TESTS})
    get_filename_component(exe ${src} NAME_WE)
    add_executable(${exe} ${src})
    target_link_libraries(${exe} ${LIBS} gtest pthread concurrent)
    list(APPEND unittest_deps ${exe})
endforeach ()
add_custom_target(unittest DEPENDS ${unittest_deps})