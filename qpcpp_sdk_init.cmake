# Pre-initialize the QPC SDK
# This file must be included prior to the project() call

# Note: this file is perhaps named badly, as it provides a method qpc_sdk_init which
# the enclosing project calls LATER to actually "initialize" the SDK (by including the CMakeLists.txt from this
# same directory)

if (NOT TARGET _qpcpp_sdk_pre_init_marker)
    add_library(_qpcpp_sdk_pre_init_marker INTERFACE)

    function(qpcpp_is_top_level_project VAR)
        string(TOLOWER ${CMAKE_CURRENT_LIST_DIR} __list_dir)
        string(TOLOWER ${CMAKE_SOURCE_DIR} __source_dir)
        if (__source_dir STREQUAL __list_dir)
            set(${VAR} 1 PARENT_SCOPE)
        else()
            set(${VAR} 0 PARENT_SCOPE)
        endif()
    endfunction()

    function(qpcpp_message_debug MESSAGE)
        # The log-level system was added in CMake 3.15.
        if(${CMAKE_VERSION} VERSION_LESS "3.15.0")
            message(${MESSAGE})
        else()
            message(DEBUG ${MESSAGE})
        endif()
    endfunction()

    if (NOT QPCPP_SDK_PATH)
        set(QPCPP_SDK_PATH ${CMAKE_CURRENT_LIST_DIR})
    endif ()

    file(REAL_PATH "${QPCPP_SDK_PATH}" QPCPP_SDK_PATH BASE_DIRECTORY "${CMAKE_BINARY_DIR}")

    set(QPCPP_SDK_PATH ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "Path to the QP/C++ SDK" FORCE)

    list(APPEND CMAKE_MODULE_PATH ${QPCPP_SDK_PATH}/cmake)

    message("QPCPP_SDK_PATH is ${CMAKE_CURRENT_LIST_DIR}")

    function(qpcpp_sdk_init)
        if (NOT CMAKE_PROJECT_NAME)
            message(WARNING "qpcpp_sdk_init() should be called after the project is created (and languages added)")
        endif()

        add_subdirectory(${QPCPP_SDK_PATH} qpcpp-sdk)
    endfunction()
endif()
