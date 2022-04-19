//============================================================================
// Product:  Board Support Package (BSP) for the Calculator example
// Last Updated for Version: 5.4.2
// Date of the Last Update:  2015-06-05
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
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
#ifndef BSP_HPP
#define BSP_HPP

#define KEY_PLUS    '+'
#define KEY_MINUS   '-'
#define KEY_MULT    '*'
#define KEY_DIVIDE  '/'

void BSP_clear (void);
void BSP_negate(void);
void BSP_insert(int keyId);
double BSP_get_value(void);
int  BSP_eval(double operand1, int oper, double operand2);
void BSP_exit(void);

void BSP_display(void);
void BSP_message(char const *state);

#endif // BSP_HPP
