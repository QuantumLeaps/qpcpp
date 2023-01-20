# test script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html

# preamble...
def on_reset():
    expect_pause()
    glb_filter(GRP_ALL)
    continue_test()
    expect("@timestamp AO-Subsc Obj=Philo::inst[2],Sig=EAT_SIG")
    expect("@timestamp AO-Subsc Obj=Philo::inst[2],Sig=TEST_SIG")
    expect("===RTC===> St-Init  Obj=Philo::inst[2],State=QP::QHsm::top->Philo::thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=Philo::inst[2].m_timeEvt,AO=Philo::inst[2],Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=Philo::inst[2],State=Philo::thinking")
    expect("@timestamp Init===> Obj=Philo::inst[2],State=Philo::thinking")
    expect("===RTC===> St-Init  Obj=Table_dummy,State=QP::QHsm::top->NULL")
    expect_run()

# tests...
test("init")
