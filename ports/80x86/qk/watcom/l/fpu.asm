;*****************************************************************************
; Purpose: FPU_save() and FPU_restore() implementation for x86/x87
; Last Updated for Version: 4.0.00
; Date of the Last Update:  Feb 18, 2008
;
;                    Q u a n t u m     L e a P s
;                    ---------------------------
;                    innovating embedded systems
;
; Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
;
; This program is open source software: you can redistribute it and/or
; modify it under the terms of the GNU General Public License as published
; by the Free Software Foundation, either version 2 of the License, or
; (at your option) any later version.
;
; Alternatively, this program may be distributed and modified under the
; terms of Quantum Leaps commercial licenses, which expressly supersede
; the GNU General Public License and are specifically designed for
; licensees interested in retaining the proprietary status of their code.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <http://www.gnu.org/licenses/>.
;
; Contact information:
; Quantum Leaps Web sites: http://www.quantum-leaps.com
;                          http://www.state-machine.com
; e-mail:                  info@quantum-leaps.com
;*****************************************************************************

            PUBLIC _FPU_save
            PUBLIC _FPU_restore

.MODEL      LARGE
.CODE
.386

;*****************************************************************************
; void FPU_save(FPU_context *ctx);

_FPU_save    PROC FAR
             PUSH   BP                      ; Save work registers
             MOV    BP,SP
             PUSH   ES
             PUSH   BX
;
             LES    BX, DWORD PTR [BP+6]    ; Point to FPU context memory
;
             FSAVE  ES:[BX]                 ; Save FPU context
;
             POP    BX                      ; Restore work registers
             POP    ES
             POP    BP
;
             RET                            ; Return to caller
_FPU_save    ENDP

;*****************************************************************************
; void FPU_restore(FPU_context *ctx);

_FPU_restore PROC FAR
             PUSH   BP                      ; Save work registers
             MOV    BP,SP
             PUSH   ES
             PUSH   BX
;
             LES    BX, DWORD PTR [BP+6]    ; Point to FPU context memory
;
             FRSTOR ES:[BX]                 ; Restore FPU context
;
             POP    BX                      ; Restore work registers
             POP    ES
             POP    BP
;
             RET                            ; Return to caller
_FPU_restore ENDP

             END
