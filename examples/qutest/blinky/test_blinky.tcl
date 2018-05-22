#-----------------------------------------------------------------------------
# Product: QSPY -- test-script example for qutest.tcl
# Last updated for version 6.2.0
# Last updated on  2018-03-12
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) Quantum Leaps, LLC, All rights reserved.
#
# This program is open source software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Alternatively, this program may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GNU General Public License and are specifically designed for
# licensees interested in retaining the proprietary status of their code.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Contact information:
# https://www.state-machine.com
# mailto:info@state-machine.com
#-----------------------------------------------------------------------------

# preamble...
proc on_reset {} {
    expect_pause
    glb_filter ON
    continue
    expect "%timestamp TE0-Arm  Obj=l_blinky.m_timeEvt,AO=l_blinky,*"
    expect "===RTC===> St-Init  Obj=l_blinky,State=QHsm::top->off"
    expect "%timestamp LED 0"
    expect "===RTC===> St-Entry Obj=l_blinky,State=off"
    expect "%timestamp Init===> Obj=l_blinky,State=off"
    current_obj SM_AO l_blinky
}

# tests...
test "TIMEOUT_SIG->l_blinky"
post TIMEOUT_SIG
expect "%timestamp QF-New   Sig=TIMEOUT_SIG,*"
expect "%timestamp MP-Get   Obj=smlPoolSto,*"
expect "%timestamp AO-Post  Sdr=QS_RX,Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp AO-GetL  Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp Disp===> Obj=l_blinky,Sig=TIMEOUT_SIG,State=off"
expect "%timestamp LED 1"
expect "===RTC===> St-Entry Obj=l_blinky,State=on"
expect "%timestamp ===>Tran Obj=l_blinky,Sig=TIMEOUT_SIG,State=off->on"
expect "%timestamp QF-gc    Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp MP-Put   Obj=smlPoolSto,*"
expect "%timestamp Trg-Done QS_RX_EVENT"

test "timeEvt->Blinky (tick)"
current_obj TE l_blinky.m_timeEvt
tick
expect "%timestamp TE0-Post Obj=l_blinky.m_timeEvt,Sig=TIMEOUT_SIG,AO=l_blinky"
expect "%timestamp AO-Post  Sdr=QS_RX,Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp AO-GetL  Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp Disp===> Obj=l_blinky,Sig=TIMEOUT_SIG,State=off"
expect "%timestamp LED 1"
expect "===RTC===> St-Entry Obj=l_blinky,State=on"
expect "%timestamp ===>Tran Obj=l_blinky,Sig=TIMEOUT_SIG,State=off->on"
expect "%timestamp Trg-Done QS_RX_TICK"

test "timeEvt->Blinky (tick)" -noreset
tick
expect "%timestamp TE0-Post Obj=l_blinky.m_timeEvt,Sig=TIMEOUT_SIG,AO=l_blinky"
expect "%timestamp AO-Post  Sdr=QS_RX,Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp AO-GetL  Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "%timestamp Disp===> Obj=l_blinky,Sig=TIMEOUT_SIG,State=on"
expect "%timestamp LED 0"
expect "===RTC===> St-Entry Obj=l_blinky,State=off"
expect "%timestamp ===>Tran Obj=l_blinky,Sig=TIMEOUT_SIG,State=on->off"
expect "%timestamp Trg-Done QS_RX_TICK"


# the end
end
