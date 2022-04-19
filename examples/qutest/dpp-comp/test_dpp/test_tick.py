# test script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html/qutest.html

# preamble...
def on_reset():
    expect_pause()
    continue_test()
    expect_run()

# tests...
test("tick")
glb_filter(GRP_ALL)
current_obj(OBJ_TE, "Philo::inst[2].m_timeEvt")
tick()
expect("           Tick<0>  Ctr=*")
expect("           TE0-ADis Obj=Philo::inst[2].m_timeEvt,AO=Table::inst")
expect("@timestamp TE0-Post Obj=Philo::inst[2].m_timeEvt,Sig=TIMEOUT_SIG,AO=Table::inst")
expect("@timestamp AO-Post  Sdr=QS_RX,Obj=Table::inst,Evt<Sig=TIMEOUT_SIG,Pool=0,Ref=0>,*")
expect("@timestamp AO-GetL  Obj=Table::inst,Evt<Sig=TIMEOUT_SIG,Pool=0,Ref=0>")
expect("@timestamp Disp===> Obj=Table::inst,Sig=TIMEOUT_SIG,State=Table::serving")
expect("@timestamp Disp===> Obj=Philo::inst[2],Sig=TIMEOUT_SIG,State=Philo::thinking")
expect("@timestamp TE0-DisA Obj=Philo::inst[2].m_timeEvt,AO=Table::inst")
expect("===RTC===> St-Exit  Obj=Philo::inst[2],State=Philo::thinking")
expect("@timestamp MP-Get   Obj=EvtPool1,*")
expect("@timestamp QF-New   Sig=HUNGRY_SIG,*")
expect("@timestamp AO-LIFO  Obj=Table::inst,Evt<Sig=HUNGRY_SIG,Pool=1,*")
expect("@timestamp QUTEST_ON_POST HUNGRY_SIG,Obj=Table::inst 2")
expect("===RTC===> St-Entry Obj=Philo::inst[2],State=Philo::hungry")
expect("@timestamp ===>Tran Obj=Philo::inst[2],Sig=TIMEOUT_SIG,State=Philo::thinking->Philo::hungry")
expect("@timestamp =>Intern Obj=Table::inst,Sig=TIMEOUT_SIG,State=Table::active")
expect("@timestamp AO-GetL  Obj=Table::inst,Evt<Sig=HUNGRY_SIG,Pool=1,*")
expect("@timestamp Disp===> Obj=Table::inst,Sig=HUNGRY_SIG,State=Table::serving")
expect("@timestamp BSP_CALL BSP::displayPhilStat 2 hungry  ")
expect("@timestamp Disp===> Obj=Philo::inst[2],Sig=EAT_SIG,State=Philo::hungry")
expect("@timestamp BSP_CALL BSP::random *")
expect("@timestamp TE0-Arm  Obj=Philo::inst[2].m_timeEvt,AO=Table::inst,Tim=*,Int=0")
expect("===RTC===> St-Entry Obj=Philo::inst[2],State=Philo::eating")
expect("@timestamp ===>Tran Obj=Philo::inst[2],Sig=EAT_SIG,State=Philo::hungry->Philo::eating")
expect("@timestamp BSP_CALL BSP::displayPhilStat 2 eating  ")
expect("@timestamp =>Intern Obj=Table::inst,Sig=HUNGRY_SIG,State=Table::serving")
expect("@timestamp QF-gc    Evt<Sig=HUNGRY_SIG,Pool=1,*")
expect("@timestamp MP-Put   Obj=EvtPool1,*")
expect("@timestamp Trg-Done QS_RX_TICK")

