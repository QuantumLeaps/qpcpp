# test-script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html

# preamble...
def on_reset():
    expect_pause()
    glb_filter(GRP_ALL)
    loc_filter(IDS_ALL, -IDS_AP)
    continue_test()
    expect("@timestamp AO-Subsc Obj=Table::inst,Sig=PAUSE_SIG")
    expect("@timestamp AO-Subsc Obj=Table::inst,Sig=SERVE_SIG")
    expect("@timestamp AO-Subsc Obj=Table::inst,Sig=TEST_SIG")
    expect("@timestamp BSP_CALL BSP::displayPhilStat 0 thinking")
    expect("@timestamp BSP_CALL BSP::displayPhilStat 1 thinking")
    expect("@timestamp BSP_CALL BSP::displayPhilStat 2 thinking")
    expect("@timestamp BSP_CALL BSP::displayPhilStat 3 thinking")
    expect("@timestamp BSP_CALL BSP::displayPhilStat 4 thinking")
    expect("===RTC===> St-Init  Obj=Table::inst,State=QP::QHsm::top->Table::serving")
    expect("===RTC===> St-Entry Obj=Table::inst,State=Table::serving")
    expect("@timestamp Init===> Obj=Table::inst,State=Table::serving")
    expect_run()

# tests...
test("init")

