# ports/win32-qv
target_include_directories(qpcpp PUBLIC .)
target_sources(qpcpp PRIVATE
    qf_port.cpp
    $<$<BOOL:${QPCPP_CFG_GUI}>:${CMAKE_CURRENT_SOURCE_DIR}/qwin_gui.c>
    $<$<CONFIG:Spy>:${CMAKE_CURRENT_SOURCE_DIR}/qs_port.cpp>
)
