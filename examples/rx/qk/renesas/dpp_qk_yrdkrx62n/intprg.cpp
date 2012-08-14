//////////////////////////////////////////////////////////////////////////////
// Product: BSP for YRDKRX62N board, QK, Renesas RX Standard Toolchain
// Last Updated for Version: 4.3.00
// Date of the Last Update:  Nov 28, 2011
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2011 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "bsp.h"
#include "dpp.h"

Q_DEFINE_THIS_FILE

#include <machine.h>
#include "vect.h"
#pragma section IntPRG

extern "C" {

void Dummy(void)                  { Q_ERROR(); }
void Excep_SuperVisorInst(void)   { Q_ERROR(); }
void Excep_UndefinedInst(void)    { Q_ERROR(); }
void Excep_FloatingPoint(void)    { Q_ERROR(); }
void NonMaskableInterrupt(void)   { Q_ERROR(); }

// vect=0 ....................................................................
void Excep_BRK(void)              { Q_ERROR(); }
// vector  1 reserved
// vector  2 reserved
// vector  3 reserved
// vector  4 reserved
// vector  5 reserved
// vector  6 reserved
// vector  7 reserved
// vector  8 reserved
// vector  9 reserved
// vector 10 reserved
// vector 11 reserved
// vector 12 reserved
// vector 13 reserved
// vector 14 reserved
// vector 15 reserved
// vector 16 .................................................................
void Excep_BUSERR(void)           { Q_ERROR(); }
// vector 17 reserved
// vector 18 reserved
// vector 19 reserved
// vector 20 reserved
// vector 21 .................................................................
void Excep_FCU_FCUERR(void)       { Q_ERROR(); }
// vector 22 reserved
// vector 23 .................................................................
void Excep_FCU_FRDYI(void)        { Q_ERROR(); }
// vector 24 reserved
// vector 25 reserved
// vector 26 reserved
// vector 27 reserved
// vector 28 .................................................................
void Excep_CMTU0_CMT0(void) {
    QK_ISR_ENTRY();             // inform the QK kernel about entering the ISR

#ifdef Q_SPY
    QS_tickTime_ += QS_tickPeriod_;          // account for the clock rollover
#endif
    QF::TICK(&Excep_CMTU0_CMT0);              // process all armed time events

    QK_ISR_EXIT();               // inform the QK kernel about exiting the ISR
}
// vector 29 .................................................................
//void Excep_CMTU0_CMT1(void) { Q_ERROR(); }
// vector 30 .................................................................
//void Excep_CMTU1_CMT2(void) { Q_ERROR(); }
// vector 31 .................................................................
//void Excep_CMTU1_CMT3(void) { Q_ERROR(); }
// vector 32 reserved
// vector 33 reserved
// vector 34 reserved
// vector 35 reserved
// vector 36 reserved
// vector 37 reserved
// vector 38 reserved
// vector 39 reserved
// vector 40 reserved
// vector 41 reserved
// vector 42 reserved
// vector 43 reserved
// vector 44 reserved
// vector 45 reserved
// vector 46 reserved
// vector 47 reserved
// vector 48 reserved
// vector 49 reserved
// vector 50 reserved
// vector 51 reserved
// vector 52 reserved
// vector 53 reserved
// vector 54 reserved
// vector 55 reserved
// vector 56 reserved
// vector 57 reserved
// vector 58 reserved
// vector 59 reserved
// vector 60 reserved
// vector 61 reserved
// vector 62 reserved
// vector 63 reserved
// vector 64 .................................................................
//void Excep_IRQ0(void) { Q_ERROR(); }
// vector 65 .................................................................
//void Excep_IRQ1(void) { Q_ERROR(); }
// vector 66 .................................................................
//void Excep_IRQ2(void) { Q_ERROR(); }
// vector 67 .................................................................
//void Excep_IRQ3(void) { Q_ERROR(); }
// vector 68 .................................................................
//void Excep_IRQ4(void) { Q_ERROR(); }
// vector 69 .................................................................
//void Excep_IRQ5(void) { Q_ERROR(); }
// vector 70 .................................................................
//void Excep_IRQ6(void) { Q_ERROR(); }
// vector 71 .................................................................
//void Excep_IRQ7(void) { Q_ERROR(); }
// vector 72 .................................................................
void Excep_IRQ8(void) {
    QK_ISR_ENTRY();             // inform the QK kernel about entering the ISR

    AO_Philo[0]->POST(Q_NEW(QEvent, MAX_PUB_SIG),            // for testing...
                      &QS_Excep_IRQ8);
    QK_ISR_EXIT();               // inform the QK kernel about exiting the ISR
}
// vector 73 .................................................................
void Excep_IRQ9(void) {
    QK_ISR_ENTRY();             // inform the QK kernel about entering the ISR

    AO_Philo[1]->POST(Q_NEW(QEvent, MAX_PUB_SIG),            // for testing...
                      &QS_Excep_IRQ9);
    QK_ISR_EXIT();               // inform the QK kernel about exiting the ISR
}
// vector 74 .................................................................
void Excep_IRQ10(void) {
    QK_ISR_ENTRY();             // inform the QK kernel about entering the ISR

    AO_Table->POST(Q_NEW(QEvent, MAX_PUB_SIG),               // for testing...
                   &QS_Excep_IRQ10);
    QK_ISR_EXIT();               // inform the QK kernel about exiting the ISR
}
// vector 75 .................................................................
//void Excep_IRQ11(void) { Q_ERROR(); }
// vector 76 .................................................................
//void Excep_IRQ12(void) { Q_ERROR(); }
// vector 77 .................................................................
//void Excep_IRQ13(void) { Q_ERROR(); }
// vector 78 .................................................................
//void Excep_IRQ14(void) { Q_ERROR(); }
// vector 79 .................................................................
//void Excep_IRQ15(void) { Q_ERROR(); }
// vector 80 reserved
// vector 81 reserved
// vector 82 reserved
// vector 83 reserved
// vector 84 reserved
// vector 85 reserved
// vector 86 reserved
// vector 87 reserved
// vector 88 reserved
// vector 89 reserved
// vector 90 reserved
// vector 91 reserved
// vector 92 reserved
// vector 93 reserved
// vector 94 reserved
// vector 95 reserved
// vector 96 .................................................................
//void Excep_WDT_WOVI(void) { Q_ERROR(); }
// vector 97 reserved
// vector 98 .................................................................
//void Excep_AD0_ADI0(void) { Q_ERROR(); }
// vector 99 .................................................................
//void Excep_AD1_ADI1(void) { Q_ERROR(); }
// vector 100 ................................................................
//void Excep_AD2_ADI2(void) { Q_ERROR(); }
// vector 101 ................................................................
//void Excep_AD3_ADI3(void) { Q_ERROR(); }
// vector 102 reserved
// vector 103 reserved
// vector 104 ................................................................
//void Excep_TPU0_TGI0A(void) { Q_ERROR(); }
// vector 105 ................................................................
//void Excep_TPU0_TGI0B(void) { Q_ERROR(); }
// vector 106 ................................................................
//void Excep_TPU0_TGI0C(void) { Q_ERROR(); }
// vector 107 ................................................................
//void Excep_TPU0_TGI0D(void) { Q_ERROR(); }
// vector 108 ................................................................
//void Excep_TPU0_TCI0V(void) { Q_ERROR(); }
// vector 109 reserved
// vector 110 reserved
// vector 111 ................................................................
//void Excep_TPU1_TGI1A(void) { Q_ERROR(); }
// vector 112 ................................................................
//void Excep_TPU1_TGI1B(void) { Q_ERROR(); }
// vector 113 reserved
// vector 114 reserved
// vector 115 ................................................................
//void Excep_TPU1_TCI1V(void) { Q_ERROR(); }
// vector 116 ................................................................
//void Excep_TPU1_TCI1U(void) { Q_ERROR(); }
// vector 117 ................................................................
//void Excep_TPU2_TGI2A(void) { Q_ERROR(); }
// vector 118 ................................................................
//void Excep_TPU2_TGI2B(void) { Q_ERROR(); }
// vector 119 reserved
// vector 120 ................................................................
//void Excep_TPU2_TCI2V(void) { Q_ERROR(); }
// vector 121 ................................................................
//void Excep_TPU2_TCI2U(void) { Q_ERROR(); }
// vector 122 ................................................................
//void Excep_TPU3_TGI3A(void) { Q_ERROR(); }
// vector 123 ................................................................
//void Excep_TPU3_TGI3B(void) { Q_ERROR(); }
// vector 124 ................................................................
//void Excep_TPU3_TGI3C(void) { Q_ERROR(); }
// vector 125 ................................................................
//void Excep_TPU3_TGI3D(void) { Q_ERROR(); }
// vector 126 ................................................................
//void Excep_TPU3_TCI3V(void) { Q_ERROR(); }
// vector 127 ................................................................
//void Excep_TPU4_TGI4A(void) { Q_ERROR(); }
// vector 128 ................................................................
//void Excep_TPU4_TGI4B(void) { Q_ERROR(); }
// vector 129 reserved
// vector 130 reserved
// vector 131 ................................................................
//void Excep_TPU4_TCI4V(void) { Q_ERROR(); }
// vector 132 ................................................................
//void Excep_TPU4_TCI4U(void) { Q_ERROR(); }
// vector 133 ................................................................
//void Excep_TPU5_TGI5A(void) { Q_ERROR(); }
// vector 134 ................................................................
//void Excep_TPU5_TGI5B(void) { Q_ERROR(); }
// vector 135 reserved
// vector 136 ................................................................
//void Excep_TPU5_TCI5V(void) { Q_ERROR(); }
// vector 137 ................................................................
//void Excep_TPU5_TCI5U(void) { Q_ERROR(); }
// vector 138 ................................................................
//void Excep_TPU6_TGI6A(void) { Q_ERROR(); }
// vector 139 ................................................................
//void Excep_TPU6_TGI6B(void) { Q_ERROR(); }
// vector 140 ................................................................
//void Excep_TPU6_TGI6C(void) { Q_ERROR(); }
// vector 141 ................................................................
//void Excep_TPU6_TGI6D(void) { Q_ERROR(); }
// vector 142 ................................................................
//void Excep_TPU6_TCI6V(void) { Q_ERROR(); }
// vector 143 reserved
// vector 144 reserved
// vector 145 ................................................................
//void Excep_TPU7_TGI7A(void) { Q_ERROR(); }
// vector 146 ................................................................
//void Excep_TPU7_TGI7B(void) { Q_ERROR(); }
// vector 147 reserved
// vector 148 reserved
// vector 149 ................................................................
//void Excep_TPU7_TCI7V(void) { Q_ERROR(); }
// vector 150 ................................................................
//void Excep_TPU7_TCI7U(void) { Q_ERROR(); }
// vector 151 ................................................................
//void Excep_TPU8_TGI8A(void) { Q_ERROR(); }
// vector 152 ................................................................
//void Excep_TPU8_TGI8B(void) { Q_ERROR(); }
// vector 153 reserved
// vector 154 ................................................................
//void Excep_TPU8_TCI8V(void) { Q_ERROR(); }
// vector 155 ................................................................
//void Excep_TPU8_TCI8U(void) { Q_ERROR(); }
// vector 156 ................................................................
//void Excep_TPU9_TGI9A(void) { Q_ERROR(); }
// vector 157 ................................................................
//void Excep_TPU9_TGI9B(void) { Q_ERROR(); }
// vector 158 ................................................................
//void Excep_TPU9_TGI9C(void) { Q_ERROR(); }
// vector 159 ................................................................
//void Excep_TPU9_TGI9D(void) { Q_ERROR(); }
// vector 160 ................................................................
//void Excep_TPU9_TCI9V(void) { Q_ERROR(); }
// vector 161 ................................................................
//void Excep_TPU10_TGI10A(void) { Q_ERROR(); }
// vector 162 ................................................................
//void Excep_TPU10_TGI10B(void) { Q_ERROR(); }
// vector 163 reserved
// vector 164 reserved
// vector 165 ................................................................
//void Excep_TPU10_TCI10V(void) { Q_ERROR(); }
// vector 166 ................................................................
//void Excep_TPU10_TCI10U(void) { Q_ERROR(); }
// vector 167 ................................................................
//void Excep_TPU11_TGI11A(void) { Q_ERROR(); }
// vector 168 ................................................................
//void Excep_TPU11_TGI11B(void) { Q_ERROR(); }
// vector 169 reserved
// vector 170 ................................................................
//void Excep_TPU11_TCI11V(void) { Q_ERROR(); }
// vector 171 ................................................................
//void Excep_TPU11_TCI11U(void) { Q_ERROR(); }
// vector 172 reserved
// vector 173 reserved
// vector 174 ................................................................
//void Excep_TMR0_CMI0A(void) { Q_ERROR(); }
// vector 175 ................................................................
//void Excep_TMR0_CMI0B(void) { Q_ERROR(); }
// vector 176 ................................................................
//void Excep_TMR0_OV0I(void) { Q_ERROR(); }
// vector 177 ................................................................
//void Excep_TMR1_CMI1A(void) { Q_ERROR(); }
// vector 178 ................................................................
//void Excep_TMR1_CMI1B(void) { Q_ERROR(); }
// vector 179 ................................................................
//void Excep_TMR1_OV1I(void) { Q_ERROR(); }
// vector 180 ................................................................
//void Excep_TMR2_CMI2A(void) { Q_ERROR(); }
// vector 181 ................................................................
//void Excep_TMR2_CMI2B(void) { Q_ERROR(); }
// vector 182 ................................................................
//void Excep_TMR2_OV2I(void) { Q_ERROR(); }
// vector 183 ................................................................
//void Excep_TMR3_CMI3A(void) { Q_ERROR(); }
// vector 184 ................................................................
//void Excep_TMR3_CMI3B(void) { Q_ERROR(); }
// vector 185 ................................................................
//void Excep_TMR3_OV3I(void) { Q_ERROR(); }
// vector 186 reserved
// vector 187 reserved
// vector 188 reserved
// vector 189 reserved
// vector 190 reserved
// vector 191 reserved
// vector 192 reserved
// vector 193 reserved
// vector 194 reserved
// vector 195 reserved
// vector 196 reserved
// vector 197 reserved
// vector 198 ................................................................
//void Excep_DMAC_DMTEND0(void) { Q_ERROR(); }
// vector 199 ................................................................
//void Excep_DMAC_DMTEND1(void) { Q_ERROR(); }
// vector 200 ................................................................
//void Excep_DMAC_DMTEND2(void) { Q_ERROR(); }
// vector 201 ................................................................
//void Excep_DMAC_DMTEND3(void) { Q_ERROR(); }
// vector 202 reserved
// vector 203 reserved
// vector 204 reserved
// vector 205 reserved
// vector 206 reserved
// vector 207 reserved
// vector 208 reserved
// vector 209 reserved
// vector 210 reserved
// vector 211 reserved
// vector 212 reserved
// vector 213 reserved
// vector 214 ................................................................
//void Excep_SCI0_ERI0(void) { Q_ERROR(); }
// vector 215 ................................................................
//void Excep_SCI0_RXI0(void) { Q_ERROR(); }
// vector 216 ................................................................
//void Excep_SCI0_TXI0(void) { Q_ERROR(); }
// vector 217 ................................................................
//void Excep_SCI0_TEI0(void) { Q_ERROR(); }
// vector 218 ................................................................
//void Excep_SCI1_ERI1(void) { Q_ERROR(); }
// vector 219 ................................................................
//void Excep_SCI1_RXI1(void) { Q_ERROR(); }
// vector 220 ................................................................
//void Excep_SCI1_TXI1(void) { Q_ERROR(); }
// vector 221 ................................................................
//void Excep_SCI1_TEI1(void) { Q_ERROR(); }
// vector 222 ................................................................
//void Excep_SCI2_ERI2(void) { Q_ERROR(); }
// vector 223 ................................................................
//void Excep_SCI2_RXI2(void) { Q_ERROR(); }
// vector 224 ................................................................
//void Excep_SCI2_TXI2(void) { Q_ERROR(); }
// vector 225 ................................................................
//void Excep_SCI2_TEI2(void) { Q_ERROR(); }
// vector 226 ................................................................
//void Excep_SCI3_ERI3(void) { Q_ERROR(); }
// vector 227 ................................................................
//void Excep_SCI3_RXI3(void) { Q_ERROR(); }
// vector 228 ................................................................
//void Excep_SCI3_TXI3(void) { Q_ERROR(); }
// vector 229 ................................................................
//void Excep_SCI3_TEI3(void) { Q_ERROR(); }
// vector 230 ................................................................
//void Excep_SCI4_ERI4(void) { Q_ERROR(); }
// vector 231 ................................................................
//void Excep_SCI4_RXI4(void) { Q_ERROR(); }
// vector 232 ................................................................
//void Excep_SCI4_TXI4(void) { Q_ERROR(); }
// vector 233 ................................................................
//void Excep_SCI4_TEI4(void) { Q_ERROR(); }
// vector 234 ................................................................
//void Excep_SCI5_ERI5(void) { Q_ERROR(); }
// vector 235 ................................................................
//void Excep_SCI5_RXI5(void) { Q_ERROR(); }
// vector 236 ................................................................
//void Excep_SCI5_TXI5(void) { Q_ERROR(); }
// vector 237 ................................................................
//void Excep_SCI5_TEI5(void) { Q_ERROR(); }
// vector 238 ................................................................
//void Excep_SCI6_ERI6(void) { Q_ERROR(); }
// vector 239 ................................................................
//void Excep_SCI6_RXI6(void) { Q_ERROR(); }
// vector 240 ................................................................
//void Excep_SCI6_TXI6(void) { Q_ERROR(); }
// vector 241 ................................................................
//void Excep_SCI6_TEI6(void) { Q_ERROR(); }
// vector 242 reserved
// vector 243 reserved
// vector 244 reserved
// vector 245 reserved
// vector 246 ................................................................
//void Excep_RIIC0_EEI0(void) { Q_ERROR(); }
// vector 247 ................................................................
//void Excep_RIIC0_RXI0(void) { Q_ERROR(); }
// vector 248 ................................................................
//void Excep_RIIC0_TXI0(void) { Q_ERROR(); }
// vector 249 ................................................................
//void Excep_RIIC0_TEI0(void) { Q_ERROR(); }
// vector 250 ................................................................
//void Excep_RIIC1_EEI1(void) { Q_ERROR(); }
// vector 251 ................................................................
//void Excep_RIIC1_RXI1(void) { Q_ERROR(); }
// vector 252 ................................................................
//void Excep_RIIC1_TXI1(void) { Q_ERROR(); }
// vector 253 ................................................................
//void Excep_RIIC1_TEI1(void) { Q_ERROR(); }
// vector 254 reserved
// vector 255 reserved

}                                                                 // exter "C"
