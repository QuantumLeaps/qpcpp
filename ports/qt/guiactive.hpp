/// @file
/// @brief QP/C++ port to Qt
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0 / Qt 5.x
/// Last updated on  2020-01-23
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2020 Quantum Leaps. All rights reserved.
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
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#ifndef GUIACTIVE_HPP
#define GUIACTIVE_HPP

namespace QP {

//............................................................................
class GuiQActive : public QActive {
public:
    GuiQActive(QStateHandler const initial) : QActive(initial) {}
    void start(std::uint_fast8_t const prio,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize,
               void const * const par) override;
#ifndef Q_SPY
    bool post_(QEvt const * const e, std::uint_fast16_t const margin) override;
#else
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
        void const * const sender) override;
#endif // Q_SPY
    void postLIFO(QEvt const * const e) override;
};

//............................................................................
class GuiQMActive : public QMActive {
public:
    GuiQMActive(QStateHandler const initial) : QMActive(initial) {}
    void start(std::uint_fast8_t const prio,
               QEvt const * * const qSto, std::uint_fast16_t const qLen,
               void * const stkSto, std::uint_fast16_t const stkSize,
               void const * const par) override;
#ifndef Q_SPY
    bool post_(QEvt const * const e, std::uint_fast16_t const margin) override;
#else
    bool post_(QEvt const * const e, std::uint_fast16_t const margin,
                       void const * const sender) override;
#endif // Q_SPY
    void postLIFO(QEvt const * const e) override;
};

} // namespace QP

#endif // GUIACTIVE_HPP
