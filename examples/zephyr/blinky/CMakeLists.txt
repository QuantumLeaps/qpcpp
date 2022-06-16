# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(blinky)

zephyr_include_directories(
 ../../../include
 ../../../ports/zephyr
)

target_sources(app PRIVATE
 src/main.cpp
 src/bsp.cpp
 src/blinky.cpp
 ../../../src/qf/qep_hsm.cpp
 ../../../src/qf/qep_msm.cpp
 ../../../src/qf/qf_act.cpp
 ../../../src/qf/qf_qact.cpp
 ../../../src/qf/qf_defer.cpp
 ../../../src/qf/qf_dyn.cpp
 ../../../src/qf/qf_mem.cpp
 ../../../src/qf/qf_ps.cpp
 ../../../src/qf/qf_qact.cpp
 ../../../src/qf/qf_qeq.cpp
 ../../../src/qf/qf_qmact.cpp
 ../../../src/qf/qf_time.cpp
 ../../../ports/zephyr/qf_port.cpp
)

