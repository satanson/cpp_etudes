message(STATUS "link_libs=${link_libs}")
set(MAINS
        playground.cc
        gen_decimal.cc
        decimal_add_sub_mul_div.cc
        asm_add.cc
        div_mod.cc
        string_insert.cc
        atomic_fetch_add_strength_test.cc
        )
foreach (src ${MAINS})
    get_filename_component(exe ${src} NAME_WE)
    add_executable(${exe} ${src})
    target_link_libraries(${exe} ${link_libs} pthread)
    list(APPEND main_deps ${exe})
endforeach ()
add_custom_target(main DEPENDS ${main_deps})
