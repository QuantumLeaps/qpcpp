#include "et.h"             // ET: embedded test

// includes for the CUT...
#include "qp_port.hpp"      // QP/C++ port from the port directory
#include "qsafe.h"          // QP Functional Safety (FuSa) Subsystem
#ifdef Q_SPY /* software tracing enabled? */
#include "qs_port.hpp"      // QS port from the port directory
#else
#include "qs_dummy.hpp"     // QS dummy (inactive) interface
#endif

extern "C" {

enum { RX_SIZE = 8 };

static std::uint8_t qsBuf[100];       // buffer for QS-TX channel
static std::uint8_t qsRxBuf[RX_SIZE]; // buffer for QS-RX channel

//............................................................................
void setup(void) {
}
//............................................................................
void teardown(void) {
}
//............................................................................
TEST_GROUP("QS/RX") {

QP::QS::initBuf(qsBuf, sizeof(qsBuf));

TEST("QS-RX initialization") {
    QP::QS::rxInitBuf(qsRxBuf, RX_SIZE);
    VERIFY(RX_SIZE - 1 == QP::QS::rxGetNfree());
}

TEST("QS-RX putting 3") {
    VERIFY(QP::QS::rxPut(0x1U));
    VERIFY(QP::QS::rxPut(0x2U));
    VERIFY(QP::QS::rxPut(0x3U));
    VERIFY(RX_SIZE - 1 - 3 == QP::QS::rxGetNfree());
}

TEST("QS-RX QS::rxParse") {
    QP::QS::rxParse();
    VERIFY(RX_SIZE - 1 == QP::QS::rxGetNfree());
}

TEST("QS-RX putting 6") {
    VERIFY(QP::QS::rxPut(0x1U));
    VERIFY(QP::QS::rxPut(0x2U));
    VERIFY(QP::QS::rxPut(0x3U));
    VERIFY(QP::QS::rxPut(0x4U));
    VERIFY(QP::QS::rxPut(0x5U));
    VERIFY(QP::QS::rxPut(0x6U));
    VERIFY(RX_SIZE - 1 - 6 == QP::QS::rxGetNfree());
}

TEST("QS-RX putting 3 more") {
    VERIFY(QP::QS::rxPut(0x7U));
    VERIFY(false == QP::QS::rxPut(0x8U));
    VERIFY(false == QP::QS::rxPut(0x9U));
    VERIFY(0 == QP::QS::rxGetNfree());
}

} // TEST_GROUP()

} // extern "C"

//============================================================================
// dependencies for the CUT ...

//............................................................................
extern "C" Q_NORETURN Q_onError(char const * const module, int_t const loc) {
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(loc);
    ET_onExit(-1);
    for (;;) { // make explicitly noreturn
    }
}

namespace QP {

//............................................................................
std::uint_fast16_t QF::poolGetMaxBlockSize(void) noexcept {
    return 0U;
}
//............................................................................
void QTimeEvt::tick(
    std::uint_fast8_t const tickRate,
    void const * const sender) noexcept
{
    Q_UNUSED_PAR(tickRate);
    Q_UNUSED_PAR(sender);
}
//............................................................................
void QActive::publish_(
    QEvt const * const e,
    void const * const sender,
    std::uint_fast8_t const qs_id) noexcept
{
    Q_UNUSED_PAR(e);
    Q_UNUSED_PAR(sender);
    Q_UNUSED_PAR(qs_id);
}
//............................................................................
QEvt *QF::newX_(std::uint_fast16_t const evtSize,
                std::uint_fast16_t const margin, enum_t const sig) noexcept
{
    Q_UNUSED_PAR(evtSize);
    Q_UNUSED_PAR(margin);
    Q_UNUSED_PAR(sig);
    return nullptr;
}
//............................................................................
void QF::gc(QEvt const * const e) noexcept {
    Q_UNUSED_PAR(e);
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY

void QS::onCleanup(void) {
}
//............................................................................
void QS::onReset(void) {
}
//............................................................................
void QS::onFlush(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    return (QSTimeCtr)0U;
}
//............................................................................
void QS::onCommand(uint8_t cmdId, uint32_t param1,
    uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
}
#endif // Q_SPY

} // namespace QP
