//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-25
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QP/C++ port to Qt

#ifndef PIXEL_LABEL_HPP
#define PIXEL_LABEL_HPP

#include <QLabel>
#include <QPixmap>

// PixelLabel class is for drawing graphic displays with up to 24-bit color
class PixelLabel : public QLabel {
    Q_OBJECT

public:
    PixelLabel(QWidget *parent = 0)
      : QLabel(parent), m_bits(0)
    {}
    virtual ~PixelLabel();
    void init(int width,  int xScale,
              int height, int yScale,
              quint8 const bgColor[3]);
    void clear(void);
    void setPixel(int x, int y, quint8 const color[3]);
    void clearPixel(int x, int y) {
        setPixel(x, y, m_bgColor);
    }
    void redraw(void);

private:
    int      m_width;
    int      m_xScale;
    int      m_height;
    int      m_yScale;
    QPixmap  m_pixmap;
    quint32  m_size;
    quint8  *m_bits;
    quint8   m_bgColor[3];
};

#endif // PIXEL_LABEL_HPP
