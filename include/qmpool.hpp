/// @file
/// @brief platform-independent memory pool QP::QMPool interface.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.9.1
/// Last updated on  2020-09-14
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

#ifndef QMPOOL_HPP
#define QMPOOL_HPP

#ifndef QF_MPOOL_SIZ_SIZE
    //! macro to override the default QP::QMPoolSize size.
    /// Valid values 1U, 2U, or 4U; default 2U
    #define QF_MPOOL_SIZ_SIZE 2U
#endif

#ifndef QF_MPOOL_CTR_SIZE
    //! macro to override the default QMPoolCtr size.
    //! Valid values 1U, 2U, or 4U; default 2U
    #define QF_MPOOL_CTR_SIZE 2
#endif

namespace QP {
#if (QF_MPOOL_SIZ_SIZE == 1U)
    using QMPoolSize = std::uint8_t;
#elif (QF_MPOOL_SIZ_SIZE == 2U)
    //! The data type to store the block-size based on the macro
    //! #QF_MPOOL_SIZ_SIZE.
    /// @description
    /// The dynamic range of this data type determines the maximum size
    /// of blocks that can be managed by the native QF event pool.
    using QMPoolSize = std::uint16_t;
#elif (QF_MPOOL_SIZ_SIZE == 4U)
    using QMPoolSize = std::uint32_t;
#else
    #error "QF_MPOOL_SIZ_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

#if (QF_MPOOL_CTR_SIZE == 1U)
    using QMPoolCtr = std::uint8_t;
#elif (QF_MPOOL_CTR_SIZE == 2U)
    //! The data type to store the block-counter based on the macro
    //! #QF_MPOOL_CTR_SIZE.
    /// @description
    /// The dynamic range of this data type determines the maximum number
    /// of blocks that can be stored in the pool.
    using QMPoolCtr = std::uint16_t;
#elif (QF_MPOOL_CTR_SIZE == 4U)
    using QMPoolCtr = std::uint32_t;
#else
    #error "QF_MPOOL_CTR_SIZE defined incorrectly, expected 1U, 2U, or 4U"
#endif

//****************************************************************************
//! Native QF memory pool class
/// @description
/// A fixed block-size memory pool is a very fast and efficient data
/// structure for dynamic allocation of fixed block-size chunks of memory.
/// A memory pool offers fast and deterministic allocation and recycling of
/// memory blocks and is not subject to fragmenation.@n
/// @n
/// The QP::QMPool class describes the native QF memory pool, which can be
/// used as the event pool for dynamic event allocation, or as a fast,
/// deterministic fixed block-size heap for any other objects in your
/// application.
///
/// @note
/// The QP::QMPool class contains only data members for managing a memory
/// pool, but does not contain the pool storage, which must be provided
/// externally during the pool initialization.
///
/// @note
/// The native QF event pool is configured by defining the macro
/// #QF_EPOOL_TYPE_ as QP::QMPool in the specific QF port header file.
class QMPool {
private:

    //! start of the memory managed by this memory pool
    void *m_start;

    //! end of the memory managed by this memory pool
    void *m_end;

    //! head of linked list of free blocks
    void * volatile m_free_head;

    //! maximum block size (in bytes)
    QMPoolSize m_blockSize;

    //! total number of blocks
    QMPoolCtr m_nTot;

    //! number of free blocks remaining
    QMPoolCtr volatile m_nFree;

    //! minimum number of free blocks ever present in this pool
    /// @note
    /// This attribute remembers the low watermark of the pool,
    /// which provides a valuable information for sizing event pools.
    ///
    /// @sa QP::QF::getPoolMin().
    QMPoolCtr m_nMin;

public:
    QMPool(void); //!< public default constructor

    //! Initializes the native QF event pool
    void init(void * const poolSto, std::uint_fast32_t poolSize,
              std::uint_fast16_t blockSize) noexcept;

    //! Obtains a memory block from a memory pool.
    void *get(std::uint_fast16_t const margin,
              std::uint_fast8_t const qs_id) noexcept;

    //! Returns a memory block back to a memory pool.
    void put(void * const b, std::uint_fast8_t const qs_id) noexcept;

    //! return the fixed block-size of the blocks managed by this pool
    QMPoolSize getBlockSize(void) const noexcept {
        return m_blockSize;
    }

// duplicated API to be used exclusively inside ISRs (useful in some QP ports)
#ifdef QF_ISR_API
    void *getFromISR(std::uint_fast16_t const margin,
                     std::uint_fast8_t const qs_id) noexcept;
    void putFromISR(void * const b,
                    std::uint_fast8_t const qs_id) noexcept;
#endif // QF_ISR_API

private:
    //! disallow copying of QMPools
    QMPool(QMPool const &) = delete;

    //!< disallow assigning of QMPools
    QMPool &operator=(QMPool const &) = delete;

    friend class QF;
    friend class QS;
};

} // namespace QP

//! Memory pool element to allocate correctly aligned storage for QP::QMPool
#define QF_MPOOL_EL(type_) \
    struct { void *sto_[((sizeof(type_) - 1U)/sizeof(void*)) + 1U]; }

#endif  // QMPOOL_HPP

