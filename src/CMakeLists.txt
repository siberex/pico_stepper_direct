if (NOT TARGET stepper)
    add_library("stepper" INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/stepper.h
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