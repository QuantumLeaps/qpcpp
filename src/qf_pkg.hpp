/// @file
/// @ingroup qf
/// @brief Internal (package scope) QF/C++ interface.
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-17
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

#ifndef QF_PKG_HPP
#define QF_PKG_HPP

//! helper macro to cast const away from an event pointer @p e_
#define QF_EVT_CONST_CAST_(e_) const_cast<QEvt *>(e_)

// QF-specific critical section...
#ifndef QF_CRIT_STAT_TYPE
    //! This is an internal macro for defining the critical section
    //! status type.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// provides the definition of the critical section status variable.
    /// Otherwise this macro is empty.
    /// @sa #QF_CRIT_STAT_TYPE
    #define QF_CRIT_STAT_

    //! This is an internal macro for entering a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes QF_CRIT_ENTRY() passing the key variable as the parameter.
    /// Otherwise QF_CRIT_ENTRY() is invoked with a dummy parameter.
    /// @sa QF_CRIT_ENTRY()
    #define QF_CRIT_E_()        QF_CRIT_ENTRY(dummy)

    //! This is an internal macro for exiting a critical section.
    /// @description
    /// The purpose of this macro is to enable writing the same code for the
    /// case when critical section status type is defined and when it is not.
    /// If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    /// invokes QF_CRIT_EXIT() passing the key variable as the parameter.
    /// Otherwise QF_CRIT_EXIT() is invoked with a dummy parameter.
    /// @sa QF_CRIT_EXIT()
    ///
    #define QF_CRIT_X_()        QF_CRIT_EXIT(dummy)

#elif (!defined QF_CRIT_STAT_)
    #define QF_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QF_CRIT_E_()        QF_CRIT_ENTRY(critStat_)
    #define QF_CRIT_X_()        QF_CRIT_EXIT(critStat_)
#endif  // QF_CRIT_STAT_TYPE

// Assertions inside the crticial section ------------------------------------
#ifdef Q_NASSERT // Q_NASSERT defined--assertion checking disabled

    #define Q_ASSERT_CRIT_(id_, test_)  ((void)0)
    #define Q_REQUIRE_CRIT_(id_, test_) ((void)0)
    #define Q_ERROR_CRIT_(id_)          ((void)0)

#else  // Q_NASSERT not defined--assertion checking enabled

    #define Q_ASSERT_CRIT_(id_, test_) do {\
        if ((test_)) {} else { \
            QF_CRIT_X_(); \
            Q_onAssert(&Q_this_module_[0], static_cast<int_t>(id_)); \
        } \
    } while (false)

    #define Q_REQUIRE_CRIT_(id_, test_) Q_ASSERT_CRIT_((id_), (test_))

    #define Q_ERROR_CRIT_(id_) do { \
        QF_CRIT_X_(); \
        Q_onAssert(&Q_this_module_[0], static_cast<int_t>(id_)); \
    } while (false)

#endif // Q_NASSERT


namespace QP {

// package-scope objects -----------------------------------------------------
extern QF_EPOOL_TYPE_ QF_pool_[QF_MAX_EPOOL]; //!< allocate event pools
extern std::uint_fast8_t QF_maxPool_; //!< # of initialized event pools
extern QSubscrList *QF_subscrList_;   //!< the subscriber list array
extern enum_t QF_maxPubSignal_;       //!< the maximum published signal

//............................................................................
//! Structure representing a free block in the Native QF Memory Pool
/// @sa QP::QMPool
struct QFreeBlock {
    QFreeBlock * volatile m_next;    //!< link to the next free block
};

//............................................................................
// The following flags and bitmasks are for the fields of the @c refCtr_
// attribute of the QP::QTimeEvt class (inherited from QEvt). This attribute
// is NOT used for reference counting in time events, because the @c poolId_
// attribute is zero ("static events").
//
constexpr std::uint8_t TE_IS_LINKED    = 1U << 7U;  // flag
constexpr std::uint8_t TE_WAS_DISARMED = 1U << 6U;  // flag
constexpr std::uint8_t TE_TICK_RATE    = 0x0FU;     // bitmask

//****************************************************************************
// internal helper inline functions

//! return the Pool-ID of an event @p e
inline std::uint8_t QF_EVT_POOL_ID_ (QEvt const * const e) noexcept {
    return e->poolId_;
}

//! return the Reference Conter of an event @p e
inline std::uint8_t QF_EVT_REF_CTR_ (QEvt const * const e) noexcept {
    return e->refCtr_;
}

//! increment the refCtr_ of an event @p e
inline void QF_EVT_REF_CTR_INC_(QEvt const * const e) noexcept {
    ++(QF_EVT_CONST_CAST_(e))->refCtr_;
}

//! decrement the refCtr_ of an event @p e
inline void QF_EVT_REF_CTR_DEC_(QEvt const * const e) noexcept {
    --(QF_EVT_CONST_CAST_(e))->refCtr_;
}

} // namespace QP

//! macro to test that a pointer @p x_ is in range between @p min_ and @p max_
/// @description
/// This macro is specifically and exclusively used for checking the range
/// of a block pointer returned to the pool. Such a check must rely on the
/// pointer arithmetic not compliant with the MISRA-C++:2008 rules ??? and
/// ???. Defining a specific macro for this purpose allows to selectively
/// disable the warnings for this particular case.
#define QF_PTR_RANGE_(x_, min_, max_)  (((min_) <= (x_)) && ((x_) <= (max_)))

//! access element at index @p i_ from the base pointer @p base_
#define QF_PTR_AT_(base_, i_) (base_[i_])

#endif  // QF_PKG_HPP
