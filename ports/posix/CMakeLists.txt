# ports/posix
target_include_directories(qpcpp PUBLIC .)
target_sources(qpcpp PRIVATE
    qf_port.cpp
    $<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_port.cpp>
)