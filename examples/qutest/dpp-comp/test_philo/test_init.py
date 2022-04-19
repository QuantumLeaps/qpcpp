# test script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html

# preamble...
def on_reset():
    expect_pause()
    glb_filter(GRP_ALL)
    continue_test()
    expect("===RTC===> St-Init  Obj=Philo::inst[0],State=QP::QHsm::top->Philo::thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=Philo::inst[0].m_timeEvt,AO=Table::inst,Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=Philo::inst[0],State=Philo::thinking")
    expect("@timestamp Init===> Obj=Philo::inst[0],State=Philo::thinking")
    expect("===RTC===> St-Init  Obj=Philo::inst[1],State=QP::QHsm::top->Philo::thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=Philo::inst[1].m_timeEvt,AO=Table::inst,Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=Philo::inst[1],State=Philo::thinking")
    expect("@timestamp Init===> Obj=Philo::inst[1],State=Philo::thinking")
    expect("===RTC===> St-Init  Obj=Philo::inst[2],State=QP::QHsm::top->Philo::thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=Philo::inst[2].m_timeEvt,AO=Table::inst,Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=Philo::inst[2],State=Philo::thinking")
    expect("@timestamp Init===> Obj=Philo::inst[2],State=Philo::thinking")
    expect("===RTC===> St-Init  Obj=Philo::inst[3],State=QP::QHsm::top->Philo::thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=Philo::inst[3].m_timeEvt,AO=Table::inst,Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=Philo::inst[3],State=Philo::thinking")
    expect("@timestamp Init===> Obj=Philo::inst[3],State=Philo::thinking")
    expect("===RTC===> St-Init  Obj=Philo::inst[4],State=QP::QHsm::top->Philo::thinking")
    expect("@timestamp BSP_CALL BSP::random *")
    expect("@timestamp TE0-Arm  Obj=Philo::inst[4].m_timeEvt,AO=Table::inst,Tim=*,Int=0")
    expect("===RTC===> St-Entry Obj=Philo::inst[4],State=Philo::thinking")
    expect("@timestamp Init===> Obj=Philo::inst[4],State=Philo::thinking")
    expect("===RTC===> St-Init  Obj=Table::inst,State=QP::QHsm::top->NULL")
    expect_run()

# tests...
test("init")
