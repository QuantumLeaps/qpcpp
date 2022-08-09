# QPCPP Zephyr Module
# Copyright (c) 2022 Victor Chavez
# SPDX-License-Identifier: GPL-3.0-or-later

zephyr_library()

if(CONFIG_QPCPP)

set(QPCPP_ROOT_DIR ../)
zephyr_include_directories(${QPCPP_ROOT_DIR}/include
                            ${QPCPP_ROOT_DIR}/ports/zephyr
                            )
target_include_directories(app PUBLIC  ${QPCPP_ROOT_DIR}/include
                                        ${QPCPP_ROOT_DIR}/ports/zephyr)

zephyr_library_sources( ${QPCPP_ROOT_DIR}/src/qf/qep_hsm.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qep_msm.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_act.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_qact.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_defer.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_dyn.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_mem.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_ps.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_qact.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_qeq.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_qmact.cpp
                        ${QPCPP_ROOT_DIR}/src/qf/qf_time.cpp
                        ${QPCPP_ROOT_DIR}/ports/zephyr/qf_port.cpp)

if(CONFIG_QSPY)

target_compile_definitions(app PUBLIC Q_SPY)
zephyr_library_compile_definitions(Q_SPY)
zephyr_library_sources(${QPCPP_ROOT_DIR}/src/qs/qs.cpp
                        ${QPCPP_ROOT_DIR}/src/qs/qs_rx.cpp
                        ${QPCPP_ROOT_DIR}/src/qs/qs_fp.cpp
                        ${QPCPP_ROOT_DIR}/src/qs/qutest.cpp
                        ${QPCPP_ROOT_DIR}/src/qs/qs_64bit.cpp
                        ${QPCPP_ROOT_DIR}/include/qstamp.cpp)
endif() #CONFIG_QSPY

endif() #CONFIG_QPCPP
