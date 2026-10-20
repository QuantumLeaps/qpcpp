#============================================================================
# QP/C++ Real-Time Event Framework (RTEF) -- CMake support
#
# QP/C++ is built in-tree, as part of the application, so that the library
# is always compiled with the application's own qp_config.hpp.
#
# Usage in the application's CMakeLists.txt:
#
#   # where is qp_config.hpp (and, for embedded targets, the CPU flags)
#   add_library(qpcpp_config INTERFACE)
#   target_include_directories(qpcpp_config INTERFACE
#       ${CMAKE_CURRENT_SOURCE_DIR})
#
#   set(QPCPP_PORT win32)        # required, see the list below
#   set(QPCPP_KERNEL qv)         # arm-cm only: qv (default), qk, qxk
#   add_subdirectory(<path-to-qpcpp> qpcpp)
#
#   target_link_libraries(<app> PRIVATE qpcpp::qpcpp)
#
#   link_image(<app> <linker-script>)   # bare-metal: .elf, .map, .bin
#                                       # (3rd_party/cmake, via the board)
#
# Supported ports: win32, win32-qv, posix, posix-qv, arm-cm, freertos
#
# Configurations: in the "Spy" configuration, QP/Spy software tracing is
# built into qpcpp and Q_SPY is defined for qpcpp and for the application.
#
# CMake package (optional, OFF by default, host ports only):
# qpcpp can be built and installed as a CMake package for one qp_config.hpp,
# which is installed with the package. For example:
#
#   cmake -S <path-to-qpcpp> -B build -G "Ninja Multi-Config"
#         -DCMAKE_CONFIGURATION_TYPES="Debug;Release;Spy"
#         -DQPCPP_PORT=posix -DQPCPP_CONFIG_DIR=<dir-of-qp_config.hpp>
#         -DQPCPP_INSTALL=ON                   (all in one command line)
#   cmake --build build --config Release
#   cmake --install build --config Release --prefix <install-dir>
#
# The application then uses:
#
#   find_package(qpcpp 8.1.6 REQUIRED)  # CMAKE_PREFIX_PATH=<install-dir>
#   target_link_libraries(<app> PRIVATE qpcpp::qpcpp)
#
# An application compiled with a different QP configuration than the
# installed qpcpp library fails to link (QP configuration check, see
# include/qp_cfgchk.hpp).
#============================================================================
cmake_minimum_required(VERSION 3.25)
project(qpcpp LANGUAGES CXX)

set(QPCPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

option(QPCPP_INSTALL
    "qpcpp: install qpcpp as a CMake package (host ports only)" OFF)

# check what the application must provide ...................................
if(NOT TARGET qpcpp_config AND QPCPP_CONFIG_DIR) # qp_config.hpp directory
    cmake_path(ABSOLUTE_PATH QPCPP_CONFIG_DIR
        BASE_DIRECTORY ${CMAKE_SOURCE_DIR}
        NORMALIZE OUTPUT_VARIABLE _cfg_dir)
    if(NOT EXISTS ${_cfg_dir}/qp_config.hpp)
        message(FATAL_ERROR "qpcpp: qp_config.hpp not found in "
            "QPCPP_CONFIG_DIR (${_cfg_dir})")
    endif()
    add_library(qpcpp_config INTERFACE)
    target_include_directories(qpcpp_config INTERFACE ${_cfg_dir})
endif()
if(NOT TARGET qpcpp_config)
    message(FATAL_ERROR "qpcpp: the application must define the INTERFACE "
        "target 'qpcpp_config' (with the include directory of its "
        "qp_config.hpp) "
        "before add_subdirectory(qpcpp), or set QPCPP_CONFIG_DIR")
endif()
if(NOT QPCPP_PORT)
    message(FATAL_ERROR "qpcpp: set QPCPP_PORT before "
        "add_subdirectory(qpcpp) "
        "(win32, win32-qv, posix, posix-qv, arm-cm, freertos)")
endif()
string(TOLOWER "${QPCPP_PORT}" _port)
if(QPCPP_INSTALL AND NOT _port MATCHES "^(win32|win32-qv|posix|posix-qv)$")
    message(FATAL_ERROR "qpcpp: QPCPP_INSTALL is supported only for the "
        "host ports (win32, win32-qv, posix, posix-qv), not for "
        "'${QPCPP_PORT}'")
endif()
if(PROJECT_IS_TOP_LEVEL AND NOT DEFINED CMAKE_CXX_FLAGS_SPY)
    # qpcpp built on its own (e.g., the package): Spy flags as in Debug
    set(CMAKE_CXX_FLAGS_SPY "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING
        "Flags used by the C++ compiler during Spy builds")
endif()

# the qpcpp library .........................................................
add_library(qpcpp STATIC)
add_library(qpcpp::qpcpp ALIAS qpcpp)

# NOTE: BUILD_INTERFACE/INSTALL_INTERFACE matter only for the package
target_link_libraries(qpcpp PUBLIC $<BUILD_INTERFACE:qpcpp_config>)
target_include_directories(qpcpp PUBLIC
    $<BUILD_INTERFACE:${QPCPP_DIR}/include>
    $<INSTALL_INTERFACE:include/qpcpp>
    $<INSTALL_INTERFACE:include/qpcpp/config>)
target_compile_definitions(qpcpp PUBLIC $<$<CONFIG:Spy>:Q_SPY>)

set(_qf_srcs
    src/qf/qep_hsm.cpp
    src/qf/qep_msm.cpp
    src/qf/qf_act.cpp
    src/qf/qf_actq.cpp
    src/qf/qf_defer.cpp
    src/qf/qf_dyn.cpp
    src/qf/qf_mem.cpp
    src/qf/qf_ps.cpp
    src/qf/qf_qact.cpp
    src/qf/qf_qeq.cpp
    src/qf/qf_qmact.cpp
    src/qf/qf_time.cpp
)
set(_qs_srcs
    src/qs/qs.cpp
    src/qs/qs_rx.cpp
    src/qs/qs_fp.cpp
    src/qs/qs_64bit.cpp
)
# QP/Spy is not part of every QP/C++ distribution (e.g., not on GitHub):
# then all other configurations build, and only Spy stops with an error
if(NOT EXISTS ${QPCPP_DIR}/src/qs/qs.cpp
        OR NOT EXISTS ${QPCPP_DIR}/include/qs.hpp)
    message(WARNING "qpcpp: QP/Spy sources (src/qs/qs.cpp, include/qs.hpp) "
        "not found. The Spy configuration will not build.")
    set(_qs_srcs "")
    set(_msg "qpcpp: QP/Spy sources (qs.cpp, qs.hpp) not found")
    add_custom_target(qpcpp_qs_missing
        COMMAND "$<$<CONFIG:Spy>:${CMAKE_COMMAND};-E;echo;${_msg}>"
        COMMAND "$<$<CONFIG:Spy>:${CMAKE_COMMAND};-E;false>"
        COMMAND_EXPAND_LISTS VERBATIM)
    add_dependencies(qpcpp qpcpp_qs_missing)
endif()
target_sources(qpcpp PRIVATE src/qs/qstamp.cpp) # build date/time

# the port ..................................................................
if(_port MATCHES "^(win32|win32-qv|posix|posix-qv)$") #-------- host ports
    set(_port_dir ${QPCPP_DIR}/ports/${_port})
    target_sources(qpcpp PRIVATE
        ${_qf_srcs}
        ${_port_dir}/qf_port.cpp
        "$<$<CONFIG:Spy>:${_qs_srcs};${_port_dir}/qs_port.cpp>"
    )
    if(_port MATCHES "^win32")
        target_link_libraries(qpcpp PUBLIC $<$<CONFIG:Spy>:ws2_32>) # QS socket
    else()
        find_package(Threads REQUIRED)
        target_link_libraries(qpcpp PUBLIC Threads::Threads)
    endif()

elseif(_port STREQUAL "arm-cm") #------------------- ARM Cortex-M, bare-metal
    if(NOT QPCPP_KERNEL)
        set(QPCPP_KERNEL qv)
    endif()
    string(TOLOWER "${QPCPP_KERNEL}" _kernel)
    if(_kernel STREQUAL "qv")
        set(_kernel_srcs src/qv/qv.cpp)
    elseif(_kernel STREQUAL "qk")
        set(_kernel_srcs src/qk/qk.cpp)
    elseif(_kernel STREQUAL "qxk")
        if(NOT EXISTS ${QPCPP_DIR}/src/qxk/qxk.cpp
                OR NOT EXISTS ${QPCPP_DIR}/include/qxk.hpp)
            message(FATAL_ERROR "qpcpp: QXK kernel sources (src/qxk/qxk.cpp, "
                "include/qxk.hpp) not found. QXK is not part of every QP/C++ "
                "distribution (e.g., not on GitHub). Use QV or QK.")
        endif()
        set(_kernel_srcs
            src/qxk/qxk.cpp
            src/qxk/qxk_xthr.cpp
            src/qxk/qxk_sema.cpp
            src/qxk/qxk_mutex.cpp
        )
    else()
        message(FATAL_ERROR "qpcpp: unknown QPCPP_KERNEL '${QPCPP_KERNEL}' "
            "(qv, qk, qxk)")
    endif()

    # the port subdirectory for the compiler
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(_comp gnu)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "ARMClang")
        set(_comp armclang)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "IAR")
        set(_comp iar)
    else()
        message(FATAL_ERROR "qpcpp: arm-cm port: unsupported compiler "
            "'${CMAKE_CXX_COMPILER_ID}' (GNU, ARMClang, IAR)")
    endif()

    set(_port_dir ${QPCPP_DIR}/ports/arm-cm/${_kernel}/${_comp})
    target_sources(qpcpp PRIVATE
        ${_qf_srcs}
        ${_kernel_srcs}
        ${_port_dir}/${_kernel}_port.cpp
        "$<$<CONFIG:Spy>:${_qs_srcs}>"
    )
    if(_comp STREQUAL "gnu")
        # INTERFACE: compiled into the application, so that the linker
        # always uses it (from a library it would not be pulled in)
        target_sources(qpcpp INTERFACE ${_port_dir}/syscalls.c)
    endif()

elseif(_port STREQUAL "freertos") #----------------------------------- FreeRTOS
    if(NOT TARGET freertos_kernel)
        message(FATAL_ERROR "qpcpp: freertos port: add FreeRTOS (target "
            "'freertos_kernel') before add_subdirectory(qpcpp)")
    endif()
    set(_port_dir ${QPCPP_DIR}/ports/freertos)
    list(REMOVE_ITEM _qf_srcs src/qf/qf_actq.cpp) # FreeRTOS provides the queues
    target_sources(qpcpp PRIVATE
        ${_qf_srcs}
        ${_port_dir}/qf_port.cpp
        "$<$<CONFIG:Spy>:${_qs_srcs}>"
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_sources(qpcpp INTERFACE ${_port_dir}/syscalls.c)
    endif()
    target_link_libraries(qpcpp PUBLIC freertos_kernel)

else()
    message(FATAL_ERROR "qpcpp: unknown QPCPP_PORT '${QPCPP_PORT}' "
        "(win32, win32-qv, posix, posix-qv, arm-cm, freertos)")
endif()

if(NOT EXISTS ${_port_dir})
    message(FATAL_ERROR "qpcpp: port directory not found: ${_port_dir}")
endif()
target_include_directories(qpcpp PUBLIC $<BUILD_INTERFACE:${_port_dir}>)

message(STATUS "qpcpp: port ${_port_dir}")

# CMake package (QPCPP_INSTALL=ON) ..........................................
if(QPCPP_INSTALL)
    # the qp_config.hpp the library is built with (installed with it)
    get_target_property(_cfg_dirs qpcpp_config INTERFACE_INCLUDE_DIRECTORIES)
    set(_cfg_file "")
    foreach(_d IN LISTS _cfg_dirs)
        string(REGEX REPLACE "^\\$<BUILD_INTERFACE:(.*)>$" "\\1" _d "${_d}")
        if(EXISTS "${_d}/qp_config.hpp")
            set(_cfg_file "${_d}/qp_config.hpp")
            break()
        endif()
    endforeach()
    if(NOT _cfg_file)
        message(FATAL_ERROR "qpcpp: QPCPP_INSTALL: qp_config.hpp not found "
            "in the include directories of 'qpcpp_config' (${_cfg_dirs})")
    endif()

    # the package version from the file version-<x.y.z> in the qpcpp dir.
    file(GLOB _ver_file RELATIVE ${QPCPP_DIR} ${QPCPP_DIR}/version-*)
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" _ver "${_ver_file}")
    if(NOT _ver)
        message(FATAL_ERROR "qpcpp: QPCPP_INSTALL: version file "
            "${QPCPP_DIR}/version-<x.y.z> not found")
    endif()

    # the library of each configuration has its own name
    set_target_properties(qpcpp PROPERTIES
        DEBUG_POSTFIX _dbg
        SPY_POSTFIX _spy)

    install(TARGETS qpcpp EXPORT qpcppTargets ARCHIVE DESTINATION lib)
    install(DIRECTORY ${QPCPP_DIR}/include/ DESTINATION include/qpcpp
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
    install(DIRECTORY ${_port_dir}/ DESTINATION include/qpcpp
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
    # NOTE: qp_config.hpp in its own directory, so that an application's own
    # qp_config.hpp takes precedence (and is checked by the QP configuration
    # check), while the installed qp_config.hpp is the default
    install(FILES ${_cfg_file} DESTINATION include/qpcpp/config)
    install(EXPORT qpcppTargets NAMESPACE qpcpp:: DESTINATION lib/cmake/qpcpp)

    if(_port MATCHES "^posix")
        set(_deps "find_dependency(Threads)")
    else()
        set(_deps "")
    endif()
    file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qpcppConfig.cmake
        CONTENT [=[
# qpcpp CMake package: QP/C++ @_ver@ built for the port '@_port@' and for
# the qp_config.hpp installed with this package
# (include/qpcpp/config/qp_config.hpp)
include(CMakeFindDependencyMacro)
@_deps@
include("${CMAKE_CURRENT_LIST_DIR}/qpcppTargets.cmake")
set(qpcpp_PORT "@_port@")
]=] @ONLY)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/qpcppConfigVersion.cmake
        VERSION ${_ver} COMPATIBILITY ExactVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/qpcppConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/qpcppConfigVersion.cmake
        DESTINATION lib/cmake/qpcpp)

    message(STATUS "qpcpp: package ${_ver} with ${_cfg_file}")
endif()
