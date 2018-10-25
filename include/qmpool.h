/// @file
/// @brief platform-independent memory pool QP::QMPool interface.
/// @ingroup qf
/// @cond
///***************************************************************************
/// Last updated for version 6.3.6
/// Last updated on  2018-10-04
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
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
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond

#ifndef qmpool_h
#define qmpool_h

#ifndef QF_MPOOL_SIZ_SIZE
    //! macro to override the default QP::QMPoolSize size.
    /// Valid values 1, 2, or 4; default 2
    #define QF_MPOOL_SIZ_SIZE 2
#endif

#ifndef QF_MPOOL_CTR_SIZE
    //! macro to override the default QMPoolCtr size.
    //! Valid values 1, 2, or 4; default 2
    #define QF_MPOOL_CTR_SIZE 2
#endif

namespace QP {
#if (QF_MPOOL_SIZ_SIZE == 1)
    typedef uint8_t QMPoolSize;
#elif (QF_MPOOL_SIZ_SIZE == 2)
    //! The data type to store the block-size based on the macro
    //! #QF_MPOOL_SIZ_SIZE.
    /// @description
    /// The dynamic range of this data type determines the maximum size
    /// of blocks that can be managed by the native QF event pool.
    typedef uint16_t QMPoolSize;
#elif (QF_MPOOL_SIZ_SIZE == 4)
    typedef uint32_t QMPoolSize;
#else
    #error "QF_MPOOL_SIZ_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

#if (QF_MPOOL_CTR_SIZE == 1)
    typedef uint8_t QMPoolCtr;
#elif (QF_MPOOL_CTR_SIZE == 2)
    //! The data type to store the block-counter based on the macro
    //! #QF_MPOOL_CTR_SIZE.
    /// @description
    /// The dynamic range of this data type determines the maximum number
    /// of blocks that can be stored in the pool.
    typedef uint16_t QMPoolCtr;
#elif (QF_MPOOL_CTR_SIZE == 4)
    typedef uint32_t QMPoolCtr;
#else
    #error "QF_MPOOL_CTR_SIZE defined incorrectly, expected 1, 2, or 4"
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
    void init(void * const poolSto, uint_fast32_t poolSize,
              uint_fast16_t blockSize);

    //! Obtains a memory block from a memory pool.
    void *get(uint_fast16_t const margin);

    //! Returns a memory block back to a memory pool.
    void put(void * const b);

    //! return the fixed block-size of the blocks managed by this pool
    QMPoolSize getBlockSize(void) const {
        return m_blockSize;
    }

// duplicated API to be used exclusively inside ISRs (useful in some QP ports)
#ifdef QF_ISR_API
    void *getFromISR(uint_fast16_t const margin);
    void putFromISR(void * const b);
#endif // QF_ISR_API

private:
    QMPool(QMPool const &);            //!< disallow copying of QMPools
    QMPool &operator=(QMPool const &); //!< disallow assigning of QMPools

    friend class QF;
#ifdef Q_UTEST
    friend class QS;
#endif // Q_UTEST
};

} // namespace QP

//! Memory pool element to allocate correctly aligned storage for QP::QMPool
#define QF_MPOOL_EL(type_) \
    struct { void *sto_[((sizeof(type_) - 1U)/sizeof(void*)) + 1U]; }

#endif  // qmpool_h

