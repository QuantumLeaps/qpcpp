# ports/msp430
if(QPCPP_CFG_UNIT_TEST)
    target_include_directories(qpcpp PUBLIC qutest)
else()
    if(NOT KERNEL MATCHES "q[vk]")
        message(WARNING "Kernel ${KERNEL} is not supported! Falling back to QV kernel")
        set(KERNEL qv)
    endif()
    target_include_directories(qpcpp PUBLIC ${KERNEL})
endif()
