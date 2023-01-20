//============================================================================
// Product: BSP for DPP example (console)
// Last updated for version 7.3.0
// Last updated on  2023-08-24
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"        // QP/C++ real-time embedded framework
#include "dpp.hpp"          // DPP Application interface
#include "bsp.hpp"          // Board Support Package

//============================================================================
namespace { // unnamed namespace for local stuff with internal linkage

Q_DEFINE_THIS_MODULE("bsp")

// Local objects -------------------------------------------------------------
static std::uint32_t l_rnd; // random seed

#ifdef Q_SPY

    enum AppRecords { // application-specific trace records
        BSP_CALL = QP::QS_USER,
    };

#endif

} // unnamed namespace


//============================================================================
namespace BSP {

void init() {
    // initialize the QS software tracing
    if (!QS_INIT(nullptr)) {
        Q_ERROR();
    }

    QS_FUN_DICTIONARY(&BSP::displayPaused);
    QS_FUN_DICTIONARY(&BSP::displayPhilStat);
    QS_FUN_DICTIONARY(&BSP::random);
    QS_FUN_DICTIONARY(&BSP::randomSeed);

    QS_ONLY(APP::produce_sig_dict());

    QS_USR_DICTIONARY(BSP_CALL);

    BSP::randomSeed(1234U);
}
//............................................................................
void displayPhilStat(std::uint8_t n, char const *stat) {
    QS_BEGIN_ID(BSP_CALL, 0U) // app-specific record
        QS_FUN(&BSP::displayPhilStat);
        QS_U8(0, n);
        QS_STR(stat);
    QS_END()
}
//............................................................................
void displayPaused(std::uint8_t const paused) {
    QS_TEST_PROBE_DEF(&BSP::displayPaused)

    QS_TEST_PROBE(
        Q_ASSERT_ID(100, 0);
    )
    QS_BEGIN_ID(BSP_CALL, 0U) // app-specific record
        QS_FUN(&BSP::displayPaused);
        QS_U8(0, paused);
    QS_END()
}
//............................................................................
std::uint32_t random() {
    QS_TEST_PROBE_DEF(&BSP::random)
    uint32_t rnd = 123U;

    QS_TEST_PROBE(
        rnd = qs_tp_;
    )
    QS_BEGIN_ID(BSP_CALL, 0U) // app-specific record
        QS_FUN(&BSP::random);
        QS_U32(0, rnd);
    QS_END()
    return rnd;
}
//............................................................................
void randomSeed(std::uint32_t seed) {
    QS_TEST_PROBE_DEF(&BSP::randomSeed)

    QS_TEST_PROBE(
        seed = qs_tp_;
    )
    l_rnd = seed;
    QS_BEGIN_ID(BSP_CALL, 0U) // app-specific record
        QS_FUN(&BSP::randomSeed);
        QS_U32(0, seed);
    QS_END()
}

} // namespace BSP

