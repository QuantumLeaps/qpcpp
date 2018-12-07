# test-script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html

# preamble...
proc on_reset {} {
    expect_pause
    glb_filter ON
    continue
    expect "@timestamp TE0-Arm  Obj=l_blinky.m_timeEvt,AO=l_blinky,*"
    expect "===RTC===> St-Init  Obj=l_blinky,State=QHsm::top->off"
    expect "@timestamp LED 0"
    expect "===RTC===> St-Entry Obj=l_blinky,State=off"
    expect "@timestamp Init===> Obj=l_blinky,State=off"
    current_obj SM_AO l_blinky
}

# tests...
test "TIMEOUT_SIG->l_blinky"
post TIMEOUT_SIG
expect "@timestamp QF-New   Sig=TIMEOUT_SIG,*"
expect "@timestamp MP-Get   Obj=EvtPool1,*"
expect "@timestamp AO-Post  Sdr=QS_RX,Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp AO-GetL  Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp Disp===> Obj=l_blinky,Sig=TIMEOUT_SIG,State=off"
expect "@timestamp LED 1"
expect "===RTC===> St-Entry Obj=l_blinky,State=on"
expect "@timestamp ===>Tran Obj=l_blinky,Sig=TIMEOUT_SIG,State=off->on"
expect "@timestamp QF-gc    Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp MP-Put   Obj=EvtPool1,*"
expect "@timestamp Trg-Done QS_RX_EVENT"

test "timeEvt->Blinky-off (tick)"
current_obj TE l_blinky.m_timeEvt
tick
expect "           Tick<0>  Ctr=*"
expect "@timestamp TE0-Post Obj=l_blinky.m_timeEvt,Sig=TIMEOUT_SIG,AO=l_blinky"
expect "@timestamp AO-Post  Sdr=QS_RX,Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp AO-GetL  Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp Disp===> Obj=l_blinky,Sig=TIMEOUT_SIG,State=off"
expect "@timestamp LED 1"
expect "===RTC===> St-Entry Obj=l_blinky,State=on"
expect "@timestamp ===>Tran Obj=l_blinky,Sig=TIMEOUT_SIG,State=off->on"
expect "@timestamp Trg-Done QS_RX_TICK"

test "timeEvt->Blinky-on (tick)" -noreset
tick
expect "           Tick<0>  Ctr=*"
expect "@timestamp TE0-Post Obj=l_blinky.m_timeEvt,Sig=TIMEOUT_SIG,AO=l_blinky"
expect "@timestamp AO-Post  Sdr=QS_RX,Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp AO-GetL  Obj=l_blinky,Evt<Sig=TIMEOUT_SIG,*"
expect "@timestamp Disp===> Obj=l_blinky,Sig=TIMEOUT_SIG,State=on"
expect "@timestamp LED 0"
expect "===RTC===> St-Entry Obj=l_blinky,State=off"
expect "@timestamp ===>Tran Obj=l_blinky,Sig=TIMEOUT_SIG,State=on->off"
expect "@timestamp Trg-Done QS_RX_TICK"


# the end
end
