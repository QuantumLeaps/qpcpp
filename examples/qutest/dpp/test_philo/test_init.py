# test script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/html

# preamble...
def on_reset():
    expect_pause()
    glb_filter(GRP_ON)
    continue_test()
    expect("@timestamp AO-Subsc Obj=AO_Philo<2>,Sig=EAT_SIG")
    expect("@timestamp AO-Subsc Obj=AO_Philo<2>,Sig=TEST_SIG")
    expect("===RTC===> St-Init  Obj=AO_Philo<2>,State=QP::QHsm::top->thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=l_philo<2>.m_timeEvt,AO=AO_Philo<2>,Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=AO_Philo<2>,State=thinking")
    expect("@timestamp Init===> Obj=AO_Philo<2>,State=thinking")

# tests...
test("init")
