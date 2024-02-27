# ./src/qf
target_sources(qpcpp PRIVATE
    qep_hsm.cpp
    qep_msm.cpp
    qf_act.cpp
    # qf_actq.cpp - see below
    qf_defer.cpp
    qf_dyn.cpp
    # qf_mem.cpp - see below
    qf_ps.cpp
    qf_qact.cpp
    qf_qeq.cpp
    qf_qmact.cpp
    qf_time.cpp
)
if(NOT (${QPCPP_CFG_PORT} IN_LIST QPCPP_RTOS_PORTS))
    target_sources(qpcpp PRIVATE
        qf_actq.cpp
    )
endif()
if(NOT (${QPCPP_CFG_PORT} STREQUAL uc-os2))
    target_sources(qpcpp PRIVATE
        qf_mem.cpp
    )
endif()