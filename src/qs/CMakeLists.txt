# ./src/qs
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qs.cpp)
    target_sources(qpcpp PRIVATE
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_64bit.cpp>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_fp.cpp>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_rx.cpp>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qstamp.cpp>"
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs.cpp>"
        "$<$<AND:$<CONFIG:Spy>,$<BOOL:${QPCPP_CFG_UNIT_TEST}>>:${CMAKE_CURRENT_SOURCE_DIR}/qutest.cpp>"
    )
    if(NOT "Spy" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Spy)
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "CMake Configuration Types" FORCE)
    endif()
else()
    message(WARNING "Build configuration 'Spy' (QS component) not available in GPL verion!")
    if("Spy" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES Spy)
        set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "CMake Configuration Types" FORCE)
    endif()
    target_sources(qpcpp PRIVATE
        "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qstamp.cpp>"
    )
endif()
