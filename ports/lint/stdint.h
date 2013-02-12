/*****************************************************************************
* Product: Exact-width integer types for Lint. NOTE: Adjust for you project!
* Last Updated for Version: 4.5.04
* Date of the Last Update:  Feb 09, 2013
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* Quantum Leaps Web sites: http://www.quantum-leaps.com
*                          http://www.state-machine.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#ifndef stdint_h
#define stdint_h

/*lint -save -e1960    MISRA-C++:2008 Rule 17-0-2, Re-use of C++ identifier */

/* Exact-width types. WG14/N843 C99 Standard, Section 7.18.1.1 */
typedef signed   char  int8_t;       /**< C99 exact-width  8-bit signed int */
typedef signed   short int16_t;      /**< C99 exact-width 16-bit signed int */
typedef signed   long  int32_t;      /**< C99 exact-width 32-bit signed int */

typedef unsigned char  uint8_t;    /**< C99 exact-width  8-bit unsigned int */
typedef unsigned short uint16_t;   /**< C99 exact-width 16-bit unsigned int */
typedef unsigned long  uint32_t;   /**< C99 exact-width 32-bit unsigned int */

/*lint -restore */

#endif                                                          /* stdint_h */

