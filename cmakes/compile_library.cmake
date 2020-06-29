include(CMakeParseArguments)
macro(COMPILE_LIBRARY)
    cmake_parse_arguments(
            ARG
            "STATIC;SHARED"
            "LIBS"
            ""
            ${ARGN})

    if (ARG_STATIC)
        SET(LIB_FLAG "STATIC")
    elseif (ARG_SHARED)
        SET(LIB_FLAG "SHARED")
    else ()
        message(FATAL_ERROR "COMPILE_LIBRARY(STATIC|SHARED)")
    endif ()

    file(GLOB subdir "*")
    message("subdir=${subdir}")
    list(FILTER subdir EXCLUDE REGEX "^[a-zA-Z_]+$")
    message("after filter, subdir=${subdir}")
    foreach (d ${subdir})
        if (EXISTS ${d}/CMakeLists.txt)
            add_subdirectory(${d})
            list(APPEND modules_deps ${d})
            get_filename_component(lib ${d} NAME_WE)
            list(APPEND ${ARG_LIBS} ${lib})
            set(${ARG_LIBS} ${${ARG_LIBS}} PARENT_SCOPE)
        endif ()
    endforeach ()
    add_custom_target(modules DEPENDS ${modules_deps})
endmacro()
