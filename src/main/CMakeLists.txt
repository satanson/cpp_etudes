message(STATUS "link_libs=${link_libs}")
set(MAINS
        gen_decimal.cc
        )
foreach (src ${MAINS})
    get_filename_component(exe ${src} NAME_WE)
    add_executable(${exe} ${src})
    target_link_libraries(${exe} ${link_libs} pthread)
    list(APPEND main_deps ${exe})
endforeach ()
add_custom_target(main DEPENDS ${main_deps})
