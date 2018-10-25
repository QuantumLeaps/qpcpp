# test-script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html

import sys
import pytest
import struct
from qspypy.qspy import *

def on_reset(qutest):
    """ Common reset handler called by qutest after resetting target """

    qutest.expect_pause()
    qutest.glb_filter(FILTER.ON)
    qutest.Continue()  # note continue in lower case. is a reserved word in python
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<0>,Sig=EAT_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<0>,Sig=TEST_SIG")
    qutest.expect("===RTC===> St-Init  Obj=AO_Philo<0>,State=QP::QHsm::top->thinking")
    qutest.expect("%timestamp BSP_CALL BSP::random *")
    qutest.expect("%timestamp TE0-Arm  Obj=l_philo<0>.m_timeEvt,AO=AO_Philo<0>,Tim=*,Int=0")
    qutest.expect("===RTC===> St-Entry Obj=AO_Philo<0>,State=thinking")
    qutest.expect("%timestamp Init===> Obj=AO_Philo<0>,State=thinking")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<1>,Sig=EAT_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<1>,Sig=TEST_SIG")
    qutest.expect("===RTC===> St-Init  Obj=AO_Philo<1>,State=QP::QHsm::top->thinking")
    qutest.expect("%timestamp BSP_CALL BSP::random *")
    qutest.expect("%timestamp TE0-Arm  Obj=l_philo<1>.m_timeEvt,AO=AO_Philo<1>,Tim=*,Int=0")
    qutest.expect("===RTC===> St-Entry Obj=AO_Philo<1>,State=thinking")
    qutest.expect("%timestamp Init===> Obj=AO_Philo<1>,State=thinking")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<2>,Sig=EAT_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<2>,Sig=TEST_SIG")
    qutest.expect("===RTC===> St-Init  Obj=AO_Philo<2>,State=QP::QHsm::top->thinking")
    qutest.expect("%timestamp BSP_CALL BSP::random *")
    qutest.expect("%timestamp TE0-Arm  Obj=l_philo<2>.m_timeEvt,AO=AO_Philo<2>,Tim=*,Int=0")
    qutest.expect("===RTC===> St-Entry Obj=AO_Philo<2>,State=thinking")
    qutest.expect("%timestamp Init===> Obj=AO_Philo<2>,State=thinking")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<3>,Sig=EAT_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<3>,Sig=TEST_SIG")
    qutest.expect("===RTC===> St-Init  Obj=AO_Philo<3>,State=QP::QHsm::top->thinking")
    qutest.expect("%timestamp BSP_CALL BSP::random *")
    qutest.expect("%timestamp TE0-Arm  Obj=l_philo<3>.m_timeEvt,AO=AO_Philo<3>,Tim=*,Int=0")
    qutest.expect("===RTC===> St-Entry Obj=AO_Philo<3>,State=thinking")
    qutest.expect("%timestamp Init===> Obj=AO_Philo<3>,State=thinking")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<4>,Sig=EAT_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Philo<4>,Sig=TEST_SIG")
    qutest.expect("===RTC===> St-Init  Obj=AO_Philo<4>,State=QP::QHsm::top->thinking")
    qutest.expect("%timestamp BSP_CALL BSP::random *")
    qutest.expect("%timestamp TE0-Arm  Obj=l_philo<4>.m_timeEvt,AO=AO_Philo<4>,Tim=*,Int=0")
    qutest.expect("===RTC===> St-Entry Obj=AO_Philo<4>,State=thinking")
    qutest.expect("%timestamp Init===> Obj=AO_Philo<4>,State=thinking")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Table,Sig=DONE_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Table,Sig=PAUSE_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Table,Sig=SERVE_SIG")
    qutest.expect("%timestamp AO-Subsc Obj=AO_Table,Sig=TEST_SIG")
    qutest.expect("%timestamp BSP_CALL BSP::displayPhilStat 0 thinking")
    qutest.expect("%timestamp BSP_CALL BSP::displayPhilStat 1 thinking")
    qutest.expect("%timestamp BSP_CALL BSP::displayPhilStat 2 thinking")
    qutest.expect("%timestamp BSP_CALL BSP::displayPhilStat 3 thinking")
    qutest.expect("%timestamp BSP_CALL BSP::displayPhilStat 4 thinking")
    qutest.expect("===RTC===> St-Init  Obj=AO_Table,State=QP::QHsm::top->serving")
    qutest.expect("===RTC===> St-Entry Obj=AO_Table,State=serving")
    qutest.expect("%timestamp Init===> Obj=AO_Table,State=serving")

def test_DPP_init(qutest):
    pass # empty function


if __name__ == "__main__":
    # stop on first failure, verbose output but small stack trace
    options = ['-x', '-v', '--tb=short']
    options.extend(sys.argv)
    pytest.main(options)

