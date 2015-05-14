/// @file
/// @brief QP/C++ port to Qt
/// @cond
///***************************************************************************
/// Last Updated for Version: QP 5.4.0/Qt 5.x
/// Last updated on  2015-05-03
///
///                    Q u a n t u m     L e a P s
///                    ---------------------------
///                    innovating embedded systems
///
/// Copyright (C) Quantum Leaps, www.state-machine.com.
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
/// Web:   www.state-machine.com
/// Email: info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef guiactive_h
#define guiactive_h

namespace QP {

//............................................................................
class GuiQMActive : public QMActive {
public:
    GuiQMActive(QStateHandler const initial) : QMActive(initial) {}
    virtual void start(uint_fast8_t const prio,
                       QEvt const *qSto[], uint_fast16_t const qLen,
                       void * const stkSto, uint_fast16_t const stkSize,
                       QEvt const * const ie);
#ifndef Q_SPY
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin);
#else
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin,
                       void const * const sender);
#endif // Q_SPY
    virtual void postLIFO(QEvt const * const e);
};

//............................................................................
class GuiQActive : public QActive {
public:
    GuiQActive(QStateHandler const initial) : QActive(initial) {}
    virtual void start(uint_fast8_t const prio,
        QEvt const *qSto[], uint_fast16_t const qLen,
        void * const stkSto, uint_fast16_t const stkSize,
        QEvt const * const ie);
#ifndef Q_SPY
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin);
#else
    virtual bool post_(QEvt const * const e, uint_fast16_t const margin,
        void const * const sender);
#endif // Q_SPY
    virtual void postLIFO(QEvt const * const e);
};

} // namespace QP

#endif // guiactive_h
