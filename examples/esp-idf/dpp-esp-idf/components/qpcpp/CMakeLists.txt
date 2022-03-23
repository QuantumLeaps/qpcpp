set(QP_PATH "../../../../..")

idf_component_register(SRCS     "${QP_PATH}/src/qf/qep_hsm.cpp"
                                "${QP_PATH}/src/qf/qep_msm.cpp"
                                "${QP_PATH}/src/qf/qf_act.cpp"
                                "${QP_PATH}/src/qf/qf_actq.cpp"
                                "${QP_PATH}/src/qf/qf_defer.cpp"
                                "${QP_PATH}/src/qf/qf_dyn.cpp"
                                "${QP_PATH}/src/qf/qf_mem.cpp"
                                "${QP_PATH}/src/qf/qf_ps.cpp"
                                "${QP_PATH}/src/qf/qf_qact.cpp"
                                "${QP_PATH}/src/qf/qf_qeq.cpp"
                                "${QP_PATH}/src/qf/qf_qmact.cpp"
                                "${QP_PATH}/src/qf/qf_time.cpp"
                                "${QP_PATH}/ports/esp-idf/qf_port.cpp"
                    INCLUDE_DIRS "${QP_PATH}/include"
                                 "${QP_PATH}/src"
                                "${QP_PATH}/ports/esp-idf"
                                )