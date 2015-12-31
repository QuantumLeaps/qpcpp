/// @file
/// @brief QS receive channel services
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 5.6.0
/// Last updated on  2015-12-26
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps. All rights reserved.
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
/// http://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#define QP_IMPL           // this is QP implementation
#include "qs_port.h"      // QS port
#include "qs_pkg.h"       // QS package-scope internal interface
#include "qassert.h"      // QP assertions

namespace QP {

Q_DEFINE_THIS_MODULE("qs_rx")

//****************************************************************************
QS::QSrxPriv QS::rxPriv_; // QS-RX private data

//****************************************************************************
#if (QS_OBJ_PTR_SIZE == 1)
    typedef uint8_t QSAddr;
#elif (QS_OBJ_PTR_SIZE == 2)
    typedef uint16_t QSAddr;
#elif (QS_OBJ_PTR_SIZE == 4)
    typedef uint32_t QSAddr;
#elif (QS_OBJ_PTR_SIZE == 8)
    typedef uint64_t QSAddr;
#endif

// extended-state variables used for parsing various QS-RX Records...
struct CmdVar {
    uint32_t param;
    uint8_t  idx;
    uint8_t  cmdId;
} ;

struct TickVar {
    uint8_t rate;
};

struct PeekVar {
    QSAddr  addr;
    uint8_t idx;
    uint8_t len;
};

struct PokeVar {
    uint8_t data[8];
    QSAddr  addr;
    uint8_t idx;
    uint8_t len;
};

struct GFltVar {
    uint8_t data[16];
    uint8_t idx;
};

struct LFltVar {
    QSAddr addr;
    uint8_t idx;
    uint8_t fltId;
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
        CmdVar  cmd;
        TickVar tick;
        PeekVar peek;
        PokeVar poke;
        GFltVar gFlt;
        LFltVar lFlt;
        AFltVar aFlt;
        EvtVar  evt;
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
    WAIT4_CMD_PARAM,
    WAIT4_CMD_FRAME,
    WAIT4_RESET_FRAME,
    WAIT4_TICK_RATE,
    WAIT4_TICK_FRAME,
    WAIT4_PEEK_ADDR,
    WAIT4_PEEK_LEN,
    WAIT4_PEEK_FRAME,
    WAIT4_POKE_ADDR,
    WAIT4_POKE_LEN,
    WAIT4_POKE_DATA,
    WAIT4_POKE_FRAME,
    WAIT4_GLB_FILTER_LEN,
    WAIT4_GLB_FILTER_DATA,
    WAIT4_GLB_FILTER_FRAME,
    WAIT4_LOC_FILTER_ID,
    WAIT4_LOC_FILTER_ADDR,
    WAIT4_LOC_FILTER_FRAME,
    WAIT4_AO_FILTER_PRIO,
    WAIT4_AO_FILTER_FRAME,
    WAIT4_EVT_PRIO,
    WAIT4_EVT_SIG,
    WAIT4_EVT_LEN,
    WAIT4_EVT_PAR,
    WAIT4_EVT_FRAME,
    ERROR_STATE
};

// Internal helper functions...
static void rxParseData_(uint8_t const b);
static void rxHandleGoodFrame_(void);
static void rxHandleBadFrame_(void);
static void rxReportSuccess_(enum QSpyRxRecords const recId);
static void rxReportError_(uint8_t const stateId);
static bool rxAddr_(uint8_t const b, QSAddr *const addr, uint8_t *const idx);

//! Internal QS-RX function to take a transition in the QS-RX FSM
static inline void tran_(RxStateEnum const target) {
    l_rx.state = static_cast<uint8_t>(target);
}

// QS-RX identifier for output of trace records
static uint8_t const l_QS_RX = static_cast<uint8_t>(0);

//****************************************************************************
/// @description
/// This function should be called from QS::onStartup() to provide QS-RX with
/// the receive data buffer. The first parameter @p sto[] is the address of
/// the memory block, and the second parameter @p stoSize is the size of
/// this block in bytes. The size of the QS RX buffer cannot exceed 64KB.
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
    static char_t const s_rxObjDic[] = "QS_RX";
    QS_CRIT_STAT_

    rxPriv_.buf   = &sto[0];
    rxPriv_.end   = static_cast<QSCtr>(stoSize) - static_cast<QSCtr>(1);
    rxPriv_.head  = static_cast<QSCtr>(0);
    rxPriv_.tail  = static_cast<QSCtr>(0);

    tran_(WAIT4_SEQ);
    l_rx.esc    = static_cast<uint8_t>(0);
    l_rx.seq    = static_cast<uint8_t>(0);
    l_rx.chksum = static_cast<uint8_t>(0);

    QS_CRIT_ENTRY_();
    beginRec(static_cast<uint_fast8_t>(QS_OBJ_DICT));
    QS_OBJ_(&l_QS_RX);
    QS_STR_(&s_rxObjDic[0]);
    endRec();
    QS_CRIT_EXIT_();
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
            if (l_rx.chksum == QS_GOOD_CHKSUM) {
                rxHandleGoodFrame_();
            }
            else { // bad checksum
                rxReportError_(static_cast<uint8_t>(0x00));
                rxHandleBadFrame_();
            }

            // get ready for the next frame
            l_rx.esc    = static_cast<uint8_t>(0);
            l_rx.chksum = static_cast<uint8_t>(0);
            tran_(WAIT4_SEQ);
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
                rxReportError_(static_cast<uint8_t>(1U << 5));
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
                    l_rx.var.peek.addr = static_cast<QSAddr>(0);
                    l_rx.var.peek.idx = static_cast<uint8_t>(0);
                    tran_(WAIT4_PEEK_ADDR);
                    break;
                case QS_RX_POKE:
                    l_rx.var.poke.addr = static_cast<QSAddr>(0);
                    l_rx.var.poke.idx = static_cast<uint8_t>(0);
                    tran_(WAIT4_POKE_ADDR);
                    break;
                case QS_RX_GLB_FILTER:
                    tran_(WAIT4_GLB_FILTER_LEN);
                    break;
                case QS_RX_LOC_FILTER:
                    tran_(WAIT4_LOC_FILTER_ID);
                    break;
                case QS_RX_AO_FILTER:
                    tran_(WAIT4_AO_FILTER_PRIO);
                    break;
                case QS_RX_EVENT:
                    tran_(WAIT4_EVT_PRIO);
                    break;
                default:
                    rxReportError_(static_cast<uint8_t>(1U << 5));
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
            l_rx.var.cmd.cmdId = b;
            l_rx.var.cmd.idx   = static_cast<uint8_t>(0);
            l_rx.var.cmd.param = static_cast<uint32_t>(0);
            tran_(WAIT4_CMD_PARAM);
            break;
        }
        case WAIT4_CMD_PARAM: {
            l_rx.var.cmd.param |=
                (static_cast<uint32_t>(b) << l_rx.var.cmd.idx);
            l_rx.var.cmd.idx   += static_cast<uint8_t>(8);
            if (l_rx.var.cmd.idx == static_cast<uint8_t>(8*4)) {
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
            l_rx.var.tick.rate = b;
            tran_(WAIT4_TICK_FRAME);
            break;
        }
        case WAIT4_TICK_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_PEEK_ADDR: {
            if (rxAddr_(b, &l_rx.var.peek.addr, &l_rx.var.peek.idx)) {
                tran_(WAIT4_PEEK_LEN);
            }
            break;
        }
        case WAIT4_PEEK_LEN: {
            l_rx.var.peek.len = b;
            tran_(WAIT4_PEEK_FRAME);
            break;
        }
        case WAIT4_PEEK_FRAME: {
            // keep ignoring the data until a frame is collected
            break;
        }
        case WAIT4_POKE_ADDR: {
            if (rxAddr_(b, &l_rx.var.poke.addr, &l_rx.var.poke.idx)) {
                tran_(WAIT4_POKE_LEN);
            }
            break;
        }
        case WAIT4_POKE_LEN: {
            if (b <= static_cast<uint8_t>(sizeof(l_rx.var.poke.data))) {
                l_rx.var.poke.len = b;
                l_rx.var.poke.idx = static_cast<uint8_t>(0);
                tran_(WAIT4_POKE_DATA);
            }
            else {
                rxReportError_(static_cast<uint8_t>(1U << 5));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_POKE_DATA: {
            l_rx.var.poke.data[l_rx.var.poke.idx] = b;
            ++l_rx.var.poke.idx;
            if (l_rx.var.poke.idx == l_rx.var.poke.len) {
                tran_(WAIT4_POKE_FRAME);
            }
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
                rxReportError_(static_cast<uint8_t>(1U << 5));
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
        case WAIT4_LOC_FILTER_ID: {
            if (b < static_cast<uint8_t>(6)) {
                l_rx.var.lFlt.fltId = b;
                l_rx.var.lFlt.addr  = static_cast<QSAddr>(0);
                l_rx.var.lFlt.idx  = static_cast<uint8_t>(0);
                tran_(WAIT4_LOC_FILTER_ADDR);
            }
            else {
                rxReportError_(static_cast<uint8_t>(1U << 5));
                tran_(ERROR_STATE);
            }
            break;
        }
        case WAIT4_LOC_FILTER_ADDR: {
            if (rxAddr_(b, &l_rx.var.lFlt.addr, &l_rx.var.lFlt.idx)) {
                tran_(WAIT4_LOC_FILTER_FRAME);
            }
            break;
        }
        case WAIT4_LOC_FILTER_FRAME: {
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
            if (l_rx.var.evt.idx == static_cast<uint8_t>(8*Q_SIGNAL_SIZE)) {
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
            if (l_rx.var.evt.idx == static_cast<uint8_t>(8*2)) {
                if ((l_rx.var.evt.len + static_cast<uint16_t>(sizeof(QEvt)))
                    <= static_cast<uint16_t>(QF::poolGetMaxBlockSize()))
                {
                    l_rx.var.evt.e = QF::newX_(
                        (static_cast<uint_fast16_t>(l_rx.var.evt.len)
                         + static_cast<uint_fast16_t>(sizeof(QEvt))),
                        static_cast<uint_fast16_t>(1), // margin
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
                        rxReportError_(static_cast<uint8_t>(1U << 5));
                        tran_(ERROR_STATE);
                    }
                }
                else {
                    rxReportError_(static_cast<uint8_t>(1U << 5));
                    tran_(ERROR_STATE);
                }
            }
            break;
        }
        case WAIT4_EVT_PAR: {  // event parameters
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
        case ERROR_STATE: {
            // keep ignoring the data until a good frame is collected
            break;
        }
        default: {  // unexpected or unimplemented state
            rxReportError_(static_cast<uint8_t>(1U << 5));
            tran_(ERROR_STATE);
            break;
        }
    }
}

//****************************************************************************
static void rxHandleGoodFrame_(void) {
    QS_CRIT_STAT_
    uint8_t i;
    uint8_t *buf_;

    switch (l_rx.state) {
        case WAIT4_INFO_FRAME: {
            // no need to report success, because a QS record is produced
            QS_CRIT_ENTRY_();
            QS_target_info_(static_cast<uint8_t>(0)); // send only Target info
            QS_CRIT_EXIT_();
            break;
        }
        case WAIT4_RESET_FRAME: {
            // no need to report success, because the target resets
            QP::QS::onReset(); // reset the Target
            break;
        }
        case WAIT4_CMD_FRAME: {
            rxReportSuccess_(QS_RX_COMMAND);
            QS::onCommand(l_rx.var.cmd.cmdId, l_rx.var.cmd.param);
            break;
        }
        case WAIT4_TICK_FRAME: {
            rxReportSuccess_(QS_RX_TICK);
            QF::TICK_X(l_rx.var.tick.rate, &l_QS_RX);
            break;
        }
        case WAIT4_PEEK_FRAME: {
            // no need to report success, because a QS record is produced
            QS_BEGIN_(QS_PEEK_DATA,
                static_cast<void *>(0), static_cast<void *>(0))
                QS_TIME_(); // timestamp
                // address...
#if (QS_OBJ_PTR_SIZE == 1)
                QS::u8_(l_rx.var.peek.addr);
#elif (QS_OBJ_PTR_SIZE == 2)
                QS::u16_(l_rx.var.peek.addr);
#elif (QS_OBJ_PTR_SIZE == 4)
                QS::u32_(l_rx.var.peek.addr);
#elif (QS_OBJ_PTR_SIZE == 8)
                QS::u64_(l_rx.var.peek.addr);
#endif
                QS_U8_(l_rx.var.peek.len);   // data length
                buf_ = reinterpret_cast<uint8_t *>(l_rx.var.peek.addr);
                for (i = static_cast<uint8_t>(0);
                     i < l_rx.var.peek.len;
                     ++i)
                {
                    QS_U8_(QS_PTR_AT_(buf_, i));  // data bytes
                }
            QS_END_()
            break;
        }
        case WAIT4_POKE_FRAME: {
            rxReportSuccess_(QS_RX_POKE);
            buf_ = reinterpret_cast<uint8_t *>(l_rx.var.poke.addr);
            QS_CRIT_ENTRY_(); // poke the data within a critical section
            for (i = static_cast<uint8_t>(0); i < l_rx.var.poke.len; ++i) {
                QS_PTR_AT_(buf_, i) = l_rx.var.poke.data[i];
            }
            QS_CRIT_EXIT_();
            break;
        }
        case WAIT4_GLB_FILTER_FRAME: {
            rxReportSuccess_(QS_RX_GLB_FILTER);

            // never disable the non-maskable records
            l_rx.var.gFlt.data[0] |= static_cast<uint8_t>(0x01);
            l_rx.var.gFlt.data[7] |= static_cast<uint8_t>(0xF0);
            l_rx.var.gFlt.data[8] |= static_cast<uint8_t>(0x3F);

            // never turn the last 3 records on (0x7D, 0x7E, 0x7F)
            l_rx.var.gFlt.data[15] &= static_cast<uint8_t>(0xE0);

            for (i = static_cast<uint8_t>(0);
                 i < static_cast<uint8_t>(sizeof(QS::priv_.glbFilter));
                 ++i)
            {
                QS::priv_.glbFilter[i] = l_rx.var.gFlt.data[i];
            }
            break;
        }
        case WAIT4_LOC_FILTER_FRAME: {
            i = static_cast<uint8_t>(1);
            switch (l_rx.var.lFlt.fltId) {
                case 0:
                    QS::priv_.smObjFilter =
                        reinterpret_cast<void *>(l_rx.var.lFlt.addr);
                   break;
                case 1:
                    QS::priv_.aoObjFilter =
                        reinterpret_cast<void *>(l_rx.var.lFlt.addr);
                    break;
                case 2:
                    QS::priv_.mpObjFilter =
                        reinterpret_cast<void *>(l_rx.var.lFlt.addr);
                    break;
                case 3:
                    QS::priv_.eqObjFilter =
                        reinterpret_cast<void *>(l_rx.var.lFlt.addr);
                    break;
                case 4:
                    QS::priv_.teObjFilter =
                        reinterpret_cast<void *>(l_rx.var.lFlt.addr);
                    break;
                case 5:
                    QS::priv_.apObjFilter =
                        reinterpret_cast<void *>(l_rx.var.lFlt.addr);
                    break;
                default:
                    rxReportError_(static_cast<uint8_t>(2U << 5));
                    i = static_cast<uint8_t>(0);
                    break;
            }
            if (i != static_cast<uint8_t>(0)) {
                rxReportSuccess_(QS_RX_LOC_FILTER);
            }
            break;
        }
        case WAIT4_AO_FILTER_FRAME: {
            if (l_rx.var.aFlt.prio <= static_cast<uint8_t>(QF_MAX_ACTIVE)) {
                rxReportSuccess_(QS_RX_AO_FILTER);
                QS::priv_.aoObjFilter = QF::active_[l_rx.var.aFlt.prio];
                QS::priv_.smObjFilter = QF::active_[l_rx.var.aFlt.prio];
            }
            else {
                rxReportError_(static_cast<uint8_t>(2U << 5));
            }
            break;
        }
        case WAIT4_EVT_FRAME: {
            if (l_rx.var.evt.prio == static_cast<uint8_t>(0)) {
                rxReportSuccess_(QS_RX_EVENT);
                QF::PUBLISH(l_rx.var.evt.e, &l_QS_RX);
            }
            else if (l_rx.var.evt.prio < static_cast<uint8_t>(QF_MAX_ACTIVE)){
                rxReportSuccess_(QS_RX_EVENT);
                (void)QF::active_[l_rx.var.evt.prio]->POST_X(
                               l_rx.var.evt.e,
                               static_cast<uint_fast16_t>(1), // margin
                               &l_QS_RX);
            }
            else {
                rxReportError_(static_cast<uint8_t>(2U << 5));
                QF::gc(l_rx.var.evt.e);
            }
            break;
        }
        case ERROR_STATE: {
            // keep ignoring all bytes until new frame
            break;
        }
        default: {
            rxReportError_(static_cast<uint8_t>(2U << 5));
            break;
        }
    }
}

//****************************************************************************
static void rxHandleBadFrame_(void) {
    switch (l_rx.state) {
        case WAIT4_EVT_FRAME: {
            Q_ASSERT_ID(910, l_rx.var.evt.e != (QEvt *)0);
            QF::gc(l_rx.var.evt.e);
            break;
        }
        default: {
            break;
        }
    }
}

//****************************************************************************
static void rxReportSuccess_(enum QSpyRxRecords const recId) {
    QS_CRIT_STAT_
    QS_BEGIN_(QS_RX_STATUS, static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_();    // timestamp
        QS_U8_(recId); // record ID
    QS_END_()
}

//****************************************************************************
static void rxReportError_(uint8_t const stateId) {
    QS_CRIT_STAT_
    QS_BEGIN_(QS_RX_STATUS, static_cast<void *>(0), static_cast<void *>(0))
        QS_TIME_();    // timestamp
        QS_U8_(static_cast<uint8_t>(0x80) | stateId | l_rx.state); // error no
    QS_END_()
}

//****************************************************************************
static bool rxAddr_(uint8_t const b, QSAddr *const addr, uint8_t *const idx) {
    *addr |= (static_cast<uint32_t>(b) << *idx);
    *idx += static_cast<uint8_t>(8);
    return (*idx == static_cast<uint8_t>(8*QS_OBJ_PTR_SIZE)) ? true : false;
}

} // namespace QP
