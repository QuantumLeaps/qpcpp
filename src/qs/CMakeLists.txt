# ./src/qs
target_sources(qpcpp PRIVATE
    "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_64bit.cpp>"
    "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_fp.cpp>"
    "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_rx.cpp>"
    "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qstamp.cpp>"
    "$<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs.cpp>"
    "$<$<AND:$<CONFIG:Spy>,$<BOOL:${QPCPP_CFG_UNIT_TEST}>>:${CMAKE_CURRENT_SOURCE_DIR}/qutest.cpp>"
)
