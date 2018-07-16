# test-script for QUTest unit testing harness
# see https://www.state-machine.com/qtools/qutest.html

import sys
import pytest
import struct
from qspypy.qspy import FILTER, QS_OBJ_KIND


def on_reset(qutest):
    """ Common reset handler called by qutest after resetting target """

    qutest.expect_pause()
    qutest.Continue()  # note continue in lower case. is a reserved word in python
    qutest.glb_filter(FILTER.SM, FILTER.AO, FILTER.UA)
    qutest.loc_filter(QS_OBJ_KIND.AO, 'AO_Table')
    qutest.current_obj(QS_OBJ_KIND.SM_AO, 'AO_Table')

# tests...


def test_PAUSE_Table(qutest):
    qutest.dispatch('PAUSE_SIG')
    qutest.expect(
        "%timestamp Disp===> Obj=AO_Table,Sig=PAUSE_SIG,State=serving")
    qutest.expect("%timestamp BSP_CALL BSP::displayPaused 1")
    qutest.expect("===RTC===> St-Entry Obj=AO_Table,State=paused")
    qutest.expect(
        "%timestamp ===>Tran Obj=AO_Table,Sig=PAUSE_SIG,State=serving->paused")
    qutest.expect("%timestamp Trg-Done QS_RX_EVENT")


def test_SERVE_Table_1(qutest):
    qutest.command(1)
    qutest.expect(
        "%timestamp Disp===> Obj=AO_Table,Sig=SERVE_SIG,State=serving")
    qutest.expect(
        "%timestamp =>Ignore Obj=AO_Table,Sig=SERVE_SIG,State=serving")
    qutest.expect("%timestamp Trg-Done QS_RX_COMMAND")


def test_SERVE_Table_2(qutest_noreset):
    qutest = qutest_noreset  # name change
    qutest.probe('BSP::displayPaused', 1)
    qutest.dispatch('PAUSE_SIG')
    qutest.expect(
        "%timestamp Disp===> Obj=AO_Table,Sig=PAUSE_SIG,State=serving")
    qutest.expect("%timestamp TstProbe Fun=BSP::displayPaused,Data=1")
    qutest.expect("%timestamp =ASSERT= Mod=bsp,Loc=100")


if __name__ == "__main__":
    options = ['-x', '-v', '--tb=short']
    options.extend(sys.argv)
    pytest.main(options)
