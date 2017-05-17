# QUTEST test of QMsmTst structural

# preamble...
proc on_reset {} {
    glb_filter SM
    current_obj SM the_msm
}

# tests...
test "QMsmTst init"
init
expect "===RTC===> St-Init  Obj=the_msm,State=NULL->QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s2->QMsmTst::s211"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp Init===> Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

#------------------
test "QMsmTst dispatch" -noreset

dispatch A_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=A_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s21->QMsmTst::s211"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=A_SIG,State=QMsmTst::s21->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch B_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=B_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=B_SIG,State=QMsmTst::s21->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch D_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=D_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s21->QMsmTst::s211"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=D_SIG,State=QMsmTst::s211->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch E_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=E_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=E_SIG,State=QMsmTst::s->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch I_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=I_SIG,State=QMsmTst::s11"
expect "%timestamp =>Intern Obj=the_msm,Sig=I_SIG,State=QMsmTst::s1"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch F_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=F_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=F_SIG,State=QMsmTst::s1->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch I_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=I_SIG,State=QMsmTst::s211"
expect "%timestamp =>Intern Obj=the_msm,Sig=I_SIG,State=QMsmTst::s2"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch I_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=I_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Unhnd Obj=the_msm,Sig=I_SIG,State=QMsmTst::s2"
expect "%timestamp =>Intern Obj=the_msm,Sig=I_SIG,State=QMsmTst::s"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch F_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=F_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=F_SIG,State=QMsmTst::s2->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch A_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=A_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s1->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=A_SIG,State=QMsmTst::s1->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch B_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=B_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=B_SIG,State=QMsmTst::s1->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch D_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=D_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Unhnd Obj=the_msm,Sig=D_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=D_SIG,State=QMsmTst::s1->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch D_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=D_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s1->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=D_SIG,State=QMsmTst::s11->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch E_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=E_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=E_SIG,State=QMsmTst::s->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch G_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=G_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=G_SIG,State=QMsmTst::s11->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch H_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=H_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=H_SIG,State=QMsmTst::s211->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch H_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=H_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=H_SIG,State=QMsmTst::s11->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch C_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=C_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s2->QMsmTst::s211"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=C_SIG,State=QMsmTst::s1->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch G_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=G_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s1->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=G_SIG,State=QMsmTst::s21->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch C_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=C_SIG,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s11"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s2->QMsmTst::s211"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s211"
expect "%timestamp ===>Tran Obj=the_msm,Sig=C_SIG,State=QMsmTst::s1->QMsmTst::s211"
expect "%timestamp Trg-Done QS_RX_EVENT"

dispatch C_SIG
expect "%timestamp Disp===> Obj=the_msm,Sig=C_SIG,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s211"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s21"
expect "===RTC===> St-Exit  Obj=the_msm,State=QMsmTst::s2"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s1"
expect "===RTC===> St-Init  Obj=the_msm,State=QMsmTst::s1->QMsmTst::s11"
expect "===RTC===> St-Entry Obj=the_msm,State=QMsmTst::s11"
expect "%timestamp ===>Tran Obj=the_msm,Sig=C_SIG,State=QMsmTst::s2->QMsmTst::s11"
expect "%timestamp Trg-Done QS_RX_EVENT"

# the end
end

