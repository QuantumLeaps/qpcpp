# ports/posix-qutest
target_include_directories(qpcpp PUBLIC .)
target_sources(qpcpp PRIVATE
    qutest_port.cpp
)