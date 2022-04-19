//============================================================================
// Product: DPP example, QUTEST
// Last updated for version 6.9.1
// Last updated on  2020-09-21
//
//                    Q u a n t u m  L e a P s
//                    ------------------------
//                    Modern Embedded Software
//
// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
// along with this program. If not, see <www.gnu.org/licenses>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_MODULE("bsp")

// namespace DPP *************************************************************
namespace DPP {

static uint32_t l_rnd; // random seed

enum {
    BSP_CALL = QP::QS_USER,
};

// BSP functions =============================================================
void BSP::init(int argc, char **argv) {
    Q_ALLEGE(QS_INIT(argc <= 1 ? nullptr : argv[1]));

    QS_FUN_DICTIONARY(&BSP::displayPaused);
    QS_FUN_DICTIONARY(&BSP::displayPhilStat);
    QS_FUN_DICTIONARY(&BSP::random);
    QS_FUN_DICTIONARY(&BSP::randomSeed);
    QS_FUN_DICTIONARY(&QP::QHsm::top);

    QS_USR_DICTIONARY(BSP_CALL);

    BSP::randomSeed(1234U);
}
//............................................................................
void BSP::displayPaused(uint8_t paused) {
    QS_TEST_PROBE_DEF(&BSP::displayPaused)

    QS_TEST_PROBE(
        Q_ERROR_ID(100);
    )
    QS_BEGIN_ID(BSP_CALL, 0U) // application-specific record
        QS_FUN(&BSP::displayPaused);
        QS_U8(0, paused);
    QS_END()
}
//............................................................................
void BSP::displayPhilStat(uint8_t n, char const *stat) {
    QS_BEGIN_ID(BSP_CALL, 0U) // application-specific record
        QS_FUN(&BSP::displayPhilStat);
        QS_U8(0, n);
        QS_STR(stat);
    QS_END()
}
//............................................................................
uint32_t BSP::random(void) {
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    uint32_t rnd = l_rnd * (3U*7U*11U*13U*23U);
    l_rnd = rnd; // set for the next time
    rnd >>= 8;

    QS_TEST_PROBE_DEF(&BSP::random)
    QS_TEST_PROBE(
        rnd = qs_tp_;
    )

    QS_BEGIN_ID(BSP_CALL, 0U) // application-specific record
        QS_FUN(&BSP::random);
        QS_U32(0, rnd);
    QS_END()
    return rnd;
}
//............................................................................
void BSP::randomSeed(uint32_t seed) {
    QS_TEST_PROBE_DEF(&BSP::randomSeed)

    QS_TEST_PROBE(
        seed = qs_tp_;
    )
    l_rnd = seed;
    QS_BEGIN_ID(BSP_CALL, 0U) // application-specific record
        QS_FUN(&BSP::randomSeed);
        QS_U32(0, seed);
    QS_END()
}

//............................................................................
void BSP::terminate(int16_t result) {
    (void)result;
}

} // namespace DPP
