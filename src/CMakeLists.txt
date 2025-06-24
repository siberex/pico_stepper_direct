if (NOT TARGET stepper)
    add_library("stepper" INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/stepper.h
    )
    target_compile_features("stepper" INTERFACE cxx_std_20)
    target_compile_options("stepper" INTERFACE
            -Wall
            -Wextra
            -Wformat
    )

    target_sources("stepper" INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/stepper.cpp
    )

    # Pull in required Pico-SDK libraries
    target_link_libraries("stepper" INTERFACE
            pico_stdlib
            hardware_pwm
    )

    #target_include_directories("stepper" INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif ()