/** @page metrics Code Metrics 

@code 
                   Standard Code Metrics for QP/C++ 5.6.1 

           Resource Standard Metrics (TM) for C, C++, C# and Java
                   Version 7.75 - mSquaredTechnologies.com

  License Type: Windows Single User License
  Licensed To : Quantum Leaps, LLC
  License No. : WS2975                          License Date: Dec 15, 2013
  Build Date  : Sep  2 2009                         Run Date: Dec 31, 2015
  (C)1996-2009 M Squared Technologies LLC
  ________________________________________________________________________

                           ~~ Function Metrics ~~
                      ~~ Complexity Detail Analysis ~~

  File: ..\include\qassert.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 43       eLOC 42       lLOC 3        Comment 240      Lines      303
  ________________________________________________________________________
  End of File: ..\include\qassert.h


  File: ..\include\qep.h
  ________________________________________________________________________

  Inline Function: QP::QEvt::QEvt
  Parameters: (QSignal const s)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 0        Comment 2        Lines        1

  Inline Function: QP::QEvt::~QEvt
  Parameters: ()
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 0        Comment 1        Lines        1

  Inline Function: QP::QMsm::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Inline Function: QP::QMsm::stateObj
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QMsm::qm_tran_
  Parameters: (QMTranActTable const * const tatbl) Non-ANSI Parameter Cons
              truct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Inline Function: QP::QMsm::qm_tran_hist_
  Parameters: (QMState const * const hist, QMTranActTable const * const ta
              tbl)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 5        eLOC 3        lLOC 3        Comment 1        Lines        5

  Inline Function: QP::QMsm::qm_tran_init_
  Parameters: (QMTranActTable const * const tatbl)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QMsm::qm_tran_ep_
  Parameters: (QMTranActTable const * const tatbl)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Inline Function: QP::QMsm::qm_tran_xp_
  Parameters: (QActionHandler const xp, QMTranActTable const *const tatbl)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 5        eLOC 3        lLOC 3        Comment 2        Lines        5

  Inline Function: QP::QMsm::qm_entry_
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QMsm::qm_exit_
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QMsm::qm_super_sub_
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Inline Function: QP::QMsm::QM_HANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QMsm::QM_UNHANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QMsm::QM_SUPER
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QMsm::Q_HANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 3        Lines        3

  Inline Function: QP::QMsm::Q_UNHANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 4        Lines        3

  Inline Function: QP::QMsm::tran_
  Parameters: (QStateHandler const target)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QMsm::tran_hist_
  Parameters: (QStateHandler const hist)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QMsm::super_
  Parameters: (QStateHandler const superstate)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QHsm::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Inline Function: QP::QHsm::state
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QEP::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 250      eLOC 218      lLOC 127      Comment 307      Lines      640
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:         23
  Total Function LOC.....:         74  Total Function Pts LOC :        0.0
  Total Function eLOC....:         53  Total Function Pts eLOC:        0.0
  Total Function lLOC....:         34  Total Function Pts lLOC:        0.0
  Total Function Params .:         15  Total Function Return .:         23
  Total Cyclo Complexity :         23  Total Function Complex.:         61
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          5  Average Function LOC ..:       3.22
  Max Function eLOC .....:          3  Average Function eLOC .:       2.30
  Max Function lLOC .....:          3  Average Function lLOC .:       1.48
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       0.65
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       1.65
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          4  Avg Total Complexity ..:       2.65
  ________________________________________________________________________
  End of File: ..\include\qep.h


  File: ..\include\qequeue.h
  ________________________________________________________________________

  Inline Function: QP::QEQueue::getNFree
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 10       Lines        3

  Inline Function: QP::QEQueue::isEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 10       Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 45       eLOC 41       lLOC 23       Comment 184      Lines      256
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:          6  Total Function Pts LOC :        0.0
  Total Function eLOC....:          4  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          2  Total Function Pts lLOC:        0.0
  Total Function Params .:          0  Total Function Return .:          2
  Total Cyclo Complexity :          2  Total Function Complex.:          4
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       3.00
  Max Function eLOC .....:          2  Average Function eLOC .:       2.00
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          0  Avg Function Parameters:       0.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          1  Avg Interface Complex. :       1.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          2  Avg Total Complexity ..:       2.00
  ________________________________________________________________________
  End of File: ..\include\qequeue.h


  File: ..\include\qf.h
  ________________________________________________________________________

  Inline Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              )
  Complexity   Param 5       Return 1      Cyclo Vg 1       Total        7
  LOC 4        eLOC 2        lLOC 1        Comment 1        Lines        4

  Inline Function: QP::QMActive::getPrio
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QMActive::setPrio
  Parameters: (uint_fast8_t const prio)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QMActive::getOsObject
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Inline Function: QP::QMActive::getThread
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Inline Function: QP::QTimeEvt::QTimeEvt
  Parameters: (enum_t const sgnl) Non-ANSI Parameter Construct Defined By 
              Compiler Directive
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 5        lLOC 3        Comment 5        Lines        8

  Inline Function: QP::QTimeEvt::postIn
  Parameters: (QMActive * const act, QTimeEvtCtr const nTicks)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QTimeEvt::postEvery
  Parameters: (QMActive * const act, QTimeEvtCtr const nTicks)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Inline Function: QP::QTimeEvt::toActive
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Inline Function: QP::QTimeEvt::toTimeEvt
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Inline Function: QP::QF::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 251      eLOC 236      lLOC 104      Comment 477      Lines      832
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:         11
  Total Function LOC.....:         32  Total Function Pts LOC :        0.0
  Total Function eLOC....:         23  Total Function Pts eLOC:        0.0
  Total Function lLOC....:         15  Total Function Pts lLOC:        0.0
  Total Function Params .:         12  Total Function Return .:         11
  Total Cyclo Complexity :         11  Total Function Complex.:         34
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          7  Average Function LOC ..:       2.91
  Max Function eLOC .....:          5  Average Function eLOC .:       2.09
  Max Function lLOC .....:          3  Average Function lLOC .:       1.36
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          5  Avg Function Parameters:       1.09
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          6  Avg Interface Complex. :       2.09
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          7  Avg Total Complexity ..:       3.09
  ________________________________________________________________________
  End of File: ..\include\qf.h


  File: ..\include\qk.h
  ________________________________________________________________________

  Inline Function: QP::QK::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 68       eLOC 63       lLOC 19       Comment 101      Lines      186
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          1
  Total Function LOC.....:          3  Total Function Pts LOC :        0.0
  Total Function eLOC....:          2  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          1  Total Function Pts lLOC:        0.0
  Total Function Params .:          0  Total Function Return .:          1
  Total Cyclo Complexity :          1  Total Function Complex.:          2
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       3.00
  Max Function eLOC .....:          2  Average Function eLOC .:       2.00
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          0  Avg Function Parameters:       0.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          1  Avg Interface Complex. :       1.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          2  Avg Total Complexity ..:       2.00
  ________________________________________________________________________
  End of File: ..\include\qk.h


  File: ..\include\qmpool.h
  ________________________________________________________________________

  Inline Function: QP::QMPool::getBlockSize
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 54       eLOC 51       lLOC 23       Comment 94       Lines      166
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          1
  Total Function LOC.....:          3  Total Function Pts LOC :        0.0
  Total Function eLOC....:          2  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          1  Total Function Pts lLOC:        0.0
  Total Function Params .:          0  Total Function Return .:          1
  Total Cyclo Complexity :          1  Total Function Complex.:          2
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       3.00
  Max Function eLOC .....:          2  Average Function eLOC .:       2.00
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          0  Avg Function Parameters:       0.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          1  Avg Interface Complex. :       1.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          2  Avg Total Complexity ..:       2.00
  ________________________________________________________________________
  End of File: ..\include\qmpool.h


  File: ..\include\qp_port.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 4        eLOC 4        lLOC 0        Comment 38       Lines       44
  ________________________________________________________________________
  End of File: ..\include\qp_port.h


  File: ..\include\qpcpp.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 40       eLOC 39       lLOC 1        Comment 89       Lines      144
  ________________________________________________________________________
  End of File: ..\include\qpcpp.h


  File: ..\include\qpset.h
  ________________________________________________________________________

  Inline Function: QP::QPSet8::isEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QPSet8::notEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QPSet8::hasElement
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 5        eLOC 4        lLOC 1        Comment 1        Lines        5

  Inline Function: QP::QPSet8::insert
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QPSet8::remove
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QPSet8::findMax
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QPSet64::isEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QPSet64::notEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Inline Function: QP::QPSet64::hasElement
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 6        lLOC 2        Comment 1        Lines        7

  Inline Function: QP::QPSet64::insert
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 8        eLOC 7        lLOC 3        Comment 1        Lines        8

  Inline Function: QP::QPSet64::remove
  Parameters: (uint_fast8_t const n)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 9        eLOC 7        lLOC 3        Comment 1        Lines        9

  Inline Function: QP::QPSet64::findMax
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 15       eLOC 12       lLOC 5        Comment 1        Lines       15

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 87       eLOC 69       lLOC 30       Comment 121      Lines      236
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:         12
  Total Function LOC.....:         65  Total Function Pts LOC :        0.0
  Total Function eLOC....:         50  Total Function Pts eLOC:        0.0
  Total Function lLOC....:         21  Total Function Pts lLOC:        0.0
  Total Function Params .:          6  Total Function Return .:         12
  Total Cyclo Complexity :         14  Total Function Complex.:         32
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         15  Average Function LOC ..:       5.42
  Max Function eLOC .....:         12  Average Function eLOC .:       4.17
  Max Function lLOC .....:          5  Average Function lLOC .:       1.75
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       0.50
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       1.50
  Max Cyclomatic Complex.:          2  Avg Cyclomatic Complex.:       1.17
  Max Total Complexity ..:          4  Avg Total Complexity ..:       2.67
  ________________________________________________________________________
  End of File: ..\include\qpset.h


  File: ..\include\qs.h
  ________________________________________________________________________

  Inline Function: QP::QS::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QS::rxPut
  Parameters: (uint8_t const b)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 3     
  Complexity   Param 1       Return 1      Cyclo Vg 4       Total        6
  LOC 14       eLOC 9        lLOC 4        Comment 1        Lines       14


    NOTICE: The end of the source file has been reached where the
            open brace count { 26 != 24 } close brace count 
            This is an indication of non-compilable code within the source
            file.  RSM processes all source code including code wrapped
            with preprocessor directives.  Accurate metrics requires that
            all code present in this file must be compilable.
  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 452      eLOC 439      lLOC 107      Comment 555      Lines     1029
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:         17  Total Function Pts LOC :        0.0
  Total Function eLOC....:         11  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          5  Total Function Pts lLOC:        0.0
  Total Function Params .:          1  Total Function Return .:          2
  Total Cyclo Complexity :          5  Total Function Complex.:          8
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         14  Average Function LOC ..:       8.50
  Max Function eLOC .....:          9  Average Function eLOC .:       5.50
  Max Function lLOC .....:          4  Average Function lLOC .:       2.50
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       0.50
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       1.50
  Max Cyclomatic Complex.:          4  Avg Cyclomatic Complex.:       2.50
  Max Total Complexity ..:          6  Avg Total Complexity ..:       4.00
  ________________________________________________________________________
  End of File: ..\include\qs.h


  File: ..\include\qs_dummy.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 68       eLOC 68       lLOC 0        Comment 44       Lines      122
  ________________________________________________________________________
  End of File: ..\include\qs_dummy.h


  File: ..\include\qv.h
  ________________________________________________________________________

  Inline Function: QP::QV::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 39       eLOC 35       lLOC 5        Comment 80       Lines      129
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          1
  Total Function LOC.....:          3  Total Function Pts LOC :        0.0
  Total Function eLOC....:          2  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          1  Total Function Pts lLOC:        0.0
  Total Function Params .:          0  Total Function Return .:          1
  Total Cyclo Complexity :          1  Total Function Complex.:          2
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       3.00
  Max Function eLOC .....:          2  Average Function eLOC .:       2.00
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          0  Avg Function Parameters:       0.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          1  Avg Interface Complex. :       1.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          2  Avg Total Complexity ..:       2.00
  ________________________________________________________________________
  End of File: ..\include\qv.h


  File: ..\include\qxk.h
  ________________________________________________________________________

  Inline Function: QP::QXK::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 84       eLOC 75       lLOC 30       Comment 118      Lines      218
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          1
  Total Function LOC.....:          3  Total Function Pts LOC :        0.0
  Total Function eLOC....:          2  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          1  Total Function Pts lLOC:        0.0
  Total Function Params .:          0  Total Function Return .:          1
  Total Cyclo Complexity :          1  Total Function Complex.:          2
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       3.00
  Max Function eLOC .....:          2  Average Function eLOC .:       2.00
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          0  Avg Function Parameters:       0.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          1  Avg Interface Complex. :       1.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          2  Avg Total Complexity ..:       2.00
  ________________________________________________________________________
  End of File: ..\include\qxk.h


  File: ..\include\qxthread.h
  ________________________________________________________________________

  Inline Function: QP::QXThread::init
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Inline Function: QP::QXThread::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              )
  Complexity   Param 5       Return 1      Cyclo Vg 1       Total        7
  LOC 4        eLOC 2        lLOC 1        Comment 1        Lines        4

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 61       eLOC 56       lLOC 29       Comment 85       Lines      168
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:          5  Total Function Pts LOC :        0.0
  Total Function eLOC....:          3  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          2  Total Function Pts lLOC:        0.0
  Total Function Params .:          6  Total Function Return .:          2
  Total Cyclo Complexity :          2  Total Function Complex.:         10
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          4  Average Function LOC ..:       2.50
  Max Function eLOC .....:          2  Average Function eLOC .:       1.50
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          5  Avg Function Parameters:       3.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          6  Avg Interface Complex. :       4.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          7  Avg Total Complexity ..:       5.00
  ________________________________________________________________________
  End of File: ..\include\qxthread.h


  File: ..\source\qf_pkg.h
  ________________________________________________________________________

  Inline Function: QP::QF_EVT_POOL_ID_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Inline Function: QP::QF_EVT_REF_CTR_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Inline Function: QP::QF_EVT_REF_CTR_INC_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Inline Function: QP::QF_EVT_REF_CTR_DEC_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 71       eLOC 67       lLOC 11       Comment 109      Lines      205
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          4
  Total Function LOC.....:          8  Total Function Pts LOC :        0.0
  Total Function eLOC....:          6  Total Function Pts eLOC:        0.0
  Total Function lLOC....:          4  Total Function Pts lLOC:        0.0
  Total Function Params .:          4  Total Function Return .:          4
  Total Cyclo Complexity :          4  Total Function Complex.:         12
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       2.00
  Max Function eLOC .....:          2  Average Function eLOC .:       1.50
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          3  Avg Total Complexity ..:       3.00
  ________________________________________________________________________
  End of File: ..\source\qf_pkg.h


  File: ..\source\qk_pkg.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 4        eLOC 4        lLOC 0        Comment 40       Lines       47
  ________________________________________________________________________
  End of File: ..\source\qk_pkg.h


  File: ..\source\qs_pkg.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 28       eLOC 25       lLOC 10       Comment 59       Lines       98
  ________________________________________________________________________
  End of File: ..\source\qs_pkg.h


  File: ..\source\qxk_pkg.h
  ________________________________________________________________________

                          ~~ Total File Summary ~~

  LOC 24       eLOC 21       lLOC 8        Comment 50       Lines       86
  ________________________________________________________________________
  End of File: ..\source\qxk_pkg.h


  File: ..\source\qep_hsm.cpp
  ________________________________________________________________________

  Function: QP::QHsm::QHsm
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 1        lLOC 1        Comment 8        Lines        3

  Function: QP::QHsm::init
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 3     
      Logical and ( && )      : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 38       eLOC 36       lLOC 26       Comment 25       Lines       57

  Function: QP::QHsm::top
  Parameters: (void * const, QEvt const * const)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 3        eLOC 2        lLOC 1        Comment 15       Lines        3

  Function: QP::QHsm::dispatch
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 5     
      Loops for / foreach     : 1     
      Conditional if / else if: 5     
      Logical or ( || )       : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 13      Total       15
  LOC 111      eLOC 98       lLOC 63       Comment 67       Lines      152

  Function: QP::QHsm::hsm_tran
  Parameters: (QStateHandler (&path)[MAX_NEST_DEPTH_])
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 4     
      Conditional if / else if: 10    
  Complexity   Param 1       Return 1      Cyclo Vg 15      Total       17
  LOC 98       eLOC 78       lLOC 48       Comment 61       Lines      139

  Function: QP::QHsm::isIn
  Parameters: (QStateHandler const s)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 16       eLOC 13       lLOC 9        Comment 21       Lines       23

  Function: QP::QHsm::childState
  Parameters: (QStateHandler const parent)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 19       eLOC 16       lLOC 12       Comment 27       Lines       25

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 339      eLOC 292      lLOC 166      Comment 282      Lines      599
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          7
  Total Function LOC.....:        288  Total Function Pts LOC :        6.4
  Total Function eLOC....:        244  Total Function Pts eLOC:        5.5
  Total Function lLOC....:        160  Total Function Pts lLOC:        3.1
  Total Function Params .:          8  Total Function Return .:          7
  Total Cyclo Complexity :         41  Total Function Complex.:         56
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:        111  Average Function LOC ..:      41.14
  Max Function eLOC .....:         98  Average Function eLOC .:      34.86
  Max Function lLOC .....:         63  Average Function lLOC .:      22.86
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.14
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.14
  Max Cyclomatic Complex.:         15  Avg Cyclomatic Complex.:       5.86
  Max Total Complexity ..:         17  Avg Total Complexity ..:       8.00
  ________________________________________________________________________
  End of File: ..\source\qep_hsm.cpp


  File: ..\source\qep_msm.cpp
  ________________________________________________________________________

  Function: QP::QMsm::QMsm
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 13       Lines        4

  Function: QP::QMsm::~QMsm
  Parameters: ()
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 2        eLOC 1        lLOC 0        Comment 4        Lines        2

  Function: QP::QMsm::init
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Logical and ( && )      : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 21       eLOC 20       lLOC 12       Comment 23       Lines       33

  Function: QP::QMsm::isInState
  Parameters: (QMState const * const state)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 13       eLOC 9        lLOC 5        Comment 15       Lines       14

  Function: QP::QMsm::dispatch
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 2     
      Conditional if / else if: 12    
  Complexity   Param 1       Return 1      Cyclo Vg 15      Total       17
  LOC 107      eLOC 91       lLOC 56       Comment 66       Lines      147

  Function: QP::QMsm::execTatbl_
  Parameters: (QMTranActTable const * const tatbl)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 6     
  Complexity   Param 1       Return 1      Cyclo Vg 8       Total       10
  LOC 53       eLOC 43       lLOC 21       Comment 31       Lines       64

  Function: QP::QMsm::exitToTranSource_
  Parameters: (QMState const *s, QMState const * const ts)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Conditional if / else if: 3     
  Complexity   Param 2       Return 1      Cyclo Vg 5       Total        8
  LOC 24       eLOC 16       lLOC 7        Comment 19       Lines       32

  Function: QP::QMsm::enterHistory_
  Parameters: (QMState const * const hist)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Loops for / foreach     : 1     
      Conditional if / else if: 2     
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 37       eLOC 31       lLOC 21       Comment 21       Lines       45

  Function: QP::QMsm::childStateObj
  Parameters: (QMState const * const parent)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 19       eLOC 14       lLOC 9        Comment 17       Lines       23

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 302      eLOC 248      lLOC 135      Comment 260      Lines      541
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          9
  Total Function LOC.....:        280  Total Function Pts LOC :        5.7
  Total Function eLOC....:        228  Total Function Pts eLOC:        4.7
  Total Function lLOC....:        133  Total Function Pts lLOC:        2.5
  Total Function Params .:          9  Total Function Return .:          9
  Total Cyclo Complexity :         44  Total Function Complex.:         62
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:        107  Average Function LOC ..:      31.11
  Max Function eLOC .....:         91  Average Function eLOC .:      25.33
  Max Function lLOC .....:         56  Average Function lLOC .:      14.78
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:         15  Avg Cyclomatic Complex.:       4.89
  Max Total Complexity ..:         17  Avg Total Complexity ..:       6.89
  ________________________________________________________________________
  End of File: ..\source\qep_msm.cpp


  File: ..\source\qf_act.cpp
  ________________________________________________________________________

  Function: QP::QF::add_
  Parameters: (QMActive * const a)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 15       eLOC 14       lLOC 8        Comment 17       Lines       20

  Function: QP::QF::remove_
  Parameters: (QMActive const * const a)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 15       eLOC 14       lLOC 8        Comment 17       Lines       20

  Function: QP::QF::bzero
  Parameters: (void * const start, uint_fast16_t len)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 8        eLOC 6        lLOC 4        Comment 13       Lines        8

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 243      eLOC 234      lLOC 25       Comment 108      Lines      356
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          3
  Total Function LOC.....:         38  Total Function Pts LOC :        4.6
  Total Function eLOC....:         34  Total Function Pts eLOC:        4.4
  Total Function lLOC....:         20  Total Function Pts lLOC:        0.5
  Total Function Params .:          4  Total Function Return .:          3
  Total Cyclo Complexity :          8  Total Function Complex.:         15
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         15  Average Function LOC ..:      12.67
  Max Function eLOC .....:         14  Average Function eLOC .:      11.33
  Max Function lLOC .....:          8  Average Function lLOC .:       6.67
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.33
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.33
  Max Cyclomatic Complex.:          3  Avg Cyclomatic Complex.:       2.67
  Max Total Complexity ..:          5  Avg Total Complexity ..:       5.00
  ________________________________________________________________________
  End of File: ..\source\qf_act.cpp


  File: ..\source\qf_actq.cpp
  ________________________________________________________________________

  Function: QP::QMActive::post_
  Parameters: (QEvt const * const e, uint_fast16_t const margin)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 5     
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 55       eLOC 46       lLOC 33       Comment 72       Lines       75

  Function: QP::QMActive::postLIFO
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 35       eLOC 29       lLOC 20       Comment 32       Lines       47

  Function: QP::QMActive::get_
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 36       eLOC 32       lLOC 22       Comment 27       Lines       48

  Function: QP::QF::getQueueMin
  Parameters: (uint_fast8_t const prio)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 10       eLOC 9        lLOC 5        Comment 17       Lines       13

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 154      eLOC 133      lLOC 80       Comment 199      Lines      327
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          4
  Total Function LOC.....:        136  Total Function Pts LOC :        2.9
  Total Function eLOC....:        116  Total Function Pts eLOC:        2.5
  Total Function lLOC....:         80  Total Function Pts lLOC:        1.5
  Total Function Params .:          4  Total Function Return .:          4
  Total Cyclo Complexity :         16  Total Function Complex.:         24
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         55  Average Function LOC ..:      34.00
  Max Function eLOC .....:         46  Average Function eLOC .:      29.00
  Max Function lLOC .....:         33  Average Function lLOC .:      20.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:          6  Avg Cyclomatic Complex.:       4.00
  Max Total Complexity ..:          9  Avg Total Complexity ..:       6.00
  ________________________________________________________________________
  End of File: ..\source\qf_actq.cpp


  File: ..\source\qf_defer.cpp
  ________________________________________________________________________

  Function: QP::QMActive::defer
  Parameters: (QEQueue * const eq, QEvt const * const e)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 3        eLOC 2        lLOC 1        Comment 21       Lines        3

  Function: QP::QMActive::recall
  Parameters: (QEQueue * const eq)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 15       eLOC 12       lLOC 8        Comment 29       Lines       27

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 25       eLOC 20       lLOC 9        Comment 92       Lines      120
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:         18  Total Function Pts LOC :        0.5
  Total Function eLOC....:         14  Total Function Pts eLOC:        0.4
  Total Function lLOC....:          9  Total Function Pts lLOC:        0.2
  Total Function Params .:          3  Total Function Return .:          2
  Total Cyclo Complexity :          4  Total Function Complex.:          9
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         15  Average Function LOC ..:       9.00
  Max Function eLOC .....:         12  Average Function eLOC .:       7.00
  Max Function lLOC .....:          8  Average Function lLOC .:       4.50
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.50
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.50
  Max Cyclomatic Complex.:          3  Avg Cyclomatic Complex.:       2.00
  Max Total Complexity ..:          5  Avg Total Complexity ..:       4.50
  ________________________________________________________________________
  End of File: ..\source\qf_defer.cpp


  File: ..\source\qf_dyn.cpp
  ________________________________________________________________________

  Function: QP::QF::poolInit
  Parameters: (void * const poolSto, uint_fast32_t const poolSize, uint_fa
              st16_t const evtSize)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical or ( || )       : 1     
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 10       eLOC 8        lLOC 4        Comment 36       Lines       13

  Function: QP::QF::newX_
  Parameters: (uint_fast16_t const evtSize, uint_fast16_t const margin, en
              um_t const sig)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 2     
  Complexity   Param 3       Return 1      Cyclo Vg 4       Total        8
  LOC 27       eLOC 21       lLOC 14       Comment 33       Lines       38

  Function: QP::QF::gc
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 32       eLOC 28       lLOC 14       Comment 39       Lines       47

  Function: QP::QF::newRef_
  Parameters: (QEvt const * const e, QEvt const * const evtRef)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 7        eLOC 5        lLOC 3        Comment 15       Lines       11

  Function: QP::QF::poolGetMaxBlockSize
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 4        eLOC 3        lLOC 1        Comment 4        Lines        4

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 98       eLOC 82       lLOC 38       Comment 175      Lines      272
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          5
  Total Function LOC.....:         80  Total Function Pts LOC :        1.8
  Total Function eLOC....:         65  Total Function Pts eLOC:        1.5
  Total Function lLOC....:         36  Total Function Pts lLOC:        0.7
  Total Function Params .:          9  Total Function Return .:          5
  Total Cyclo Complexity :         12  Total Function Complex.:         26
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         32  Average Function LOC ..:      16.00
  Max Function eLOC .....:         28  Average Function eLOC .:      13.00
  Max Function lLOC .....:         14  Average Function lLOC .:       7.20
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          3  Avg Function Parameters:       1.80
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          4  Avg Interface Complex. :       2.80
  Max Cyclomatic Complex.:          4  Avg Cyclomatic Complex.:       2.40
  Max Total Complexity ..:          8  Avg Total Complexity ..:       5.20
  ________________________________________________________________________
  End of File: ..\source\qf_dyn.cpp


  File: ..\source\qf_mem.cpp
  ________________________________________________________________________

  Function: QP::QMPool::QMPool
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 0        Comment 8        Lines        1

  Function: QP::QMPool::init
  Parameters: (void * const poolSto, uint_fast32_t poolSize, uint_fast16_t
               blockSize)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 2     
      Logical and ( && )      : 2     
  Complexity   Param 3       Return 1      Cyclo Vg 5       Total        9
  LOC 35       eLOC 31       lLOC 22       Comment 51       Lines       58

  Function: QP::QMPool::put
  Parameters: (void * const b)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 16       eLOC 15       lLOC 9        Comment 23       Lines       23

  Function: QP::QMPool::get
  Parameters: (uint_fast16_t const margin)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 3     
  Complexity   Param 1       Return 1      Cyclo Vg 4       Total        6
  LOC 40       eLOC 34       lLOC 22       Comment 46       Lines       60

  Function: QP::QF::getPoolMin
  Parameters: (uint_fast8_t const poolId)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 10       eLOC 9        lLOC 5        Comment 13       Lines       14

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 124      eLOC 111      lLOC 58       Comment 186      Lines      307
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          5
  Total Function LOC.....:        102  Total Function Pts LOC :        2.3
  Total Function eLOC....:         90  Total Function Pts eLOC:        2.1
  Total Function lLOC....:         58  Total Function Pts lLOC:        1.1
  Total Function Params .:          6  Total Function Return .:          5
  Total Cyclo Complexity :         14  Total Function Complex.:         25
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         40  Average Function LOC ..:      20.40
  Max Function eLOC .....:         34  Average Function eLOC .:      18.00
  Max Function lLOC .....:         22  Average Function lLOC .:      11.60
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          3  Avg Function Parameters:       1.20
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          4  Avg Interface Complex. :       2.20
  Max Cyclomatic Complex.:          5  Avg Cyclomatic Complex.:       2.80
  Max Total Complexity ..:          9  Avg Total Complexity ..:       5.00
  ________________________________________________________________________
  End of File: ..\source\qf_mem.cpp


  File: ..\source\qf_ps.cpp
  ________________________________________________________________________

  Function: QP::QF::psInit
  Parameters: (QSubscrList * const subscrSto, enum_t const maxSignal)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 7        eLOC 6        lLOC 3        Comment 30       Lines       11

  Function: QP::QF::publish_
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 3     
      Loops for / foreach     : 1     
      Conditional if / else if: 2     
      Logical and ( && )      : 10    
  Complexity   Param 1       Return 1      Cyclo Vg 17      Total       19
  LOC 106      eLOC 95       lLOC 52       Comment 105      Lines      207


    NOTICE: The end of the source file has been reached where the
            open brace count { 13 != 12 } close brace count 
            This is an indication of non-compilable code within the source
            file.  RSM processes all source code including code wrapped
            with preprocessor directives.  Accurate metrics requires that
            all code present in this file must be compilable.
  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 127      eLOC 115      lLOC 57       Comment 183      Lines      321
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:        113  Total Function Pts LOC :        2.4
  Total Function eLOC....:        101  Total Function Pts eLOC:        2.2
  Total Function lLOC....:         55  Total Function Pts lLOC:        1.1
  Total Function Params .:          3  Total Function Return .:          2
  Total Cyclo Complexity :         18  Total Function Complex.:         23
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:        106  Average Function LOC ..:      56.50
  Max Function eLOC .....:         95  Average Function eLOC .:      50.50
  Max Function lLOC .....:         52  Average Function lLOC .:      27.50
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.50
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.50
  Max Cyclomatic Complex.:         17  Avg Cyclomatic Complex.:       9.00
  Max Total Complexity ..:         19  Avg Total Complexity ..:      11.50
  ________________________________________________________________________
  End of File: ..\source\qf_ps.cpp


  File: ..\source\qf_qact.cpp
  ________________________________________________________________________

  Function: QP::QActive::QActive
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 1        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::init
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::dispatch
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::isIn
  Parameters: (QStateHandler const s)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 22       eLOC 15       lLOC 5        Comment 51       Lines       76
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          5
  Total Function LOC.....:         15  Total Function Pts LOC :        0.4
  Total Function eLOC....:          9  Total Function Pts eLOC:        0.3
  Total Function lLOC....:          5  Total Function Pts lLOC:        0.1
  Total Function Params .:          4  Total Function Return .:          5
  Total Cyclo Complexity :          5  Total Function Complex.:         14
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          3  Average Function LOC ..:       3.00
  Max Function eLOC .....:          2  Average Function eLOC .:       1.80
  Max Function lLOC .....:          1  Average Function lLOC .:       1.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       0.80
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       1.80
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          3  Avg Total Complexity ..:       2.80
  ________________________________________________________________________
  End of File: ..\source\qf_qact.cpp


  File: ..\source\qf_qeq.cpp
  ________________________________________________________________________

  Function: QP::QEQueue::QEQueue
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 0        Comment 4        Lines        1

  Function: QP::QEQueue::init
  Parameters: (QEvt const *qSto[], uint_fast16_t const qLen)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 15       eLOC 14       lLOC 9        Comment 19       Lines       16

  Function: QP::QEQueue::post
  Parameters: (QEvt const * const e, uint_fast16_t const margin)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 5     
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 51       eLOC 43       lLOC 29       Comment 52       Lines       72

  Function: QP::QEQueue::postLIFO
  Parameters: (QEvt const * const e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 32       eLOC 27       lLOC 19       Comment 36       Lines       43

  Function: QP::QEQueue::get
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 3     
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 37       eLOC 32       lLOC 21       Comment 31       Lines       46

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 156      eLOC 136      lLOC 78       Comment 187      Lines      315
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          5
  Total Function LOC.....:        136  Total Function Pts LOC :        2.9
  Total Function eLOC....:        117  Total Function Pts eLOC:        2.6
  Total Function lLOC....:         78  Total Function Pts lLOC:        1.5
  Total Function Params .:          5  Total Function Return .:          5
  Total Cyclo Complexity :         17  Total Function Complex.:         27
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         51  Average Function LOC ..:      27.20
  Max Function eLOC .....:         43  Average Function eLOC .:      23.40
  Max Function lLOC .....:         29  Average Function lLOC .:      15.60
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:          6  Avg Cyclomatic Complex.:       3.40
  Max Total Complexity ..:          9  Avg Total Complexity ..:       5.40
  ________________________________________________________________________
  End of File: ..\source\qf_qeq.cpp


  File: ..\source\qf_qmact.cpp
  ________________________________________________________________________

  Function: QP::QMActive::QMActive
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 9        eLOC 7        lLOC 3        Comment 1        Lines       11

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 15       eLOC 12       lLOC 3        Comment 40       Lines       58
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          1
  Total Function LOC.....:          9  Total Function Pts LOC :        0.3
  Total Function eLOC....:          7  Total Function Pts eLOC:        0.2
  Total Function lLOC....:          3  Total Function Pts lLOC:        0.1
  Total Function Params .:          1  Total Function Return .:          1
  Total Cyclo Complexity :          1  Total Function Complex.:          3
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:          9  Average Function LOC ..:       9.00
  Max Function eLOC .....:          7  Average Function eLOC .:       7.00
  Max Function lLOC .....:          3  Average Function lLOC .:       3.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:          1  Avg Cyclomatic Complex.:       1.00
  Max Total Complexity ..:          3  Avg Total Complexity ..:       3.00
  ________________________________________________________________________
  End of File: ..\source\qf_qmact.cpp


  File: ..\source\qf_time.cpp
  ________________________________________________________________________

  Function: QP::QF::tickX_
  Parameters: (uint8_t const tickRate)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 5     
  Complexity   Param 1       Return 1      Cyclo Vg 7       Total        9
  LOC 65       eLOC 53       lLOC 36       Comment 51       Lines       90

  Function: QP::QF::noTimeEvtsActiveX
  Parameters: (uint8_t const tickRate)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 14       eLOC 10       lLOC 6        Comment 11       Lines       14

  Function: QP::QTimeEvt::QTimeEvt
  Parameters: (QMActive * const act, enum_t const sgnl, uint8_t const tick
              Rate) Defined By Compiler Directive
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 1     
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 9        eLOC 7        lLOC 4        Comment 22       Lines       22

  Function: QP::QTimeEvt::QTimeEvt
  Parameters: () Defined By Compiler Directive
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 5        lLOC 3        Comment 12       Lines       18

  Function: QP::QTimeEvt::armX
  Parameters: (QTimeEvtCtr const nTicks, QTimeEvtCtr const interval)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
      Logical and ( && )      : 4     
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 28       eLOC 26       lLOC 16       Comment 48       Lines       50

  Function: QP::QTimeEvt::disarm
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 29       eLOC 26       lLOC 17       Comment 27       Lines       35

  Function: QP::QTimeEvt::rearm
  Parameters: (QTimeEvtCtr const nTicks)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
      Inlined if-else ( ? : ) : 1     
      Logical and ( && )      : 3     
  Complexity   Param 1       Return 1      Cyclo Vg 7       Total        9
  LOC 37       eLOC 32       lLOC 18       Comment 47       Lines       64

  Function: QP::QTimeEvt::ctr
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 15       eLOC 14       lLOC 10       Comment 16       Lines       18

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 241      eLOC 209      lLOC 111      Comment 297      Lines      523
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          8
  Total Function LOC.....:        204  Total Function Pts LOC :        4.5
  Total Function eLOC....:        173  Total Function Pts eLOC:        3.9
  Total Function lLOC....:        110  Total Function Pts lLOC:        2.1
  Total Function Params .:          9  Total Function Return .:          8
  Total Cyclo Complexity :         29  Total Function Complex.:         46
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         65  Average Function LOC ..:      25.50
  Max Function eLOC .....:         53  Average Function eLOC .:      21.63
  Max Function lLOC .....:         36  Average Function lLOC .:      13.75
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          3  Avg Function Parameters:       1.13
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          4  Avg Interface Complex. :       2.13
  Max Cyclomatic Complex.:          7  Avg Cyclomatic Complex.:       3.63
  Max Total Complexity ..:          9  Avg Total Complexity ..:       5.75
  ________________________________________________________________________
  End of File: ..\source\qf_time.cpp


  File: ..\source\qk.cpp
  ________________________________________________________________________

  Function: QP::QF::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 13       eLOC 12       lLOC 8        Comment 18       Lines       21

  Function: QP::QF::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        4

  Function: QP::initial_events
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 7        eLOC 5        lLOC 3        Comment 6        Lines       10

  Function: QP::QF::run
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 12       eLOC 10       lLOC 7        Comment 21       Lines       15

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              , QEvt const * const ie)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 2     
  Complexity   Param 6       Return 1      Cyclo Vg 3       Total       10
  LOC 12       eLOC 10       lLOC 8        Comment 21       Lines       16

  Function: QP::QMActive::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        3

  Function: QK_sched_
  Parameters: (uint_fast8_t p)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Conditional if / else if: 3     
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 48       eLOC 43       lLOC 22       Comment 43       Lines       71

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 136      eLOC 117      lLOC 58       Comment 205      Lines      330
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          7
  Total Function LOC.....:         98  Total Function Pts LOC :        2.6
  Total Function eLOC....:         84  Total Function Pts eLOC:        2.2
  Total Function lLOC....:         50  Total Function Pts lLOC:        1.1
  Total Function Params .:          7  Total Function Return .:          7
  Total Cyclo Complexity :         15  Total Function Complex.:         29
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         48  Average Function LOC ..:      14.00
  Max Function eLOC .....:         43  Average Function eLOC .:      12.00
  Max Function lLOC .....:         22  Average Function lLOC .:       7.14
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          6  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          7  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:          5  Avg Cyclomatic Complex.:       2.14
  Max Total Complexity ..:         10  Avg Total Complexity ..:       4.14
  ________________________________________________________________________
  End of File: ..\source\qk.cpp


  File: ..\source\qk_mutex.cpp
  ________________________________________________________________________

  Function: QP::QMutex::init
  Parameters: (uint_fast8_t const prioCeiling)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 11       eLOC 10       lLOC 6        Comment 18       Lines       20

  Function: QP::QMutex::lock
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
      Logical and ( && )      : 2     
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 35       eLOC 29       lLOC 15       Comment 26       Lines       53

  Function: QP::QMutex::unlock
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
      Logical and ( && )      : 2     
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 35       eLOC 29       lLOC 16       Comment 26       Lines       52

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 96       eLOC 82       lLOC 37       Comment 120      Lines      226
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          3
  Total Function LOC.....:         81  Total Function Pts LOC :        1.8
  Total Function eLOC....:         68  Total Function Pts eLOC:        1.5
  Total Function lLOC....:         37  Total Function Pts lLOC:        0.7
  Total Function Params .:          1  Total Function Return .:          3
  Total Cyclo Complexity :         17  Total Function Complex.:         21
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         35  Average Function LOC ..:      27.00
  Max Function eLOC .....:         29  Average Function eLOC .:      22.67
  Max Function lLOC .....:         16  Average Function lLOC .:      12.33
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       0.33
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       1.33
  Max Cyclomatic Complex.:          7  Avg Cyclomatic Complex.:       5.67
  Max Total Complexity ..:          8  Avg Total Complexity ..:       7.00
  ________________________________________________________________________
  End of File: ..\source\qk_mutex.cpp


  File: ..\source\qs.cpp
  ________________________________________________________________________

  Function: QP::QS::initBuf
  Parameters: (uint8_t sto[], uint_fast16_t const stoSize)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 21       eLOC 20       lLOC 19       Comment 28       Lines       35

  Function: QP::QS::filterOn
  Parameters: (uint_fast8_t const rec)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 18       eLOC 13       lLOC 6        Comment 15       Lines       20

  Function: QP::QS::filterOff
  Parameters: (uint_fast8_t const rec)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 1     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 23       eLOC 18       lLOC 10       Comment 13       Lines       28

  Function: QP::QS::beginRec
  Parameters: (uint_fast8_t const rec)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 14       eLOC 13       lLOC 10       Comment 16       Lines       18

  Function: QP::QS::endRec
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
      Logical and ( && )      : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 22       eLOC 18       lLOC 10       Comment 16       Lines       27

  Function: QP::QS_target_info_
  Parameters: (uint8_t const isReset)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Selection case          : 12    
      Conditional if / else if: 2     
  Complexity   Param 1       Return 1      Cyclo Vg 15      Total       17
  LOC 148      eLOC 139      lLOC 48       Comment 10       Lines      162

  Function: QP::QS::u8
  Parameters: (uint8_t const format, uint8_t const d)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 11       eLOC 10       lLOC 7        Comment 12       Lines       14

  Function: QP::QS::u16
  Parameters: (uint8_t format, uint16_t d)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 15       eLOC 14       lLOC 10       Comment 12       Lines       20

  Function: QP::QS::u32
  Parameters: (uint8_t format, uint32_t d)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 15       eLOC 13       lLOC 10       Comment 12       Lines       18

  Function: QP::QS::u8_
  Parameters: (uint8_t const d)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 10       eLOC 9        lLOC 7        Comment 11       Lines       12

  Function: QP::QS::u8u8_
  Parameters: (uint8_t const d1, uint8_t const d2)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 11       eLOC 10       lLOC 7        Comment 11       Lines       13

  Function: QP::QS::u16_
  Parameters: (uint16_t d)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 14       eLOC 13       lLOC 10       Comment 11       Lines       18

  Function: QP::QS::u32_
  Parameters: (uint32_t d)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 14       eLOC 12       lLOC 10       Comment 11       Lines       16

  Function: QP::QS::str_
  Parameters: (char_t const *s)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 20       eLOC 18       lLOC 14       Comment 15       Lines       22

  Function: QP::QS::getByte
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 18       eLOC 14       lLOC 10       Comment 19       Lines       21

  Function: QP::QS::getBlock
  Parameters: (uint16_t * const pNbytes)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 29       eLOC 23       lLOC 17       Comment 31       Lines       32

  Function: QP::QS::sig_dict
  Parameters: (enum_t const sig, void const * const obj, char_t const *nam
              e)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 14       eLOC 11       lLOC 9        Comment 3        Lines       15

  Function: QP::QS::obj_dict
  Parameters: (void const * const obj, char_t const *name)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 13       eLOC 10       lLOC 8        Comment 3        Lines       14

  Function: QP::QS::fun_dict
  Parameters: (void (* const fun)(void), char_t const *name)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 13       eLOC 11       lLOC 8        Comment 3        Lines       14

  Function: QP::QS::usr_dict
  Parameters: (enum_t const rec, char_t const * const name)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 10       eLOC 8        lLOC 7        Comment 3        Lines       10

  Function: QP::QS::mem
  Parameters: (uint8_t const *blk, uint8_t size)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 19       eLOC 17       lLOC 11       Comment 11       Lines       24

  Function: QP::QS::str
  Parameters: (char_t const *s)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 22       eLOC 20       lLOC 14       Comment 15       Lines       26

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 510      eLOC 449      lLOC 265      Comment 325      Lines      807
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:         22
  Total Function LOC.....:        494  Total Function Pts LOC :        9.6
  Total Function eLOC....:        434  Total Function Pts eLOC:        8.5
  Total Function lLOC....:        262  Total Function Pts lLOC:        5.0
  Total Function Params .:         31  Total Function Return .:         22
  Total Cyclo Complexity :         57  Total Function Complex.:        110
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:        148  Average Function LOC ..:      22.45
  Max Function eLOC .....:        139  Average Function eLOC .:      19.73
  Max Function lLOC .....:         48  Average Function lLOC .:      11.91
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          3  Avg Function Parameters:       1.41
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          4  Avg Interface Complex. :       2.41
  Max Cyclomatic Complex.:         15  Avg Cyclomatic Complex.:       2.59
  Max Total Complexity ..:         17  Avg Total Complexity ..:       5.00
  ________________________________________________________________________
  End of File: ..\source\qs.cpp


  File: ..\source\qs_64bit.cpp
  ________________________________________________________________________

  Function: QP::QS::u64_
  Parameters: (uint64_t d)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 17       eLOC 14       lLOC 10       Comment 7        Lines       19

  Function: QP::QS::u64
  Parameters: (uint8_t format, uint64_t d)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 18       eLOC 15       lLOC 10       Comment 8        Lines       21

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 42       eLOC 35       lLOC 20       Comment 57       Lines      100
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:         35  Total Function Pts LOC :        0.8
  Total Function eLOC....:         29  Total Function Pts eLOC:        0.7
  Total Function lLOC....:         20  Total Function Pts lLOC:        0.4
  Total Function Params .:          3  Total Function Return .:          2
  Total Cyclo Complexity :          4  Total Function Complex.:          9
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         18  Average Function LOC ..:      17.50
  Max Function eLOC .....:         15  Average Function eLOC .:      14.50
  Max Function lLOC .....:         10  Average Function lLOC .:      10.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.50
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.50
  Max Cyclomatic Complex.:          2  Avg Cyclomatic Complex.:       2.00
  Max Total Complexity ..:          5  Avg Total Complexity ..:       4.50
  ________________________________________________________________________
  End of File: ..\source\qs_64bit.cpp


  File: ..\source\qs_fp.cpp
  ________________________________________________________________________

  Function: QP::QS::f32
  Parameters: (uint8_t format, float32_t const d)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 20       eLOC 18       lLOC 14       Comment 14       Lines       24

  Function: QP::QS::f64
  Parameters: (uint8_t format, float64_t const d)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 2     
  Complexity   Param 2       Return 1      Cyclo Vg 3       Total        6
  LOC 29       eLOC 26       lLOC 20       Comment 10       Lines       33

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 54       eLOC 48       lLOC 34       Comment 66       Lines      114
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          2
  Total Function LOC.....:         49  Total Function Pts LOC :        1.0
  Total Function eLOC....:         44  Total Function Pts eLOC:        0.9
  Total Function lLOC....:         34  Total Function Pts lLOC:        0.6
  Total Function Params .:          4  Total Function Return .:          2
  Total Cyclo Complexity :          5  Total Function Complex.:         11
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         29  Average Function LOC ..:      24.50
  Max Function eLOC .....:         26  Average Function eLOC .:      22.00
  Max Function lLOC .....:         20  Average Function lLOC .:      17.00
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       2.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       3.00
  Max Cyclomatic Complex.:          3  Avg Cyclomatic Complex.:       2.50
  Max Total Complexity ..:          6  Avg Total Complexity ..:       5.50
  ________________________________________________________________________
  End of File: ..\source\qs_fp.cpp


  File: ..\source\qs_rx.cpp
  ________________________________________________________________________

  Inline Function: QP::tran_
  Parameters: (RxStateEnum const target)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QS::rxInitBuf
  Parameters: (uint8_t sto[], uint16_t const stoSize)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 18       eLOC 17       lLOC 15       Comment 18       Lines       21

  Function: QP::QS::rxGetNfree
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 14       eLOC 10       lLOC 5        Comment 8        Lines       14

  Function: QP::QS::rxParse
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops while / do        : 1     
      Conditional if / else if: 5     
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 36       eLOC 26       lLOC 16       Comment 5        Lines       41

  Function: QP::rxParseData_
  Parameters: (uint8_t const b)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Selection case          : 40    
      Conditional if / else if: 16    
  Complexity   Param 1       Return 1      Cyclo Vg 57      Total       59
  LOC 260      eLOC 202      lLOC 125      Comment 18       Lines      273

  Function: QP::rxHandleGoodFrame_
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 3     
      Selection case          : 17    
      Conditional if / else if: 4     
  Complexity   Param 0       Return 1      Cyclo Vg 25      Total       26
  LOC 148      eLOC 122      lLOC 72       Comment 15       Lines      159

  Function: QP::rxHandleBadFrame_
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Selection case          : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 12       eLOC 8        lLOC 4        Comment 1        Lines       12

  Function: QP::rxReportSuccess_
  Parameters: (enum QSpyRxRecords const recId)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 6        lLOC 2        Comment 3        Lines        7

  Function: QP::rxReportError_
  Parameters: (uint8_t const stateId)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 6        lLOC 2        Comment 3        Lines        7

  Function: QP::rxAddr_
  Parameters: (uint8_t const b, QSAddr *const addr, uint8_t *const idx)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Inlined if-else ( ? : ) : 1     
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 5        eLOC 4        lLOC 3        Comment 1        Lines        5

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 621      eLOC 504      lLOC 303      Comment 122      Lines      758
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:         10
  Total Function LOC.....:        510  Total Function Pts LOC :       11.7
  Total Function eLOC....:        403  Total Function Pts eLOC:        9.5
  Total Function lLOC....:        245  Total Function Pts lLOC:        5.7
  Total Function Params .:          9  Total Function Return .:         10
  Total Cyclo Complexity :        100  Total Function Complex.:        119
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:        260  Average Function LOC ..:      51.00
  Max Function eLOC .....:        202  Average Function eLOC .:      40.30
  Max Function lLOC .....:        125  Average Function lLOC .:      24.50
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          3  Avg Function Parameters:       0.90
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          4  Avg Interface Complex. :       1.90
  Max Cyclomatic Complex.:         57  Avg Cyclomatic Complex.:      10.00
  Max Total Complexity ..:         59  Avg Total Complexity ..:      11.90
  ________________________________________________________________________
  End of File: ..\source\qs_rx.cpp


  File: ..\source\qv.cpp
  ________________________________________________________________________

  Function: QP::QF::stop
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        4

  Function: QP::QF::run
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
      Conditional if / else if: 2     
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 41       eLOC 36       lLOC 18       Comment 41       Lines       63

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const, QEvt c
              onst * const ie)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 2     
  Complexity   Param 6       Return 1      Cyclo Vg 3       Total       10
  LOC 10       eLOC 8        lLOC 6        Comment 28       Lines       15

  Function: QP::QMActive::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 12       Lines        3

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 90       eLOC 78       lLOC 33       Comment 159      Lines      245
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          4
  Total Function LOC.....:         57  Total Function Pts LOC :        1.7
  Total Function eLOC....:         48  Total Function Pts eLOC:        1.5
  Total Function lLOC....:         26  Total Function Pts lLOC:        0.6
  Total Function Params .:          7  Total Function Return .:          4
  Total Cyclo Complexity :          9  Total Function Complex.:         20
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         41  Average Function LOC ..:      14.25
  Max Function eLOC .....:         36  Average Function eLOC .:      12.00
  Max Function lLOC .....:         18  Average Function lLOC .:       6.50
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          6  Avg Function Parameters:       1.75
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          7  Avg Interface Complex. :       2.75
  Max Cyclomatic Complex.:          4  Avg Cyclomatic Complex.:       2.25
  Max Total Complexity ..:         10  Avg Total Complexity ..:       5.00
  ________________________________________________________________________
  End of File: ..\source\qv.cpp


  File: ..\source\qxk.cpp
  ________________________________________________________________________

  Function: QP::thread_ao
  Parameters: (void *par)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 7        eLOC 5        lLOC 4        Comment 5        Lines        8

  Function: QP::thread_idle
  Parameters: (void *par)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Loops for / foreach     : 1     
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 9        eLOC 7        lLOC 6        Comment 3        Lines       11

  Function: QP::QF::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 6        eLOC 5        lLOC 4        Comment 14       Lines       10

  Function: QP::QF::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        4

  Function: QP::QF::run
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 10       eLOC 9        lLOC 8        Comment 19       Lines       17

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              , QEvt const * const ie)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
      Logical and ( && )      : 5     
  Complexity   Param 6       Return 1      Cyclo Vg 7       Total       14
  LOC 22       eLOC 19       lLOC 12       Comment 23       Lines       28

  Function: QP::QMActive::stop
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 10       eLOC 9        lLOC 6        Comment 15       Lines       14

  Function: QP::QXK::init
  Parameters: (void *idleStkSto, uint_fast16_t idleStkSize)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 9        eLOC 8        lLOC 5        Comment 12       Lines       13

  Function: QXK_threadRet_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 10       eLOC 9        lLOC 6        Comment 11       Lines       12

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 128      eLOC 110      lLOC 62       Comment 184      Lines      318
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          9
  Total Function LOC.....:         86  Total Function Pts LOC :        2.4
  Total Function eLOC....:         73  Total Function Pts eLOC:        2.1
  Total Function lLOC....:         52  Total Function Pts lLOC:        1.2
  Total Function Params .:         10  Total Function Return .:          9
  Total Cyclo Complexity :         18  Total Function Complex.:         37
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         22  Average Function LOC ..:       9.56
  Max Function eLOC .....:         19  Average Function eLOC .:       8.11
  Max Function lLOC .....:         12  Average Function lLOC .:       5.78
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          6  Avg Function Parameters:       1.11
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          7  Avg Interface Complex. :       2.11
  Max Cyclomatic Complex.:          7  Avg Cyclomatic Complex.:       2.00
  Max Total Complexity ..:         14  Avg Total Complexity ..:       4.11
  ________________________________________________________________________
  End of File: ..\source\qxk.cpp


  File: ..\source\qxk_mutex.cpp
  ________________________________________________________________________

  Function: QP::QXMutex::init
  Parameters: (uint_fast8_t const prioCeiling)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 2     
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 13       eLOC 12       lLOC 7        Comment 17       Lines       21

  Function: QP::QXMutex::lock
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
      Logical and ( && )      : 2     
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 38       eLOC 32       lLOC 17       Comment 25       Lines       57

  Function: QP::QXMutex::unlock
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
      Logical and ( && )      : 2     
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 47       eLOC 41       lLOC 23       Comment 30       Lines       72

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 113      eLOC 99       lLOC 47       Comment 122      Lines      248
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          3
  Total Function LOC.....:         98  Total Function Pts LOC :        2.1
  Total Function eLOC....:         85  Total Function Pts eLOC:        1.9
  Total Function lLOC....:         47  Total Function Pts lLOC:        0.9
  Total Function Params .:          1  Total Function Return .:          3
  Total Cyclo Complexity :         17  Total Function Complex.:         21
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         47  Average Function LOC ..:      32.67
  Max Function eLOC .....:         41  Average Function eLOC .:      28.33
  Max Function lLOC .....:         23  Average Function lLOC .:      15.67
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          1  Avg Function Parameters:       0.33
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          2  Avg Interface Complex. :       1.33
  Max Cyclomatic Complex.:          7  Avg Cyclomatic Complex.:       5.67
  Max Total Complexity ..:          8  Avg Total Complexity ..:       7.00
  ________________________________________________________________________
  End of File: ..\source\qxk_mutex.cpp


  File: ..\source\qxk_sema.cpp
  ________________________________________________________________________

  Function: QP::QXSemaphore::init
  Parameters: (uint_fast16_t const count)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QXSemaphore::wait
  Parameters: (uint_fast16_t const nTicks, uint_fast8_t const tickRate)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
      Logical and ( && )      : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 3       Total        6
  LOC 19       eLOC 15       lLOC 11       Comment 4        Lines       26

  Function: QP::QXSemaphore::signal
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
      Logical and ( && )      : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 20       eLOC 15       lLOC 9        Comment 3        Lines       26

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 60       eLOC 49       lLOC 22       Comment 56       Lines      123
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:          3
  Total Function LOC.....:         43  Total Function Pts LOC :        1.1
  Total Function eLOC....:         33  Total Function Pts eLOC:        0.9
  Total Function lLOC....:         22  Total Function Pts lLOC:        0.4
  Total Function Params .:          3  Total Function Return .:          3
  Total Cyclo Complexity :          8  Total Function Complex.:         14
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         20  Average Function LOC ..:      14.33
  Max Function eLOC .....:         15  Average Function eLOC .:      11.00
  Max Function lLOC .....:         11  Average Function lLOC .:       7.33
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          2  Avg Function Parameters:       1.00
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          3  Avg Interface Complex. :       2.00
  Max Cyclomatic Complex.:          4  Avg Cyclomatic Complex.:       2.67
  Max Total Complexity ..:          6  Avg Total Complexity ..:       4.67
  ________________________________________________________________________
  End of File: ..\source\qxk_sema.cpp


  File: ..\source\qxk_xthr.cpp
  ________________________________________________________________________

  Function: QP::QXThread::QXThread
  Parameters: (QXThreadHandler const handler, uint_fast8_t const tickRate)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 3        eLOC 1        lLOC 1        Comment 3        Lines        3

  Function: QP::QXThread::init
  Parameters: (QEvt const * const)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 3        Lines        3

  Function: QP::QXThread::dispatch
  Parameters: (QEvt const * const)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QXThread::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              , QEvt const * const)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
      Logical and ( && )      : 4     
  Complexity   Param 6       Return 1      Cyclo Vg 7       Total       14
  LOC 22       eLOC 18       lLOC 10       Comment 8        Lines       32

  Function: QP::QXThread::post_
  Parameters: (QEvt const * const e, uint_fast16_t const margin)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 2       Return 1      Cyclo Vg 3       Total        6
  LOC 23       eLOC 18       lLOC 14       Comment 8        Lines       32

  Function: QP::QXThread::postLIFO
  Parameters: (QEvt const * const)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QXThread::block_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Function: QP::QXThread::unblock_
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
      Logical and ( && )      : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 8        eLOC 5        lLOC 2        Comment 3        Lines       10

  Function: QP::QXThread::teArm_
  Parameters: (enum_t const sig, uint_fast16_t const nTicks, uint_fast8_t 
              const tickRate)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 2     
  Complexity   Param 3       Return 1      Cyclo Vg 3       Total        7
  LOC 15       eLOC 10       lLOC 6        Comment 14       Lines       29

  Function: QP::QXThread::teDisarm_
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 11       eLOC 8        lLOC 5        Comment 5        Lines       14

  Function: QP::QXThread::block
  Parameters: (void)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Logical and ( && )      : 1     
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 9        eLOC 8        lLOC 5        Comment 4        Lines       11

  Function: QP::QXThread::unblock
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 7        eLOC 6        lLOC 4        Comment 3        Lines       10

  Function: QP::QXThread::delay
  Parameters: (uint_fast16_t const nTicks, uint_fast8_t const tickRate)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 10       eLOC 8        lLOC 7        Comment 5        Lines       15

  Function: QP::QXThread::delayCancel
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 8        eLOC 7        lLOC 5        Comment 2        Lines       10

  Function: QP::QXThread::queueGet
  Parameters: (uint_fast16_t const nTicks, uint_fast8_t const tickRate)
    Cyclomatic Complexity Vg Detail
      Function Base           : 1
      Conditional if / else if: 4     
      Logical and ( && )      : 1     
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 56       eLOC 48       lLOC 32       Comment 24       Lines       74

  ------------------------------------------------------------------------

                          ~~ Total File Summary ~~

  LOC 219      eLOC 179      lLOC 96       Comment 136      Lines      371
  ------------------------------------------------------------------------

                        ~~ File Functional Summary ~~

  File Function Count....:         15
  Total Function LOC.....:        185  Total Function Pts LOC :        4.1
  Total Function eLOC....:        146  Total Function Pts eLOC:        3.4
  Total Function lLOC....:         96  Total Function Pts lLOC:        1.8
  Total Function Params .:         20  Total Function Return .:         15
  Total Cyclo Complexity :         34  Total Function Complex.:         69
         ------     -----     -----     ------     ------     -----       
  Max Function LOC ......:         56  Average Function LOC ..:      12.33
  Max Function eLOC .....:         48  Average Function eLOC .:       9.73
  Max Function lLOC .....:         32  Average Function lLOC .:       6.40
         ------     -----     -----     ------     ------     -----       
  Max Function Parameters:          6  Avg Function Parameters:       1.33
  Max Function Returns ..:          1  Avg Function Returns ..:       1.00
  Max Interface Complex. :          7  Avg Interface Complex. :       2.33
  Max Cyclomatic Complex.:          7  Avg Cyclomatic Complex.:       2.27
  Max Total Complexity ..:         14  Avg Total Complexity ..:       4.60
  ________________________________________________________________________
  End of File: ..\source\qxk_xthr.cpp


  ------------------------------------------------------------------------

                     ~~ Project Analysis For 41 Files ~~

  ------------------------------------------------------------------------

                         ~~ Total Project Summary ~~

  LOC 5588     eLOC 4910     lLOC 2272     Comment 6403     Lines    12364
  Average per File, metric/41 files
  LOC 136      eLOC 119      lLOC 55       Comment 156      Lines      301

  ------------------------------------------------------------------------

                      ~~ Project Functional Metrics ~~

  Function: QP::QEvt::QEvt
  Parameters: (QSignal const s)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 0        Comment 2        Lines        1

  Function: QP::QEvt::~QEvt
  Parameters: ()
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 0        Comment 1        Lines        1

  Function: QP::QMsm::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Function: QP::QMsm::stateObj
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QMsm::qm_tran_
  Parameters: (QMTranActTable const * const tatbl) Non-ANSI Parameter Cons
              truct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Function: QP::QMsm::qm_tran_hist_
  Parameters: (QMState const * const hist, QMTranActTable const * const ta
              tbl)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 5        eLOC 3        lLOC 3        Comment 1        Lines        5

  Function: QP::QMsm::qm_tran_init_
  Parameters: (QMTranActTable const * const tatbl)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QMsm::qm_tran_ep_
  Parameters: (QMTranActTable const * const tatbl)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Function: QP::QMsm::qm_tran_xp_
  Parameters: (QActionHandler const xp, QMTranActTable const *const tatbl)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 5        eLOC 3        lLOC 3        Comment 2        Lines        5

  Function: QP::QMsm::qm_entry_
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QMsm::qm_exit_
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QMsm::qm_super_sub_
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Function: QP::QMsm::QM_HANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QMsm::QM_UNHANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QMsm::QM_SUPER
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QMsm::Q_HANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 3        Lines        3

  Function: QP::QMsm::Q_UNHANDLED
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 4        Lines        3

  Function: QP::QMsm::tran_
  Parameters: (QStateHandler const target)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QMsm::tran_hist_
  Parameters: (QStateHandler const hist)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QMsm::super_
  Parameters: (QStateHandler const superstate)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QHsm::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Function: QP::QHsm::state
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QEP::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QEQueue::getNFree
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 10       Lines        3

  Function: QP::QEQueue::isEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 10       Lines        3

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              )
  Complexity   Param 5       Return 1      Cyclo Vg 1       Total        7
  LOC 4        eLOC 2        lLOC 1        Comment 1        Lines        4

  Function: QP::QMActive::getPrio
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QMActive::setPrio
  Parameters: (uint_fast8_t const prio)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QMActive::getOsObject
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Function: QP::QMActive::getThread
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Function: QP::QTimeEvt::QTimeEvt
  Parameters: (enum_t const sgnl) Non-ANSI Parameter Construct Defined By 
              Compiler Directive
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 5        lLOC 3        Comment 5        Lines        8

  Function: QP::QTimeEvt::postIn
  Parameters: (QMActive * const act, QTimeEvtCtr const nTicks)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QTimeEvt::postEvery
  Parameters: (QMActive * const act, QTimeEvtCtr const nTicks)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QTimeEvt::toActive
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Function: QP::QTimeEvt::toTimeEvt
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Function: QP::QF::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QK::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QMPool::getBlockSize
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QPSet8::isEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QPSet8::notEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QPSet8::hasElement
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 5        eLOC 4        lLOC 1        Comment 1        Lines        5

  Function: QP::QPSet8::insert
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QPSet8::remove
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QPSet8::findMax
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QPSet64::isEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QPSet64::notEmpty
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QPSet64::hasElement
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 6        lLOC 2        Comment 1        Lines        7

  Function: QP::QPSet64::insert
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 8        eLOC 7        lLOC 3        Comment 1        Lines        8

  Function: QP::QPSet64::remove
  Parameters: (uint_fast8_t const n)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 9        eLOC 7        lLOC 3        Comment 1        Lines        9

  Function: QP::QPSet64::findMax
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 15       eLOC 12       lLOC 5        Comment 1        Lines       15

  Function: QP::QS::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QS::rxPut
  Parameters: (uint8_t const b)
  Complexity   Param 1       Return 1      Cyclo Vg 4       Total        6
  LOC 14       eLOC 9        lLOC 4        Comment 1        Lines       14

  Function: QP::QV::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QXK::getVersion
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QXThread::init
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 1        Comment 2        Lines        1

  Function: QP::QXThread::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              )
  Complexity   Param 5       Return 1      Cyclo Vg 1       Total        7
  LOC 4        eLOC 2        lLOC 1        Comment 1        Lines        4

  Function: QP::QF_EVT_POOL_ID_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Function: QP::QF_EVT_REF_CTR_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 1        eLOC 1        lLOC 1        Comment 1        Lines        1

  Function: QP::QF_EVT_REF_CTR_INC_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QF_EVT_REF_CTR_DEC_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QHsm::QHsm
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 1        lLOC 1        Comment 8        Lines        3

  Function: QP::QHsm::init
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 38       eLOC 36       lLOC 26       Comment 25       Lines       57

  Function: QP::QHsm::top
  Parameters: (void * const, QEvt const * const)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 3        eLOC 2        lLOC 1        Comment 15       Lines        3

  Function: QP::QHsm::dispatch
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 13      Total       15
  LOC 111      eLOC 98       lLOC 63       Comment 67       Lines      152

  Function: QP::QHsm::hsm_tran
  Parameters: (QStateHandler (&path)[MAX_NEST_DEPTH_])
  Complexity   Param 1       Return 1      Cyclo Vg 15      Total       17
  LOC 98       eLOC 78       lLOC 48       Comment 61       Lines      139

  Function: QP::QHsm::isIn
  Parameters: (QStateHandler const s)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 16       eLOC 13       lLOC 9        Comment 21       Lines       23

  Function: QP::QHsm::childState
  Parameters: (QStateHandler const parent)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 19       eLOC 16       lLOC 12       Comment 27       Lines       25

  Function: QP::QMsm::QMsm
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 13       Lines        4

  Function: QP::QMsm::~QMsm
  Parameters: ()
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 2        eLOC 1        lLOC 0        Comment 4        Lines        2

  Function: QP::QMsm::init
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 21       eLOC 20       lLOC 12       Comment 23       Lines       33

  Function: QP::QMsm::isInState
  Parameters: (QMState const * const state)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 13       eLOC 9        lLOC 5        Comment 15       Lines       14

  Function: QP::QMsm::dispatch
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 15      Total       17
  LOC 107      eLOC 91       lLOC 56       Comment 66       Lines      147

  Function: QP::QMsm::execTatbl_
  Parameters: (QMTranActTable const * const tatbl)
  Complexity   Param 1       Return 1      Cyclo Vg 8       Total       10
  LOC 53       eLOC 43       lLOC 21       Comment 31       Lines       64

  Function: QP::QMsm::exitToTranSource_
  Parameters: (QMState const *s, QMState const * const ts)
  Complexity   Param 2       Return 1      Cyclo Vg 5       Total        8
  LOC 24       eLOC 16       lLOC 7        Comment 19       Lines       32

  Function: QP::QMsm::enterHistory_
  Parameters: (QMState const * const hist)
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 37       eLOC 31       lLOC 21       Comment 21       Lines       45

  Function: QP::QMsm::childStateObj
  Parameters: (QMState const * const parent)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 19       eLOC 14       lLOC 9        Comment 17       Lines       23

  Function: QP::QF::add_
  Parameters: (QMActive * const a)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 15       eLOC 14       lLOC 8        Comment 17       Lines       20

  Function: QP::QF::remove_
  Parameters: (QMActive const * const a)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 15       eLOC 14       lLOC 8        Comment 17       Lines       20

  Function: QP::QF::bzero
  Parameters: (void * const start, uint_fast16_t len)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 8        eLOC 6        lLOC 4        Comment 13       Lines        8

  Function: QP::QMActive::post_
  Parameters: (QEvt const * const e, uint_fast16_t const margin)
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 55       eLOC 46       lLOC 33       Comment 72       Lines       75

  Function: QP::QMActive::postLIFO
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 35       eLOC 29       lLOC 20       Comment 32       Lines       47

  Function: QP::QMActive::get_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 36       eLOC 32       lLOC 22       Comment 27       Lines       48

  Function: QP::QF::getQueueMin
  Parameters: (uint_fast8_t const prio)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 10       eLOC 9        lLOC 5        Comment 17       Lines       13

  Function: QP::QMActive::defer
  Parameters: (QEQueue * const eq, QEvt const * const e)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 3        eLOC 2        lLOC 1        Comment 21       Lines        3

  Function: QP::QMActive::recall
  Parameters: (QEQueue * const eq)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 15       eLOC 12       lLOC 8        Comment 29       Lines       27

  Function: QP::QF::poolInit
  Parameters: (void * const poolSto, uint_fast32_t const poolSize, uint_fa
              st16_t const evtSize)
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 10       eLOC 8        lLOC 4        Comment 36       Lines       13

  Function: QP::QF::newX_
  Parameters: (uint_fast16_t const evtSize, uint_fast16_t const margin, en
              um_t const sig)
  Complexity   Param 3       Return 1      Cyclo Vg 4       Total        8
  LOC 27       eLOC 21       lLOC 14       Comment 33       Lines       38

  Function: QP::QF::gc
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 32       eLOC 28       lLOC 14       Comment 39       Lines       47

  Function: QP::QF::newRef_
  Parameters: (QEvt const * const e, QEvt const * const evtRef)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 7        eLOC 5        lLOC 3        Comment 15       Lines       11

  Function: QP::QF::poolGetMaxBlockSize
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 4        eLOC 3        lLOC 1        Comment 4        Lines        4

  Function: QP::QMPool::QMPool
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 0        Comment 8        Lines        1

  Function: QP::QMPool::init
  Parameters: (void * const poolSto, uint_fast32_t poolSize, uint_fast16_t
               blockSize)
  Complexity   Param 3       Return 1      Cyclo Vg 5       Total        9
  LOC 35       eLOC 31       lLOC 22       Comment 51       Lines       58

  Function: QP::QMPool::put
  Parameters: (void * const b)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 16       eLOC 15       lLOC 9        Comment 23       Lines       23

  Function: QP::QMPool::get
  Parameters: (uint_fast16_t const margin)
  Complexity   Param 1       Return 1      Cyclo Vg 4       Total        6
  LOC 40       eLOC 34       lLOC 22       Comment 46       Lines       60

  Function: QP::QF::getPoolMin
  Parameters: (uint_fast8_t const poolId)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 10       eLOC 9        lLOC 5        Comment 13       Lines       14

  Function: QP::QF::psInit
  Parameters: (QSubscrList * const subscrSto, enum_t const maxSignal)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 7        eLOC 6        lLOC 3        Comment 30       Lines       11

  Function: QP::QF::publish_
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 17      Total       19
  LOC 106      eLOC 95       lLOC 52       Comment 105      Lines      207

  Function: QP::QActive::QActive
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 1        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::init
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::dispatch
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QActive::isIn
  Parameters: (QStateHandler const s)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QEQueue::QEQueue
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 1        eLOC 1        lLOC 0        Comment 4        Lines        1

  Function: QP::QEQueue::init
  Parameters: (QEvt const *qSto[], uint_fast16_t const qLen)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 15       eLOC 14       lLOC 9        Comment 19       Lines       16

  Function: QP::QEQueue::post
  Parameters: (QEvt const * const e, uint_fast16_t const margin)
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 51       eLOC 43       lLOC 29       Comment 52       Lines       72

  Function: QP::QEQueue::postLIFO
  Parameters: (QEvt const * const e)
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 32       eLOC 27       lLOC 19       Comment 36       Lines       43

  Function: QP::QEQueue::get
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 37       eLOC 32       lLOC 21       Comment 31       Lines       46

  Function: QP::QMActive::QMActive
  Parameters: (QStateHandler const initial)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 9        eLOC 7        lLOC 3        Comment 1        Lines       11

  Function: QP::QF::tickX_
  Parameters: (uint8_t const tickRate)
  Complexity   Param 1       Return 1      Cyclo Vg 7       Total        9
  LOC 65       eLOC 53       lLOC 36       Comment 51       Lines       90

  Function: QP::QF::noTimeEvtsActiveX
  Parameters: (uint8_t const tickRate)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 14       eLOC 10       lLOC 6        Comment 11       Lines       14

  Function: QP::QTimeEvt::QTimeEvt
  Parameters: (QMActive * const act, enum_t const sgnl, uint8_t const tick
              Rate) Defined By Compiler Directive
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 9        eLOC 7        lLOC 4        Comment 22       Lines       22

  Function: QP::QTimeEvt::QTimeEvt
  Parameters: () Defined By Compiler Directive
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 5        lLOC 3        Comment 12       Lines       18

  Function: QP::QTimeEvt::armX
  Parameters: (QTimeEvtCtr const nTicks, QTimeEvtCtr const interval)
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 28       eLOC 26       lLOC 16       Comment 48       Lines       50

  Function: QP::QTimeEvt::disarm
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 29       eLOC 26       lLOC 17       Comment 27       Lines       35

  Function: QP::QTimeEvt::rearm
  Parameters: (QTimeEvtCtr const nTicks)
  Complexity   Param 1       Return 1      Cyclo Vg 7       Total        9
  LOC 37       eLOC 32       lLOC 18       Comment 47       Lines       64

  Function: QP::QTimeEvt::ctr
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 15       eLOC 14       lLOC 10       Comment 16       Lines       18

  Function: QP::QF::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 13       eLOC 12       lLOC 8        Comment 18       Lines       21

  Function: QP::QF::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        4

  Function: QP::initial_events
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 7        eLOC 5        lLOC 3        Comment 6        Lines       10

  Function: QP::QF::run
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 12       eLOC 10       lLOC 7        Comment 21       Lines       15

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              , QEvt const * const ie)
  Complexity   Param 6       Return 1      Cyclo Vg 3       Total       10
  LOC 12       eLOC 10       lLOC 8        Comment 21       Lines       16

  Function: QP::QMActive::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        3

  Function: QK_sched_
  Parameters: (uint_fast8_t p)
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 48       eLOC 43       lLOC 22       Comment 43       Lines       71

  Function: QP::QMutex::init
  Parameters: (uint_fast8_t const prioCeiling)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 11       eLOC 10       lLOC 6        Comment 18       Lines       20

  Function: QP::QMutex::lock
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 35       eLOC 29       lLOC 15       Comment 26       Lines       53

  Function: QP::QMutex::unlock
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 35       eLOC 29       lLOC 16       Comment 26       Lines       52

  Function: QP::QS::initBuf
  Parameters: (uint8_t sto[], uint_fast16_t const stoSize)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 21       eLOC 20       lLOC 19       Comment 28       Lines       35

  Function: QP::QS::filterOn
  Parameters: (uint_fast8_t const rec)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 18       eLOC 13       lLOC 6        Comment 15       Lines       20

  Function: QP::QS::filterOff
  Parameters: (uint_fast8_t const rec)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 23       eLOC 18       lLOC 10       Comment 13       Lines       28

  Function: QP::QS::beginRec
  Parameters: (uint_fast8_t const rec)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 14       eLOC 13       lLOC 10       Comment 16       Lines       18

  Function: QP::QS::endRec
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 22       eLOC 18       lLOC 10       Comment 16       Lines       27

  Function: QP::QS_target_info_
  Parameters: (uint8_t const isReset)
  Complexity   Param 1       Return 1      Cyclo Vg 15      Total       17
  LOC 148      eLOC 139      lLOC 48       Comment 10       Lines      162

  Function: QP::QS::u8
  Parameters: (uint8_t const format, uint8_t const d)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 11       eLOC 10       lLOC 7        Comment 12       Lines       14

  Function: QP::QS::u16
  Parameters: (uint8_t format, uint16_t d)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 15       eLOC 14       lLOC 10       Comment 12       Lines       20

  Function: QP::QS::u32
  Parameters: (uint8_t format, uint32_t d)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 15       eLOC 13       lLOC 10       Comment 12       Lines       18

  Function: QP::QS::u8_
  Parameters: (uint8_t const d)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 10       eLOC 9        lLOC 7        Comment 11       Lines       12

  Function: QP::QS::u8u8_
  Parameters: (uint8_t const d1, uint8_t const d2)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 11       eLOC 10       lLOC 7        Comment 11       Lines       13

  Function: QP::QS::u16_
  Parameters: (uint16_t d)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 14       eLOC 13       lLOC 10       Comment 11       Lines       18

  Function: QP::QS::u32_
  Parameters: (uint32_t d)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 14       eLOC 12       lLOC 10       Comment 11       Lines       16

  Function: QP::QS::str_
  Parameters: (char_t const *s)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 20       eLOC 18       lLOC 14       Comment 15       Lines       22

  Function: QP::QS::getByte
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 18       eLOC 14       lLOC 10       Comment 19       Lines       21

  Function: QP::QS::getBlock
  Parameters: (uint16_t * const pNbytes)
  Complexity   Param 1       Return 1      Cyclo Vg 5       Total        7
  LOC 29       eLOC 23       lLOC 17       Comment 31       Lines       32

  Function: QP::QS::sig_dict
  Parameters: (enum_t const sig, void const * const obj, char_t const *nam
              e)
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 14       eLOC 11       lLOC 9        Comment 3        Lines       15

  Function: QP::QS::obj_dict
  Parameters: (void const * const obj, char_t const *name)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 13       eLOC 10       lLOC 8        Comment 3        Lines       14

  Function: QP::QS::fun_dict
  Parameters: (void (* const fun)(void), char_t const *name)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 13       eLOC 11       lLOC 8        Comment 3        Lines       14

  Function: QP::QS::usr_dict
  Parameters: (enum_t const rec, char_t const * const name)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 10       eLOC 8        lLOC 7        Comment 3        Lines       10

  Function: QP::QS::mem
  Parameters: (uint8_t const *blk, uint8_t size)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 19       eLOC 17       lLOC 11       Comment 11       Lines       24

  Function: QP::QS::str
  Parameters: (char_t const *s)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 22       eLOC 20       lLOC 14       Comment 15       Lines       26

  Function: QP::QS::u64_
  Parameters: (uint64_t d)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 17       eLOC 14       lLOC 10       Comment 7        Lines       19

  Function: QP::QS::u64
  Parameters: (uint8_t format, uint64_t d)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 18       eLOC 15       lLOC 10       Comment 8        Lines       21

  Function: QP::QS::f32
  Parameters: (uint8_t format, float32_t const d)
  Complexity   Param 2       Return 1      Cyclo Vg 2       Total        5
  LOC 20       eLOC 18       lLOC 14       Comment 14       Lines       24

  Function: QP::QS::f64
  Parameters: (uint8_t format, float64_t const d)
  Complexity   Param 2       Return 1      Cyclo Vg 3       Total        6
  LOC 29       eLOC 26       lLOC 20       Comment 10       Lines       33

  Function: QP::tran_
  Parameters: (RxStateEnum const target)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 1        Lines        3

  Function: QP::QS::rxInitBuf
  Parameters: (uint8_t sto[], uint16_t const stoSize)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 18       eLOC 17       lLOC 15       Comment 18       Lines       21

  Function: QP::QS::rxGetNfree
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 14       eLOC 10       lLOC 5        Comment 8        Lines       14

  Function: QP::QS::rxParse
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 36       eLOC 26       lLOC 16       Comment 5        Lines       41

  Function: QP::rxParseData_
  Parameters: (uint8_t const b)
  Complexity   Param 1       Return 1      Cyclo Vg 57      Total       59
  LOC 260      eLOC 202      lLOC 125      Comment 18       Lines      273

  Function: QP::rxHandleGoodFrame_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 25      Total       26
  LOC 148      eLOC 122      lLOC 72       Comment 15       Lines      159

  Function: QP::rxHandleBadFrame_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 12       eLOC 8        lLOC 4        Comment 1        Lines       12

  Function: QP::rxReportSuccess_
  Parameters: (enum QSpyRxRecords const recId)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 6        lLOC 2        Comment 3        Lines        7

  Function: QP::rxReportError_
  Parameters: (uint8_t const stateId)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 7        eLOC 6        lLOC 2        Comment 3        Lines        7

  Function: QP::rxAddr_
  Parameters: (uint8_t const b, QSAddr *const addr, uint8_t *const idx)
  Complexity   Param 3       Return 1      Cyclo Vg 2       Total        6
  LOC 5        eLOC 4        lLOC 3        Comment 1        Lines        5

  Function: QP::QF::stop
  Parameters: (void) Non-ANSI Parameter Construct
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        4

  Function: QP::QF::run
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 41       eLOC 36       lLOC 18       Comment 41       Lines       63

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const, QEvt c
              onst * const ie)
  Complexity   Param 6       Return 1      Cyclo Vg 3       Total       10
  LOC 10       eLOC 8        lLOC 6        Comment 28       Lines       15

  Function: QP::QMActive::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 12       Lines        3

  Function: QP::thread_ao
  Parameters: (void *par)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 7        eLOC 5        lLOC 4        Comment 5        Lines        8

  Function: QP::thread_idle
  Parameters: (void *par)
  Complexity   Param 1       Return 1      Cyclo Vg 2       Total        4
  LOC 9        eLOC 7        lLOC 6        Comment 3        Lines       11

  Function: QP::QF::init
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 6        eLOC 5        lLOC 4        Comment 14       Lines       10

  Function: QP::QF::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 3        eLOC 2        lLOC 1        Comment 13       Lines        4

  Function: QP::QF::run
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 10       eLOC 9        lLOC 8        Comment 19       Lines       17

  Function: QP::QMActive::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              , QEvt const * const ie)
  Complexity   Param 6       Return 1      Cyclo Vg 7       Total       14
  LOC 22       eLOC 19       lLOC 12       Comment 23       Lines       28

  Function: QP::QMActive::stop
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 10       eLOC 9        lLOC 6        Comment 15       Lines       14

  Function: QP::QXK::init
  Parameters: (void *idleStkSto, uint_fast16_t idleStkSize)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 9        eLOC 8        lLOC 5        Comment 12       Lines       13

  Function: QXK_threadRet_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 10       eLOC 9        lLOC 6        Comment 11       Lines       12

  Function: QP::QXMutex::init
  Parameters: (uint_fast8_t const prioCeiling)
  Complexity   Param 1       Return 1      Cyclo Vg 3       Total        5
  LOC 13       eLOC 12       lLOC 7        Comment 17       Lines       21

  Function: QP::QXMutex::lock
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 38       eLOC 32       lLOC 17       Comment 25       Lines       57

  Function: QP::QXMutex::unlock
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 7       Total        8
  LOC 47       eLOC 41       lLOC 23       Comment 30       Lines       72

  Function: QP::QXSemaphore::init
  Parameters: (uint_fast16_t const count)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 4        eLOC 3        lLOC 2        Comment 1        Lines        4

  Function: QP::QXSemaphore::wait
  Parameters: (uint_fast16_t const nTicks, uint_fast8_t const tickRate)
  Complexity   Param 2       Return 1      Cyclo Vg 3       Total        6
  LOC 19       eLOC 15       lLOC 11       Comment 4        Lines       26

  Function: QP::QXSemaphore::signal
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 4       Total        5
  LOC 20       eLOC 15       lLOC 9        Comment 3        Lines       26

  Function: QP::QXThread::QXThread
  Parameters: (QXThreadHandler const handler, uint_fast8_t const tickRate)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 3        eLOC 1        lLOC 1        Comment 3        Lines        3

  Function: QP::QXThread::init
  Parameters: (QEvt const * const)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 3        Lines        3

  Function: QP::QXThread::dispatch
  Parameters: (QEvt const * const)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QXThread::start
  Parameters: (uint_fast8_t const prio, QEvt const *qSto[], uint_fast16_t 
              const qLen, void * const stkSto, uint_fast16_t const stkSize
              , QEvt const * const)
  Complexity   Param 6       Return 1      Cyclo Vg 7       Total       14
  LOC 22       eLOC 18       lLOC 10       Comment 8        Lines       32

  Function: QP::QXThread::post_
  Parameters: (QEvt const * const e, uint_fast16_t const margin)
  Complexity   Param 2       Return 1      Cyclo Vg 3       Total        6
  LOC 23       eLOC 18       lLOC 14       Comment 8        Lines       32

  Function: QP::QXThread::postLIFO
  Parameters: (QEvt const * const)
  Complexity   Param 1       Return 1      Cyclo Vg 1       Total        3
  LOC 3        eLOC 2        lLOC 1        Comment 2        Lines        3

  Function: QP::QXThread::block_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 4        eLOC 3        lLOC 2        Comment 2        Lines        4

  Function: QP::QXThread::unblock_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 3       Total        4
  LOC 8        eLOC 5        lLOC 2        Comment 3        Lines       10

  Function: QP::QXThread::teArm_
  Parameters: (enum_t const sig, uint_fast16_t const nTicks, uint_fast8_t 
              const tickRate)
  Complexity   Param 3       Return 1      Cyclo Vg 3       Total        7
  LOC 15       eLOC 10       lLOC 6        Comment 14       Lines       29

  Function: QP::QXThread::teDisarm_
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 11       eLOC 8        lLOC 5        Comment 5        Lines       14

  Function: QP::QXThread::block
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 2       Total        3
  LOC 9        eLOC 8        lLOC 5        Comment 4        Lines       11

  Function: QP::QXThread::unblock
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 7        eLOC 6        lLOC 4        Comment 3        Lines       10

  Function: QP::QXThread::delay
  Parameters: (uint_fast16_t const nTicks, uint_fast8_t const tickRate)
  Complexity   Param 2       Return 1      Cyclo Vg 1       Total        4
  LOC 10       eLOC 8        lLOC 7        Comment 5        Lines       15

  Function: QP::QXThread::delayCancel
  Parameters: (void)
  Complexity   Param 0       Return 1      Cyclo Vg 1       Total        2
  LOC 8        eLOC 7        lLOC 5        Comment 2        Lines       10

  Function: QP::QXThread::queueGet
  Parameters: (uint_fast16_t const nTicks, uint_fast8_t const tickRate)
  Complexity   Param 2       Return 1      Cyclo Vg 6       Total        9
  LOC 56       eLOC 48       lLOC 32       Comment 24       Lines       74

  Total: Functions  
  LOC 3374     eLOC 2803     lLOC 1725     InCmp 401      CycloCmp     558
  Function Points         FP(LOC) 59.5    FP(eLOC) 49.9   FP(lLOC)    30.9

  ------------------------------------------------------------------------

                      ~~ Project Functional Analysis ~~

  Total Functions .......:        196  Total Physical Lines ..:       4393
  Total LOC .............:       3374  Total Function Pts LOC :       59.5
  Total eLOC ............:       2803  Total Function Pts eLOC:       49.9
  Total lLOC.............:       1725  Total Function Pts lLOC:       30.9
  Total Cyclomatic Comp. :        558  Total Interface Comp. .:        401
  Total Parameters ......:        205  Total Return Points ...:        196
  Total Comment Lines ...:       2541  Total Blank Lines .....:        516
         ------     -----     -----     ------     ------     -----       
  Avg Physical Lines ....:      22.41
  Avg LOC ...............:      17.21  Avg eLOC ..............:      14.30
  Avg lLOC ..............:       8.80  Avg Cyclomatic Comp. ..:       2.85
  Avg Interface Comp. ...:       2.05  Avg Parameters ........:       1.05
  Avg Return Points .....:       1.00  Avg Comment Lines .....:      12.96
         ------     -----     -----     ------     ------     -----       
  Max LOC ...............:        260
  Max eLOC ..............:        202  Max lLOC ..............:        125
  Max Cyclomatic Comp. ..:         57  Max Interface Comp. ...:          7
  Max Parameters ........:          6  Max Return Points .....:          1
  Max Comment Lines .....:        105  Max Total Lines .......:        273
         ------     -----     -----     ------     ------     -----       
  Min LOC ...............:          1
  Min eLOC ..............:          1  Min lLOC ..............:          0
  Min Cyclomatic Comp. ..:          1  Min Interface Comp. ...:          1
  Min Parameters ........:          0  Min Return Points .....:          1
  Min Comment Lines .....:          1  Min Total Lines .......:          1

  ------------------------------------------------------------------------

                             ~~ File Summary ~~

  C Source Files *.c ....:          0  C/C++ Include Files *.h:         18
  C++ Source Files *.c* .:         23  C++ Include Files *.h* :          0
  C# Source Files *.cs ..:          0  Java Source File *.jav*:          0
  Other File Count ......:          0  Total File Count ......:         41
  ________________________________________________________________________

@endcode 
*/ 
