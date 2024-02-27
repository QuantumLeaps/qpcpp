# use a recent CMake version
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(VERSION 3.13)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    cmake_policy(SET CMP0083 NEW)
endif()

# QPC SDK project root CMakeLists.txt
set(QPCPP_HOST_PORTS posix win32)
set(QPCPP_RTOS_PORTS embos freertos threadx uc-os2)
set(QPCPP_BAREMETAL_PORTS arm-cm arm-cr msp430 pic32)
set(QPCPP_MISC_PORTS qep-only)
foreach(p in HOST RTOS BAREMETAL MISC)
    list(APPEND QPCPP_ALL_PORTS ${QPCPP_${p}_PORTS})
endforeach()

# project configuration
if(DEFINED ENV{QPCPP_CFG_KERNEL} AND (NOT QPCPP_CFG_KERNEL))
    set(QPCPP_CFG_KERNEL $ENV{QPCPP_CFG_KERNEL})
    message("Using QPCPP_CFG_KERNEL from environment ('${QPCPP_CFG_KERNEL}')")
elseif(NOT QPCPP_CFG_KERNEL)
    set(QPCPP_CFG_KERNEL qv)
    message("Set QPCPP_CFG_KERNEL to ('${QPCPP_CFG_KERNEL}') since not specified")
endif()

if(DEFINED ENV{QPCPP_CFG_PORT} AND (NOT QPCPP_CFG_PORT))
    set(QPCPP_CFG_PORT $ENV{QPCPP_CFG_PORT})
    message("Using QPCPP_CFG_PORT from environment ('${QPCPP_CFG_PORT}')")
endif()

if(NOT QPCPP_CFG_GUI)
    set(QPCPP_CFG_GUI OFF CACHE BOOL "enable GUI support for matching ports (e.g. win32 or posix & GTK)")
    message("Set QPCPP_CFG_GUI to ('${QPCPP_CFG_GUI}') since not specified")
endif()

if(NOT QPCPP_CFG_UNIT_TEST)
    set(QPCPP_CFG_UNIT_TEST OFF CACHE BOOL "enable detailled unit testing support")
    message("Set QPCPP_CFG_UNIT_TEST to ('${QPCPP_CFG_UNIT_TEST}') since not specified")
endif()

if(NOT QPCPP_CFG_DEBUG)
    set(QPCPP_CFG_DEBUG ON CACHE BOOL "enable debug sessions")
    message("Set QPCPP_CFG_DEBUG to ('${QPCPP_CFG_DEBUG}') since not specified")
endif()

if(NOT QPCPP_CFG_VERBOSE)
    set(QPCPP_CFG_VERBOSE OFF CACHE BOOL "enable verbose build output")
    message("Set QPCPP_CFG_VERBOSE to ('${QPCPP_CFG_VERBOSE}') since not specified")
endif()

project(
    qpcpp
    VERSION "1.0.0"
    DESCRIPTION "QPCPP library"
    LANGUAGES C ASM
)

# add support for SPY configuration if not set via CMAKE_TOOLCHAIN_FILE
if(NOT CMAKE_C_FLAGS_SPY)
    foreach(LANG IN ITEMS C CXX ASM)
        set(CMAKE_${LANG}_FLAGS_SPY "${CMAKE_${LANG}_FLAGS_DEBUG} -DQ_SPY")
    endforeach()
endif()

# check target port plausibility
if(NOT QPCPP_CFG_PORT)
    message(WARNING "No PORT is configured! Falling back to target system '${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}'.")
    if(CMAKE_SYSTEM_NAME)
        string(TOLOWER ${CMAKE_SYSTEM_NAME} sysName)
    else()
        set(sysName generic)
    endif()
    if(CMAKE_SYSTEM_PROCESSOR)
        string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} sysProc)
    else()
        set(sysproc none)
    endif()
    if((sysName STREQUAL generic) AND (sysProc STREQUAL arm))
        set(PORT arm-cm)
    elseif(WIN32)
        set(PORT win32)
    elseif(UNIX)
        set(PORT posix)
    else()
        message(FATAL_ERROR No valid target port could be found. Aborting!)
    endif()
else()
    string(TOLOWER ${QPCPP_CFG_PORT} PORT)
endif()

if(NOT (${PORT} IN_LIST QPCPP_ALL_PORTS))
    if(DEFINED PORT)
        message(FATAL_ERROR "Target port '${PORT}' not found!")
    else()
        message(FATAL_ERROR "Target port not defined!")
    endif()
endif()

set(QPCPP_CFG_PORT ${PORT} CACHE STRING "The QPC target port for the system platform")
message(STATUS "Set QPCPP_CFG_PORT to ('${QPCPP_CFG_PORT}')")

# check/set Qx real time kernel
string(TOLOWER ${QPCPP_CFG_KERNEL} KERNEL)
list (APPEND kernList qv qk qxk)
if(NOT (${KERNEL} IN_LIST kernList))
    if(QPCPP_CFG_KERNEL)
        message(WARNING "Unknown kernel '${QPCPP_CFG_KERNEL}' specified!
    Falling back to QV kernel")
    endif()
    set(QPCPP_CFG_KERNEL QV CACHE STRING "set the desired micro kernel for your project. (default: QV)" FORCE)
    set(KERNEL qv)
endif()
unset(kernList)

add_library(qpcpp STATIC "")

# set position independent code compile/link parameters
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    include(CheckPIESupported)
    check_pie_supported()
endif()
set_property(TARGET qpcpp PROPERTY POSITION_INDEPENDENT_CODE FALSE)

# set up the include directory search path
target_include_directories(qpcpp PUBLIC
    src
    include
)

# add subdirectories with source/header files
add_subdirectory(src)
add_subdirectory(ports)

# set general defines
target_compile_definitions(qpcpp PRIVATE
    $<$<BOOL:${ADD_DEBUG_CODE}>:${ADD_DEBUG_CODE}>
    $<$<BOOL:${QPCPP_CFG_GUI}>:QWIN_GUI>
    # $<$<CONFIG:Spy>:Q_SPY> # set via toolchain file
    $<$<AND:$<CONFIG:Spy>,$<BOOL:${QPCPP_CFG_UNIT_TEST}>>:Q_UTEST>
)

target_compile_options(qpcpp PRIVATE
    $<$<BOOL:${QPCPP_CFG_VERBOSE}>:-v>
)

target_link_options(qpcpp PRIVATE
    $<$<BOOL:${QPCPP_CFG_VERBOSE}>:-v>
)

target_link_libraries(qpcpp PUBLIC
    $<$<AND:$<CONFIG:Spy>,$<STREQUAL:win32,${PORT}>>:ws2_32>
)

# print configuration
message(STATUS
"========================================================
  Configured project ${PROJECT_NAME} for ${PORT}
    PROJECT_NAME                = ${QPCPP_PROJECT}
    TARGET                      = ${TARGET}
    IMAGE                       = ${IMAGE}
    SW_VERSION                  = ${SW_VERSION}
    PORT                        = ${PORT}

    QPCPP_CFG_GUI               = ${QPCPP_CFG_GUI}
    QPCPP_CFG_UNIT_TEST         = ${QPCPP_CFG_UNIT_TEST}
    QPCPP_CFG_KERNEL            = ${QPCPP_CFG_KERNEL}
    QPCPP_CFG_DEBUG             = ${QPCPP_CFG_DEBUG}
    CMAKE_C_CPPCHECK            = ${CMAKE_C_CPPCHECK}
-- ========================================================
"
)

if(QPCPP_CFG_VERBOSE)
    message(STATUS
"========================================================
  System information:
    CMAKE_VERSION               = ${CMAKE_VERSION}
    CMAKE_CROSSCOMPILING        = ${CMAKE_CROSSCOMPILING}
    CMAKE_HOST_SYSTEM           = ${CMAKE_HOST_SYSTEM}
    CMAKE_HOST_SYSTEM_NAME      = ${CMAKE_HOST_SYSTEM_NAME}
    CMAKE_HOST_LINUX            = ${CMAKE_HOST_LINUX}
    CMAKE_HOST_UNIX             = ${CMAKE_HOST_UNIX}
    CMAKE_HOST_WIN32            = ${CMAKE_HOST_WIN32}
    CMAKE_SYSTEM                = ${CMAKE_SYSTEM}
    CMAKE_SYSTEM_NAME           = ${CMAKE_SYSTEM_NAME}
    CMAKE_SYSTEM_PROCESSOR      = ${CMAKE_SYSTEM_PROCESSOR}
    WIN32                       = ${WIN32}
    MSYS                        = ${MSYS}
    MINGW                       = ${MINGW}
    UNIX                        = ${UNIX}
    LINUX                       = ${LINUX}

    CMAKE_BUILD_TYPE            = ${CMAKE_BUILD_TYPE}
    CMAKE_C_COMPILER            = ${CMAKE_C_COMPILER}
    CMAKE_C_COMPILER_ID         = ${CMAKE_C_COMPILER_ID}
    CMAKE_C_COMPILER_VERSION    = ${CMAKE_C_COMPILER_VERSION}
    CMAKE_C_FLAGS               = ${CMAKE_C_FLAGS}

    CMAKE_CXX_COMPILER          = ${CMAKE_CXX_COMPILER}
    CMAKE_CXX_FLAGS             = ${CMAKE_CXX_FLAGS}

    CMAKE_ASM_COMPILER          = ${CMAKE_ASM_COMPILER}
    CMAKE_ASM_FLAGS             = ${CMAKE_ASM_FLAGS}
-- ========================================================
"
    )
endif()
