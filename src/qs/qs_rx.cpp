/// @file
/// @brief QS receive channel services
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.5.1
/// Last updated on  2019-05-22
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2019 Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qs_port.h"      // QS port
#include "qs_pkg.h"       // QS package-scope internal interface
#include "qf_pkg.h"       // QF package-scope internal interface
#include "qassert.h"      // QP assertions

namespace QP {

Q_DEFINE_THIS_MODULE("qs_rx")

//****************************************************************************
QS::QSrxPriv QS::rxPriv_; // QS-RX private data

//****************************************************************************
#if (QS_OBJ_PTR_SIZE == 1)
    typedef uint8_t QSObj;
#elif (QS_OBJ_PTR_SIZE == 2)
    typedef uint16_t QSObj;
#elif (QS_OBJ_PTR_SIZE == 4)
    typedef uint32_t QSObj;
#elif (QS_OBJ_PTR_SIZE == 8)
    typedef uint64_t QSObj;
#endif

#if (QS_FUN_PTR_SIZE == 1)
    typedef uint8_t QSFun;
#elif (QS_FUN_PTR_SIZE == 2)
    typedef uint16_t QSFun;
#elif (QS_FUN_PTR_SIZE == 4)
    typedef uint32_t QSFun;
#elif (QS_FUN_PTR_SIZE == 8)
    typedef uint64_t QSFun;
#endif

/// @cond
/// Exlcude the following internals from the Doxygen documentation
/// Extended-state variables used for parsing various QS-RX Records
struct CmdVar {
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint8_t  idx;
    uint8_t  cmdId;
};

struct TickVar {
    uint_fast8_t rate;
};

struct PeekVar {
    uint16_t offs;
    uint8_t  size;
    uint8_t  num;
    uint8_t  idx;
};

struct PokeVar {
    uint32_t data;
    uint16_t offs;
    uint8_t  size;
    uint8_t  num;
    uint8_t  idx;
    uint8_t  fill;
};

struct GFltVar {
    uint8_t data[16];
    uint8_t idx;
};

struct ObjVar {
    QSObj   addr;
    uint8_t idx;
    uint8_t kind; // see qs.h, enum QSpyObjKind
    uint8_t recId;
};

struct TPVar {  // Test-Probe
    QSFun    addr;
    uint32_t data;
    uint8_t  idx;
};

struct AFltVar {
    uint8_t prio;
};

struct EvtVar {
    QEvt    *e;
    uint8_t *p;
    QSignal  sig;
    uint16_t len;
    uint8_t  prio;
    uint8_t  idx;
};

// extended-state variables for the current QS-RX state
static struct ExtState {
    union Variant {
        CmdVar   cmd;
        TickVar  tick;
        PeekVar  peek;
        PokeVar  poke;
        GFltVar  gFlt;
        AFltVar  aFlt;
        ObjVar   obj;
        EvtVar   evt;
        TPVar    tp;
    } var;
    uint8_t state;
    uint8_t esc;
    uint8_t seq;
    uint8_t chksum;
} l_rx;

enum RxStateEnum {
    WAIT4_SEQ,
    WAIT4_REC,
    WAIT4_INFO_FRAME,
    WAIT4_CMD_ID,
    WAIT4_CMD_PARAM1,
    WAIT4_CMD_PARAM2,
    WAIT4_CMD_PARAM3,
    WAIT4_CMD_FRAME,
    WAIT4_RESET_FRAME,
    WAIT4_TICK_RATE,
    WAIT4_TICK_FRAME,
    WAIT4_PEEK_OFFS,
    WAIT4_PEEK_SIZE,
    WAIT4_PEEK_NUM,
    WAIT4_PEEK_FRAME,
    WAIT4_POKE_OFFS,
    WAIT4_POKE_SIZE,
    WAIT4_POKE_NUM,
    WAIT4_POKE_DATA,
    WAIT4_POKE_FRAME,
    WAIT4_FILL_DATA,
    WAIT4_FILL_FRAME,
    WAIT4_GLB_FILTER_LEN,
    WAIT4_GLB_FILTER_DATA,
    WAIT4_GLB_FILTER_FRAME,
    WAIT4_AO_FILTER_PRIO,
    WAIT4_AO_FILTER_FRAME,
    WAIT4_OBJ_KIND,
    WAIT4_OBJ_ADDR,
    WAIT4_OBJ_FRAME,
    WAIT4_QUERY_KIND,
    WAIT4_QUERY_FRAME,
    WAIT4_EVT_PRIO,
    WAIT4_EVT_SIG,
    WAIT4_EVT_LEN,
    WAIT4_EVT_PAR,
    WAIT4_EVT_FRAME,
    WAIT4_TEST_SETUP_FRAME,
    WAIT4_TEST_TEARDOWN_FRAME,
    WAIT4_TEST_PROBE_DATA,
    WAIT4_TEST_PROBE_ADDR,
    WAIT4_TEST_PROBE_FRAME,
    WAIT4_TEST_CONTINUE_FRAME,
    ERROR_STATE
};

#ifdef Q_UTEST
    static struct TestData {
        TPVar     tpBuf[16]; // buffer of Test-Probes received so far
        uint8_t   tpNum;     // current number of Test-Probes
        QSTimeCtr testTime;  // test time (tick counter)
    } l_testData;
#endif // Q_UTEST

// internal helper functions...
static void rxParseData_(uint8_t b);
static void rxHandleBadFrame_(uint8_t state);
static void rxReportAck_(enum QSpyRxRecords recId);
static void rxReportError_(uint8_t code);
static void rxReportDone_(enum QSpyRxRecords recId);
static void rxPoke_(void);

//! Internal QS-RX function to take a transition in the QS-RX FSM
static inline void tran_(RxStateEnum const target) {
    l_rx.state = static_cast<uint8_t>(target);
}
/// @endcond


//****************************************************************************
/// @description
/// This function should be called from QS::onStartup() to provide QS-RX with
/// the receive data buffer.
///
/// @param[in]  sto[]   the address of the memory block
/// @param[in]  stoSize the size of this block [bytes]. The size of the
///                     QS RX buffer cannot exceed 64KB.
///
/// @note QS-RX can work with quite small data buffers, but you will start
/// losing data if the buffer is not drained fast enough in the idle task.
///
/// @note If the data input rate exceeds the QS-RX processing rate, the data
/// will be lost, but the QS protocol will notice that:
/// (1) that the checksum in the incomplete QS records will fail; and
/// (2) the sequence counter in QS records will show discontinuities.
///
/// The QS-RX channel will report any data errors by sending the
/// QS_RX_DATA_ERROR trace record.
///
void QS::rxInitBuf(uint8_t sto[], uint16_t const stoSize) {
    rxPriv_.buf  = &sto[0];
    rxPriv_.end  = static_cast<QSCtr>(stoSize) - static_cast<QSCtr>(1);
    rxPriv_.head = static_cast<QSCtr>(0);
    rxPriv_.tail = static_cast<QSCtr>(0);

    rxPriv_.currObj[QS::SM_OBJ] = static_cast<void *>(0);
    rxPriv_.currObj[QS::AO_OBJ] = static_cast<void *>(0);
    rxPriv_.currObj[QS::MP_OBJ] = static_cast<void *>(0);
    rxPriv_.currObj[QS::EQ_OBJ] = static_cast<void *>(0);
    rxPriv_.currObj[QS::TE_OBJ] = static_cast<void *>(0);
    rxPriv_.currObj[QS::AP_OBJ] = static_cast<void *>(0);

    tran_(WAIT4_SEQ);
    l_rx.esc    = static_cast<uint8_t>(0);
    l_rx.seq    = static_cast<uint8_t>(0);
    l_rx.chksum = static_cast<uint8_t>(0);

    beginRec(static_cast<uint_fast8_t>(QS_OBJ_DICT));
        QS_OBJ_(&rxPriv_);
        QS_STR_("QS_RX");
    endRec();
    // no QS_REC_DONE(), because QS is not running yet

#ifdef Q_UTEST
    l_testData.tpNum    = static_cast<uint8_t>(0);
    l_testData.testTime = static_cast<QSTimeCtr>(0);
#endif // Q_UTEST
}

//****************************************************************************
/// @description
/// This function is intended to be called from the ISR that reads the QS-RX
/// bytes from the QSPY application. The function returns the conservative
/// number of free bytes currently available in the buffer, assuming that
/// the head pointer is not being moved concurrently. The tail pointer might
/// be moving, meaning that bytes can be concurrently removed from the buffer.
///
uint16_t QS::rxGetNfree(void) {
    uint16_t nFree;
    if (rxPriv_.head == rxPriv_.tail) {
        nFree = static_cast<uint16_t>(rxPriv_.end);
    }
    else if (rxPriv_.head < rxPriv_.tail) {
        nFree = static_cast<uint16_t>(rxPriv_.tail - rxPriv_.head);
    }
    else {
        nFree = static_cast<uint16_t>((rxPriv_.tail + rxPriv_.end)
                                      - rxPriv_.head);
    }
    return nFree;
}

//****************************************************************************
void QS::rxParse(void) {
    while (rxPriv_.head != rxPriv_.tail) { // QS-RX buffer not empty?
        uint8_t b = QS_PTR_AT_(rxPriv_.buf, rxPriv_.tail);

        if (rxPriv_.tail != static_cast<QSCtr>(0)) {
            --rxPriv_.tail;
        }
        else {
             rxPriv_.tail = rxPriv_.end;
        }

        if (l_rx.esc != static_cast<uint8_t>(0)) {  // escaped byte arrived?
            l_rx.esc = static_cast<uint8_t>(0);
            b ^= QS_ESC_XOR;

            l_rx.chksum += b;
            rxParseData_(b);
        }
        else if (b == QS_ESC) {
            l_rx.esc = static_cast<uint8_t>(1);
        }
        else if (b == QS_FRAME) {
            // get ready for the next frame
            b = l_rx.state; // save the current state in b
            l_rx.esc = static_cast<uint8_t>(0);
            tran_(WAIT4_SEQ);

            if (l_rx.chksum == QS_GOOD_CHKSUM) {
                l_rx.chksum = static_cast<uint8_t>(0);
                rxHandleGoodFrame_(b);
            }
            else { // bad checksum
                l_rx.chksum = static_cast<uint8_t>(0);
                rxReportError_(static_cast<uint8_t>(0x00));
                rxHandleBadFrame_(b);
            }
        }
        else {
            l_rx.chksum += b;
            rxParseData_(b);
        }
    }
}

//****************************************************************************
static void rxParseData_(uint8_t const b) {
    switch (l_rx.state) {
        case WAIT4_SEQ: {
            ++l_rx.seq;
            if (l_rx.seq != b) { // not the expected sequence?
                rxReportError_(static_cast<uint8_t>(0x42));
                l_rx.seq = b; // update the sequence
            }
            tran_(WAIT4_REC);
            break;
        }
        case WAIT4_REC: {
            switch (b) {
                case QS_RX_INFO:
                    tran_(WAIT4_INFO_FRAME);
                    break;
                case QS_RX_COMMAND:
                    tran_(WAIT4_CMD_ID);
                    break;
                case QS_RX_RESET:
                    tran_(WAIT4_RESET_FRAME);
                    break;
                case QS_RX_TICK:
                    tran_(WAIT4_TICK_RATE);
                    break;
                case QS_RX_PEEK:
                    if (QS::rxPriv_.currObj[QS::AP_OBJ]
                        != static_cast<void *>(0))
                    {
                        l_rx.var.peek.offs = static_cast<uint16_t>(0);
                        l_rx.var.peek.idx  = static_cast<uint8_t>(0);
                        tran_(WAIT4_PEEK_OFFS);
                    }
                    else {
                        rxReportError_(static_cast<uint8_t>(QS_RX_PEEK));
                        tran_(ERROR_STATE);
                    }
                    break;
                case QS_RX_POKE:
                case QS_RX_FILL:
                    l_rx.var.poke.fill =
                        (b == static_cast<uint8_t>(QS_RX_FILL))
                            ? static_cast<uint8_t>(1)
                            : static_cast<uint8_t>(0);
                    if (QS::rxPriv_.currObj[QS::AP_OBJ]
                        != static_cast<void *>(0))
                    {
                        l_rx.var.poke.offs = static_cast<uint16_t>(0);
                        l_rx.var.poke.idx  = static_cast<uint8_t>(0);
                        tran_(WAIT4_POKE_OFFS);
                    }
                    else {
                        rxReportError_(
                            (l_rx.var.poke.fill != static_cast<uint8_t>(0))
                                ? static_cast<uint8_t>(QS_RX_FILL)
                                : static_cast<uint8_t>(QS_RX_FILL));
                        tran_(ERROR_STATE);
                    }
                    break;
                case QS_RX_GLB_FILTER:
                    tran_(WAIT4_GLB_FILTER_LEN);
                    break;
                case QS_RX_LOC_FILTER:
                    l_rx.var.obj.recId =
                        static_cast<uint8_t>(QS_RX_LOC_FILTER);
                    tran_(WAIT4_OBJ_KIND);
                    break;
                case QS_RX_AO_FILTER:
                    tran_(WAIT4_AO_FILTER_PRIO);
                    break;
                case QS_RX_CURR_OBJ:
                    l_rx.var.obj.recId = static_cast<uint8_t>(QS_RX_CURR_OBJ);
                    tran_(WAIT4_OBJ_KIND);
                    break;
                case QS_RX_QUERY_CURR:
                    l_rx.var.obj.recId = static_cast<uint8_t>(QS_RX_QUERY_CURR);
                    tran_(WAIT4_QUERY_KIND);
                    break;
                case QS_RX_EVENT:
                    tran_(WAIT4_EVT_PRIO);
                    break;

#ifdef Q_UTEST
                case QS_RX_TEST_SETUP:
                    tran_(WAIT4_TEST_SETUP_FRAME);
                    break;
                case QS_RX_TEST_TEARDOWN:
                    tran_(WAIT4_TEST_TEARDOWN_FRAME);
                    break;
                case QS_RX_TEST_CONTINUE:
                    tran_(WAIT4_TEST_CONTINUE_FRAME);
                    break;
                case QS_RX_TEST_PROBE:
                    if (l_testData.tpNum
                        < static_cast<uint8_t>(
                              (sizeof(l_testData.tpBuf)
                               / sizeof(l_testData.tpBuf[0]))))
                    {
                        l_rx.var.tp.data = static_cast<uint32_t>(0);
                        l_rx.var.tp.idx  = static_cast<uint8_t>(0);
                        tran_(WAIT4_TEST_PROBE_DATA);
                    }
                    else { // the number of Test-Probes exceeded
                        rxReportError_(
                            static_cast<uint8_t>(QS_RX_TEST_PROBE));
                        tran_(ERROR_STATE);
                    }
                    break;
#endif // Q_UTEST

                default:
                    rxReportError_(static_cast<uint8_t>(0x43U));
                    tran_(ERROR_STATE);
                    break;
            }
            break;
        }
        case WAIT4_INFO_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_CMD_ID: {
            l_rx.var.cmd.cmdId  = b;
            l_rx.var.cmd.idx    = static_cast<uint8_t>(0);
            l_rx.var.cmd.param1 = static_cast<uint32_t>(0);
            l_rx.var.cmd.param2 = static_cast<uint32_t>(0);
            l_rx.var.cmd.param3 = static_cast<uint32_t>(0);
            tran_(WAIT4_CMD_PARAM1);
            break;
        }
        case WAIT4_CMD_PARAM1: {
            l_rx.var.cmd.param1 |=
                (static_cast<uint32_t>(b) << l_rx.var.cmd.idx);
            l_rx.var.cmd.idx    += static_cast<uint8_t>(8);
            if (l_rx.var.cmd.idx == static_cast<uint8_t>((8*4))) {
                l_rx.var.cmd.idx = static_cast<uint8_t>(0);
                tran_(WAIT4_CMD_PARAM2);
            }
            break;
        }
        case WAIT4_CMD_PARAM2: {
            l_rx.var.cmd.param2 |=
                static_cast<uint32_t>(b) << l_rx.var.cmd.idx;
            l_rx.var.cmd.idx    += static_cast<uint8_t>(8);
            if (l_rx.var.cmd.idx == static_cast<uint8_t>((8*4))) {
                l_rx.var.cmd.idx = static_cast<uint8_t>(0);
                tran_(WAIT4_CMD_PARAM3);
            }
            break;
        }
        case WAIT4_CMD_PARAM3: {
            l_rx.var.cmd.param3 |=
                static_cast<uint32_t>(b) << l_rx.var.cmd.idx;
            l_rx.var.cmd.idx    += static_cast<uint8_t>(8);
            if (l_rx.var.cmd.idx == static_cast<uint8_t>((8*4))) {
                l_rx.var.cmd.idx = static_cast<uint8_t>(0);
                tran_(WAIT4_CMD_FRAME);
            }
            break;
        }
        case WAIT4_CMD_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_RESET_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TICK_RATE: {
            l_rx.var.tick.rate = static_cast<uint_fast8_t>(b);
            tran_(WAIT4_TICK_FRAME);
            break;
        }
        case WAIT4_TICK_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_PEEK_OFFS: {
            if (l_rx.var.peek.idx == static_cast<uint8_t>(0)) {
                l_rx.var.peek.offs = static_cast<uint16_t>(b);
                l_rx.var.peek.idx += static_cast<uint8_t>(8);
            }
            else {
                l_rx.var.peek.offs |= static_cast<uint16_t>(
                                          static_cast<uint16_t>(b) << 8);
                tran_(WAIT4_PEEK_SIZE);
            }
            break;
        }
        case WAIT4_PEEK_SIZE: {
            if ((b == static_cast<uint8_t>(1))
                    || (b == static_cast<uint8_t>(2))
                    || (b == static_cast<uint8_t>(4)))
            {
                l_rx.var.peek.size = b;
                tran_(WAIT4_PEEK_NUM);
            }
            else {
                rxReportError_(static_cast<uint8_t>(QS_RX_PEEK));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_PEEK_NUM: {
            l_rx.var.peek.num = b;
            tran_(WAIT4_PEEK_FRAME);
            break;
        }
        case WAIT4_PEEK_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_POKE_OFFS: {
            if (l_rx.var.poke.idx == static_cast<uint8_t>(0)) {
                l_rx.var.poke.offs = static_cast<uint16_t>(b);
                l_rx.var.poke.idx  = static_cast<uint8_t>(1);
            }
            else {
                l_rx.var.poke.offs |= static_cast<uint16_t>(
                                          static_cast<uint16_t>(b) << 8);
                tran_(WAIT4_POKE_SIZE);
            }
            break;
        }
        case WAIT4_POKE_SIZE: {
            if ((b == static_cast<uint8_t>(1))
                    || (b == static_cast<uint8_t>(2))
                    || (b == static_cast<uint8_t>(4)))
            {
                l_rx.var.poke.size = b;
                tran_(WAIT4_POKE_NUM);
            }
            else {
                rxReportError_((l_rx.var.poke.fill != static_cast<uint8_t>(0))
                                  ? static_cast<uint8_t>(QS_RX_FILL)
                                  : static_cast<uint8_t>(QS_RX_POKE));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_POKE_NUM: {
            if (b > static_cast<uint8_t>(0)) {
                l_rx.var.poke.num  = b;
                l_rx.var.poke.data = static_cast<uint32_t>(0);
                l_rx.var.poke.idx  = static_cast<uint8_t>(0);
                tran_((l_rx.var.poke.fill != static_cast<uint8_t>(0))
                            ? WAIT4_FILL_DATA
                            : WAIT4_POKE_DATA);
            }
            else {
                rxReportError_((l_rx.var.poke.fill != static_cast<uint8_t>(0))
                                  ? static_cast<uint8_t>(QS_RX_FILL)
                                  : static_cast<uint8_t>(QS_RX_POKE));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_FILL_DATA: {
            l_rx.var.poke.data |=
                static_cast<uint32_t>(b) << l_rx.var.poke.idx;
            l_rx.var.poke.idx += static_cast<uint8_t>(8);
            if ((l_rx.var.poke.idx >> 3) == l_rx.var.poke.size) {
                tran_(WAIT4_FILL_FRAME);
            }
            break;
        }
        case WAIT4_POKE_DATA: {
            l_rx.var.poke.data |=
                static_cast<uint32_t>(b) << l_rx.var.poke.idx;
            l_rx.var.poke.idx += static_cast<uint8_t>(8);
            if ((l_rx.var.poke.idx >> 3) == l_rx.var.poke.size) {
                rxPoke_();
                --l_rx.var.poke.num;
                if (l_rx.var.poke.num == static_cast<uint8_t>(0)) {
                    tran_(WAIT4_POKE_FRAME);
                }
            }
            break;
        }
        case WAIT4_FILL_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_POKE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_GLB_FILTER_LEN: {
            if (b == static_cast<uint8_t>(sizeof(l_rx.var.gFlt.data))) {
                l_rx.var.gFlt.idx = static_cast<uint8_t>(0);
                tran_(WAIT4_GLB_FILTER_DATA);
            }
            else {
                rxReportError_(static_cast<uint8_t>(QS_RX_GLB_FILTER));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_GLB_FILTER_DATA: {
            l_rx.var.gFlt.data[l_rx.var.gFlt.idx] = b;
            ++l_rx.var.gFlt.idx;
            if (l_rx.var.gFlt.idx
                == static_cast<uint8_t>(sizeof(l_rx.var.gFlt.data)))
            {
                tran_(WAIT4_GLB_FILTER_FRAME);
            }
            break;
        }
        case WAIT4_GLB_FILTER_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_OBJ_KIND: {
            if (b <= static_cast<uint8_t>(QS::SM_AO_OBJ)) {
                l_rx.var.obj.kind = b;
                l_rx.var.obj.addr = static_cast<QSObj>(0);
                l_rx.var.obj.idx  = static_cast<uint8_t>(0);
                tran_(WAIT4_OBJ_ADDR);
            }
            else {
                rxReportError_(l_rx.var.obj.recId);
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_OBJ_ADDR: {
            l_rx.var.obj.addr |=
                static_cast<uint32_t>(b) << l_rx.var.obj.idx;
            l_rx.var.obj.idx += static_cast<uint8_t>(8);
            if (l_rx.var.obj.idx
                == static_cast<uint8_t>((8*QS_OBJ_PTR_SIZE)))
            {
                tran_(WAIT4_OBJ_FRAME);
            }
            break;
        }
        case WAIT4_OBJ_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_QUERY_KIND: {
            if (b < static_cast<uint8_t>(QS::MAX_OBJ)) {
                l_rx.var.obj.kind = b;
                tran_(WAIT4_QUERY_FRAME);
            }
            else {
                rxReportError_(l_rx.var.obj.recId);
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_QUERY_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_AO_FILTER_PRIO: {
            l_rx.var.aFlt.prio = b;
            tran_(WAIT4_AO_FILTER_FRAME);
            break;
        }
        case WAIT4_AO_FILTER_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_EVT_PRIO: {
            l_rx.var.evt.prio = b;
            l_rx.var.evt.sig  = static_cast<QSignal>(0);
            l_rx.var.evt.idx  = static_cast<uint8_t>(0);
            tran_(WAIT4_EVT_SIG);
            break;
        }
        case WAIT4_EVT_SIG: {
            l_rx.var.evt.sig |= static_cast<QSignal>(
                                static_cast<uint32_t>(b) << l_rx.var.evt.idx);
            l_rx.var.evt.idx += static_cast<uint8_t>(8);
            if (l_rx.var.evt.idx == static_cast<uint8_t>((8*Q_SIGNAL_SIZE))) {
                l_rx.var.evt.len = static_cast<uint16_t>(0);
                l_rx.var.evt.idx = static_cast<uint8_t>(0);
                tran_(WAIT4_EVT_LEN);
            }
            break;
        }
        case WAIT4_EVT_LEN: {
            l_rx.var.evt.len |= static_cast<uint16_t>(
                                static_cast<uint32_t>(b) << l_rx.var.evt.idx);
            l_rx.var.evt.idx += static_cast<uint8_t>(8);
            if (l_rx.var.evt.idx == static_cast<uint8_t>((8*2))) {
                if ((l_rx.var.evt.len + static_cast<uint16_t>(sizeof(QEvt)))
                    <= static_cast<uint16_t>(QF::poolGetMaxBlockSize()))
                {
                    // report Ack before generating any other QS records
                    rxReportAck_(QS_RX_EVENT);

                    l_rx.var.evt.e = QF::newX_(
                        (static_cast<uint_fast16_t>(l_rx.var.evt.len)
                         + static_cast<uint_fast16_t>(sizeof(QEvt))),
                        static_cast<uint_fast16_t>(0), // margin
                        static_cast<enum_t>(l_rx.var.evt.sig));
                    // event allocated?
                    if (l_rx.var.evt.e != static_cast<QEvt *>(0)) {
                        l_rx.var.evt.p =
                            reinterpret_cast<uint8_t *>(l_rx.var.evt.e);
                        l_rx.var.evt.p += sizeof(QEvt);
                        if (l_rx.var.evt.len > static_cast<uint16_t>(0)) {
                            tran_(WAIT4_EVT_PAR);
                        }
                        else {
                            tran_(WAIT4_EVT_FRAME);
                        }
                    }
                    else {
                        rxReportError_(static_cast<uint8_t>(QS_RX_EVENT));
                        tran_(ERROR_STATE);
                    }
                }
                else {
                    rxReportError_(static_cast<uint8_t>(QS_RX_EVENT));
                    tran_(ERROR_STATE);
                }
            }
            break;
        }
        case WAIT4_EVT_PAR: { // event parameters
            *l_rx.var.evt.p = b;
            ++l_rx.var.evt.p;
            --l_rx.var.evt.len;
            if (l_rx.var.evt.len == static_cast<uint16_t>(0)) {
                tran_(WAIT4_EVT_FRAME);
            }
            break;
        }
        case WAIT4_EVT_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }

#ifdef Q_UTEST
        case WAIT4_TEST_SETUP_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_TEARDOWN_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_CONTINUE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_TEST_PROBE_DATA: {
            l_rx.var.tp.data |= static_cast<uint32_t>(b) << l_rx.var.tp.idx;
            l_rx.var.tp.idx += static_cast<uint8_t>(8);
            if (l_rx.var.tp.idx
                == static_cast<uint8_t>(8U*sizeof(uint32_t)))
            {
                l_rx.var.tp.addr = static_cast<uint32_t>(0);
                l_rx.var.tp.idx  = static_cast<uint8_t>(0);
                tran_(WAIT4_TEST_PROBE_ADDR);
            }
            break;
        }
        case WAIT4_TEST_PROBE_ADDR: {
            l_rx.var.tp.addr |= static_cast<uint32_t>(b) << l_rx.var.tp.idx;
            l_rx.var.tp.idx += static_cast<uint8_t>(8);
            if (l_rx.var.tp.idx == static_cast<uint8_t>(8*QS_FUN_PTR_SIZE)) {
                tran_(WAIT4_TEST_PROBE_FRAME);
            }
            break;
        }
        case WAIT4_TEST_PROBE_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
#endif // Q_UTEST

        case ERROR_STATE: {
            // keep ignoring the data until a good frame is collected
            break;
        }
        default: { // unexpected or unimplemented state
            rxReportError_(static_cast<uint8_t>(0x45));
            tran_(ERROR_STATE);
            break;
        }
    }
}

//****************************************************************************
void QS::rxHandleGoodFrame_(uint8_t state) {
    uint8_t i;
    uint8_t *ptr;

    switch (state) {
        case WAIT4_INFO_FRAME: {
            // no need to report Ack or Done
            QS_target_info_(static_cast<uint8_t>(0)); // send only Target info
            break;
        }
        case WAIT4_RESET_FRAME: {
            // no need to report Ack or Done, because Target resets
            QS::onReset(); // reset the Target
            break;
        }
        case WAIT4_CMD_PARAM1: // intentionally fall-through
        case WAIT4_CMD_PARAM2: // intentionally fall-through
        case WAIT4_CMD_PARAM3: // intentionally fall-through
        case WAIT4_CMD_FRAME: {
            rxReportAck_(QS_RX_COMMAND);
            QS::onCommand(l_rx.var.cmd.cmdId, l_rx.var.cmd.param1,
                         l_rx.var.cmd.param2, l_rx.var.cmd.param3);
#ifdef Q_UTEST
            QS::processTestEvts_(); // process all events produced
#endif
            rxReportDone_(QS_RX_COMMAND);
            break;
        }
        case WAIT4_TICK_FRAME: {
            rxReportAck_(QS_RX_TICK);
#ifdef Q_UTEST
            QS::tickX_(static_cast<uint_fast8_t>(l_rx.var.tick.rate),
                       &QS::rxPriv_); // process tick
            QS::processTestEvts_(); // process all events produced
#else
            QF::tickX_(static_cast<uint_fast8_t>(l_rx.var.tick.rate),
                       &QS::rxPriv_);
#endif
            rxReportDone_(QS_RX_TICK);
            break;
        }
        case WAIT4_PEEK_FRAME: {
            // no need to report Ack or Done
            QS::beginRec(static_cast<uint_fast8_t>(QS_PEEK_DATA));
                ptr = (static_cast<uint8_t *>(QS::rxPriv_.currObj[QS::AP_OBJ])
                       + l_rx.var.peek.offs);
                QS_TIME_();                  // timestamp
                QS_U16_(l_rx.var.peek.offs); // data offset
                QS_U8_(l_rx.var.peek.size);  // data size
                QS_U8_(l_rx.var.peek.num);   // number of data items
                for (i =static_cast<uint8_t>(0); i < l_rx.var.peek.num; ++i) {
                    switch (l_rx.var.peek.size) {
                        case 1:
                            QS_U8_(*(ptr + i));
                            break;
                        case 2:
                            QS_U16_(*(reinterpret_cast<uint16_t *>(ptr) + i));
                            break;
                        case 4:
                            QS_U32_(*(reinterpret_cast<uint32_t *>(ptr) + i));
                            break;
                        default:
                            break;
                    }
                }
            QS::endRec();
            QS_REC_DONE();
            break;
        }
        case WAIT4_POKE_DATA: {
            // received less than expected poke data items
            rxReportError_(static_cast<uint8_t>(QS_RX_POKE));
            break;
        }
        case WAIT4_POKE_FRAME: {
            rxReportAck_(QS_RX_POKE);
            // no need to report done
            break;
        }
        case WAIT4_FILL_FRAME: {
            rxReportAck_(QS_RX_FILL);
            ptr = (static_cast<uint8_t *>(QS::rxPriv_.currObj[QS::AP_OBJ])
                   + l_rx.var.poke.offs);
            for (i = static_cast<uint8_t>(0); i < l_rx.var.poke.num; ++i) {
                switch (l_rx.var.poke.size) {
                    case 1:
                        *(ptr + i) = static_cast<uint8_t>(l_rx.var.poke.data);
                        break;
                    case 2:
                        *(reinterpret_cast<uint16_t *>(ptr) + i) =
                            static_cast<uint16_t>(l_rx.var.poke.data);
                        break;
                    case 4:
                        *(reinterpret_cast<uint32_t *>(ptr) + i) =
                            l_rx.var.poke.data;
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case WAIT4_GLB_FILTER_FRAME: {
            rxReportAck_(QS_RX_GLB_FILTER);

            // never disable the non-maskable records
            l_rx.var.gFlt.data[0] |= static_cast<uint8_t>(0x01);
            l_rx.var.gFlt.data[7] |= static_cast<uint8_t>(0xFC);
            l_rx.var.gFlt.data[8] |= static_cast<uint8_t>(0x3F);

            // never enable the last 3 records (0x7D, 0x7E, 0x7F)
            l_rx.var.gFlt.data[15] &= static_cast<uint8_t>(0x1F);

            for (i = static_cast<uint8_t>(0);
                 i < static_cast<uint8_t>(sizeof(QS::priv_.glbFilter)); ++i)
            {
                QS::priv_.glbFilter[i] = l_rx.var.gFlt.data[i];
            }
            // no need to report Done
            break;
        }
        case WAIT4_OBJ_FRAME: {
            i = l_rx.var.obj.kind;
            if (i < static_cast<uint8_t>(QS::MAX_OBJ)) {
                if (l_rx.var.obj.recId
                    == static_cast<uint8_t>(QS_RX_LOC_FILTER))
                {
                    QS::priv_.locFilter[i] =
                         reinterpret_cast<void *>(l_rx.var.obj.addr);
                }
                else {
                    QS::rxPriv_.currObj[i] =
                        reinterpret_cast<void *>(l_rx.var.obj.addr);
                }
                rxReportAck_(
                    static_cast<enum QSpyRxRecords>(l_rx.var.obj.recId));
            }
            // both SM and AO
            else if (i == static_cast<uint8_t>(QS::SM_AO_OBJ)) {
                if (l_rx.var.obj.recId
                    == static_cast<uint8_t>(QS_RX_LOC_FILTER))
                {
                    QS::priv_.locFilter[QS::SM_OBJ] =
                        reinterpret_cast<void *>(l_rx.var.obj.addr);
                    QS::priv_.locFilter[QS::AO_OBJ] =
                        reinterpret_cast<void *>(l_rx.var.obj.addr);
                }
                else {
                    QS::rxPriv_.currObj[QS::SM_OBJ] =
                        reinterpret_cast<void *>(l_rx.var.obj.addr);
                    QS::rxPriv_.currObj[QS::AO_OBJ] =
                        reinterpret_cast<void *>(l_rx.var.obj.addr);
                }
                rxReportAck_(
                    static_cast<enum QSpyRxRecords>(l_rx.var.obj.recId));
            }
            else {
                rxReportError_(l_rx.var.obj.recId);
            }
            break;
        }
        case WAIT4_QUERY_FRAME: {
            i = l_rx.var.obj.kind;
            ptr = reinterpret_cast<uint8_t *>(QS::rxPriv_.currObj[i]);
            if (ptr != static_cast<void *>(0)) {
                QS::beginRec(static_cast<uint_fast8_t>(QS_QUERY_DATA));
                    QS_TIME_(); // timestamp
                    QS_U8_(i);  // object kind
                    QS_OBJ_(ptr);
                    switch (i) {
                        case QS::SM_OBJ:
                            QS_FUN_(reinterpret_cast<QHsm *>(ptr)->m_state.fun);
                            break;

#ifdef Q_UTEST
                        case QS::AO_OBJ:
                            QS_EQC_(reinterpret_cast<QActive *>(ptr)->m_eQueue.m_nFree);
                            QS_EQC_(reinterpret_cast<QActive *>(ptr)->m_eQueue.m_nMin);
                            break;
                        case QS::MP_OBJ:
                            QS_MPC_(reinterpret_cast<QMPool *>(ptr)->m_nFree);
                            QS_MPC_(reinterpret_cast<QMPool *>(ptr)->m_nMin);
                            break;
                        case QS::EQ_OBJ:
                            QS_EQC_(reinterpret_cast<QEQueue *>(ptr)->m_nFree);
                            QS_EQC_(reinterpret_cast<QEQueue *>(ptr)->m_nMin);
                            break;
                        case QS::TE_OBJ:
                            QS_OBJ_(reinterpret_cast<QTimeEvt *>(ptr)->m_act);
                            QS_TEC_(reinterpret_cast<QTimeEvt *>(ptr)->m_ctr);
                            QS_TEC_(reinterpret_cast<QTimeEvt *>(ptr)->m_interval);
                            QS_SIG_(reinterpret_cast<QTimeEvt *>(ptr)->sig);
                            QS_U8_ (reinterpret_cast<QTimeEvt *>(ptr)->refCtr_);
                            break;
#endif // Q_UTEST

                        default:
                            break;
                    }
                QS::endRec();
                QS_REC_DONE();
            }
            else {
                rxReportError_(static_cast<uint8_t>(QS_RX_QUERY_CURR));
            }
            break;
        }
        case WAIT4_AO_FILTER_FRAME: {
            rxReportAck_(QS_RX_AO_FILTER);
            if (l_rx.var.aFlt.prio <= static_cast<uint8_t>(QF_MAX_ACTIVE)) {
                rxReportAck_(QS_RX_AO_FILTER);
                QS::priv_.locFilter[QS::AO_OBJ] =
                    QF::active_[l_rx.var.aFlt.prio];
                QS::priv_.locFilter[QS::SM_OBJ] =
                    QF::active_[l_rx.var.aFlt.prio];
            }
            else {
                rxReportError_(static_cast<uint8_t>(QS_RX_AO_FILTER));
            }
            // no need to report Done
            break;
        }
        case WAIT4_EVT_FRAME: {
            // NOTE: Ack was already reported in the WAIT4_EVT_LEN state
#ifdef Q_UTEST
            QS::onTestEvt(l_rx.var.evt.e); // "massage" the event, if needed
#endif // Q_UTEST
            // use 'i' as status, 0 == success,no-recycle
            i = static_cast<uint8_t>(0);

            if (l_rx.var.evt.prio == static_cast<uint8_t>(0)) { // publish
                QF::PUBLISH(l_rx.var.evt.e, &QS::rxPriv_);
            }
            else if (l_rx.var.evt.prio < static_cast<uint8_t>(QF_MAX_ACTIVE))
            {
                if (!QF::active_[l_rx.var.evt.prio]->POST_X(
                                l_rx.var.evt.e,
                                static_cast<uint_fast16_t>(0), // margin
                                &QS::rxPriv_))
                {
                    // failed QACTIVE_POST() recycles the event
                    i = static_cast<uint8_t>(0x80); // failure, no recycle
                }
            }
            else if (l_rx.var.evt.prio == static_cast<uint8_t>(255)) {
                // dispatch to the current SM object
                if (QS::rxPriv_.currObj[QS::SM_OBJ]
                    != static_cast<void *>(0))
                {
                    // increment the ref-ctr to simulate the situation
                    // when the event is just retreived from a queue.
                    // This is expected for the following QF::gc() call.
                    //
                    QF_EVT_REF_CTR_INC_(l_rx.var.evt.e);

                    static_cast<QHsm *>(QS::rxPriv_.currObj[QS::SM_OBJ])
                            ->dispatch(l_rx.var.evt.e);
                    i = static_cast<uint8_t>(0x01);  // success, recycle
                }
                else {
                    i = static_cast<uint8_t>(0x81);  // failure, recycle
                }
            }
            else if (l_rx.var.evt.prio == static_cast<uint8_t>(254)) {
                // init the current SM object"
                if (QS::rxPriv_.currObj[QS::SM_OBJ] != static_cast<void *>(0))
                {
                    // increment the ref-ctr to simulate the situation
                    // when the event is just retreived from a queue.
                    // This is expected for the following QF::gc() call.
                    //
                    QF_EVT_REF_CTR_INC_(l_rx.var.evt.e);

                    static_cast<QHsm *>(QS::rxPriv_.currObj[QS::SM_OBJ])
                            ->init(l_rx.var.evt.e);
                    i = static_cast<uint8_t>(0x01);  // success, recycle
                }
                else {
                    i = static_cast<uint8_t>(0x81);  // failure, recycle
                }
            }
            else if (l_rx.var.evt.prio == static_cast<uint8_t>(253)) {
                // post to the current AO
                if (QS::rxPriv_.currObj[QS::AO_OBJ] != static_cast<void *>(0))
                {
                    if (!static_cast<QActive *>(
                            QS::rxPriv_.currObj[QS::AO_OBJ])->POST_X(
                                l_rx.var.evt.e,
                                static_cast<uint_fast16_t>(0), // margin
                                &QS::rxPriv_))
                    {
                        // failed QACTIVE_POST() recycles the event
                        i = static_cast<uint8_t>(0x80); // failure, no recycle
                    }
                }
                else {
                    i = static_cast<uint8_t>(0x81);  // failure, recycle
                }
            }
            else {
                i = static_cast<uint8_t>(0x81);  // failure, recycle
            }

            // recycle needed?
            if ((i & static_cast<uint8_t>(0x01)) != static_cast<uint8_t>(0)) {
                QF::gc(l_rx.var.evt.e);
            }
            // failure?
            if ((i & static_cast<uint8_t>(0x80)) != static_cast<uint8_t>(0)) {
                rxReportError_(static_cast<uint8_t>(QS_RX_EVENT));
            }
            else {
#ifdef Q_UTEST
                QS::processTestEvts_(); // process all events produced
#endif
                rxReportDone_(QS_RX_EVENT);
            }
            break;
        }

#ifdef Q_UTEST
        case WAIT4_TEST_SETUP_FRAME: {
            rxReportAck_(QS_RX_TEST_SETUP);
            l_testData.tpNum = static_cast<uint8_t>(0); // clear Test-Probes
            l_testData.testTime = static_cast<QSTimeCtr>(0); //clear time tick
            // don't clear current objects
            QS::onTestSetup(); // application-specific test setup
            // no need to report Done
            break;
        }
        case WAIT4_TEST_TEARDOWN_FRAME: {
            rxReportAck_(QS_RX_TEST_TEARDOWN);
            QS::onTestTeardown(); // application-specific test teardown
            // no need to report Done
            break;
        }
        case WAIT4_TEST_CONTINUE_FRAME: {
            rxReportAck_(QS_RX_TEST_CONTINUE);
            QS::rxPriv_.inTestLoop = false; // exit the QUTest loop
            // no need to report Done
            break;
        }
        case WAIT4_TEST_PROBE_FRAME: {
            rxReportAck_(QS_RX_TEST_PROBE);
            Q_ASSERT_ID(815, l_testData.tpNum
                             < static_cast<uint8_t>(sizeof(l_testData.tpBuf)
                                               /sizeof(l_testData.tpBuf[0])));
            l_testData.tpBuf[l_testData.tpNum] = l_rx.var.tp;
            ++l_testData.tpNum;
            // no need to report Done
            break;
        }
#endif // Q_UTEST

        case ERROR_STATE: {
            // keep ignoring all bytes until new frame
            break;
        }
        default: {
            rxReportError_(static_cast<uint8_t>(0x47));
            break;
        }
    }
}

//****************************************************************************
static void rxHandleBadFrame_(uint8_t state) {
    rxReportError_(static_cast<uint8_t>(0x50)); // error for all bad frames
    switch (state) {
        case WAIT4_EVT_FRAME: {
            Q_ASSERT_ID(910, l_rx.var.evt.e != (QEvt *)0);
            QF::gc(l_rx.var.evt.e); // don't leak an allocated event
            break;
        }
        default: {
            break;
        }
    }
}

/****************************************************************************/
static void rxReportAck_(enum QSpyRxRecords recId) {
    QS::beginRec(static_cast<uint_fast8_t>(QS_RX_STATUS));
        QS_U8_(recId); // record ID
    QS::endRec();
    QS_REC_DONE();
}

//****************************************************************************
static void rxReportError_(uint8_t const code) {
    QS::beginRec(static_cast<uint_fast8_t>(QS_RX_STATUS));
        QS_U8_(static_cast<uint8_t>(static_cast<uint8_t>(0x80) | code));
    QS::endRec();
    QS_REC_DONE();
}

/****************************************************************************/
static void rxReportDone_(enum QSpyRxRecords recId) {
    QS::beginRec(static_cast<uint_fast8_t>(QS_TARGET_DONE));
        QS_TIME_();    // timestamp
        QS_U8_(recId); // record ID
    QS::endRec();
    QS_REC_DONE();
}

/****************************************************************************/
static void rxPoke_(void) {
    uint8_t *ptr = ((uint8_t *)QS::rxPriv_.currObj[QS::AP_OBJ]
                    + l_rx.var.poke.offs);
    switch (l_rx.var.poke.size) {
        case 1:
            *ptr = static_cast<uint8_t>(l_rx.var.poke.data);
            break;
        case 2:
            *(uint16_t *)ptr = static_cast<uint16_t>(l_rx.var.poke.data);
            break;
        case 4:
            *(uint32_t *)ptr = l_rx.var.poke.data;
            break;
        default:
            Q_ERROR_ID(900);
            break;
    }

    l_rx.var.poke.data = static_cast<uint32_t>(0);
    l_rx.var.poke.idx  = static_cast<uint8_t>(0);
    l_rx.var.poke.offs += static_cast<uint16_t>(l_rx.var.poke.size);
}

//============================================================================
#ifdef Q_UTEST

//****************************************************************************
/// @description
/// This function obtains the Test-Probe for a given API.
///
/// @param[in]  api_id  the API-ID that requests its Test-Probe
///
/// @returns Test-Probe data that has been received for the given API
/// from the Host (running qutest). For any ginve API, the function returns
/// the Test-Probe data in the same order as it was received from the Host.
/// If there is no Test-Probe for a ginve API, or no more Test-Probes for
/// a given API, the function returns zero.
///
uint32_t QS::getTestProbe_(void (* const api)(void)) {
    uint32_t data = static_cast<uint32_t>(0);
    uint_fast8_t i;
    for (i = static_cast<uint_fast8_t>(0);
         i < static_cast<uint_fast8_t>(l_testData.tpNum);
         ++i)
    {
        if (l_testData.tpBuf[i].addr == (QSFun)api) {
            QS_CRIT_STAT_

            data = l_testData.tpBuf[i].data;
            --l_testData.tpNum;
            // move all remaining entries in the buffer up by one
            for (; i < static_cast<uint_fast8_t>(l_testData.tpNum); ++i) {
                l_testData.tpBuf[i] =
                    l_testData.tpBuf[i + static_cast<uint_fast8_t>(1)];
            }
            // i == l_testData.tpNum, which terminates the top loop
            QS::beginRec(static_cast<uint_fast8_t>(QS_TEST_PROBE_GET));
                QS_TIME_();    // timestamp
                QS_FUN_(api);  // the calling API
                QS_U32_(data); // the Test-Probe data
            QS::endRec();
            QS_REC_DONE();
        }
    }
    return data;
}

/****************************************************************************/
QSTimeCtr QS::onGetTime(void) {
    return (++l_testData.testTime);
}

#endif // Q_UTEST

} // namespace QP

