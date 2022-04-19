# test-script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html/qutest.html

# preamble...
def on_reset():
    expect_pause()
    continue_test()
    expect_run()
    glb_filter(GRP_ALL)
    loc_filter(IDS_ALL, -IDS_AO, -IDS_AP)
    ao_filter("Table::inst")
    current_obj(OBJ_SM_AO, "Table::inst")

def on_setup():
    print("on_setup")

def on_teardown():
    print("on_teardown")

# tests...
test("post HUNGRY[2]")
post("HUNGRY_SIG", pack("<B", 2))
expect("@timestamp MP-Get   Obj=EvtPool1,*")
expect("@timestamp QF-New   Sig=HUNGRY_SIG,*")
expect("@timestamp AO-Post  Sdr=QS_RX,Obj=Table::inst,Evt<Sig=HUNGRY_SIG,*")
expect("@timestamp QUTEST_ON_POST HUNGRY_SIG,Obj=Table::inst 2")
expect("@timestamp AO-GetL  Obj=Table::inst,Evt<Sig=HUNGRY_SIG,*")
expect("@timestamp Disp===> Obj=Table::inst,Sig=HUNGRY_SIG,State=Table::serving")
expect("@timestamp BSP_CALL BSP::displayPhilStat 2 hungry  ")
expect("@timestamp BSP_CALL BSP::displayPhilStat 2 eating  ")
expect("@timestamp =>Intern Obj=Table::inst,Sig=HUNGRY_SIG,State=Table::serving")
expect("@timestamp QF-gc    Evt<Sig=HUNGRY_SIG,*")
expect("@timestamp MP-Put   Obj=EvtPool1,*")
expect("@timestamp Trg-Done QS_RX_EVENT")

test("post HUNGRY[1]", NORESET)
post("HUNGRY_SIG", pack("<B", 1))
expect("@timestamp MP-Get   Obj=EvtPool1,*")
expect("@timestamp QF-New   Sig=HUNGRY_SIG,*")
expect("@timestamp AO-Post  Sdr=QS_RX,Obj=Table::inst,Evt<Sig=HUNGRY_SIG,*")
expect("@timestamp QUTEST_ON_POST HUNGRY_SIG,Obj=Table::inst 1")
expect("@timestamp AO-GetL  Obj=Table::inst,Evt<Sig=HUNGRY_SIG,*")
expect("@timestamp Disp===> Obj=Table::inst,Sig=HUNGRY_SIG,State=Table::serving")
expect("@timestamp BSP_CALL BSP::displayPhilStat 1 hungry  ")
expect("@timestamp =>Intern Obj=Table::inst,Sig=HUNGRY_SIG,State=Table::serving")
expect("@timestamp QF-gc    Evt<Sig=HUNGRY_SIG,*")
expect("@timestamp MP-Put   Obj=EvtPool1,*")
expect("@timestamp Trg-Done QS_RX_EVENT")

test("publish DONE[2]", NORESET)
publish("DONE_SIG", pack("<B", 2))
expect("@timestamp MP-Get   Obj=EvtPool1,*")
expect("@timestamp QF-New   Sig=DONE_SIG,*")
expect("@timestamp QF-Pub   Sdr=QS_RX,Evt<Sig=DONE_SIG,*")
expect("@timestamp QF-gc    Evt<Sig=DONE_SIG,*")
expect("@timestamp MP-Put   Obj=EvtPool1,*")
expect("@timestamp Trg-Done QS_RX_EVENT")

