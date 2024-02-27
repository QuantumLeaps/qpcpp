# ports/arm-cm
string(TOLOWER ${CMAKE_C_COMPILER_ID} _compiler_)
list (APPEND compList gnu iar armclang)
if(NOT (${_compiler_} IN_LIST compList))
    message(WARNING "no support for compiler '${_compiler_}'. Falling back to GNU. Just give it a try :)")
    set(_compiler_ gnu)
endif()
unset(compList)

set(qx_port_cpp ${KERNEL}_port.cpp)

if(QPCPP_CFG_UNIT_TEST)
    target_include_directories(qpcpp PUBLIC qutest)
else()
    target_include_directories(qpcpp PUBLIC ${KERNEL}/${_compiler_})
endif()

target_sources(qpcpp PRIVATE ${KERNEL}/${_compiler_}/${qx_port_cpp})

