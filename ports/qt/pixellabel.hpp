//! @file
//! @brief QP/C++ port to Qt
//! @cond
//============================================================================
//! Last updated for version 6.6.0 / Qt 5.x
//! Last updated on  2019-07-30
//!
//!                    Q u a n t u m  L e a P s
//!                    ------------------------
//!                    Modern Embedded Software
//!
//! Copyright (C) 2005-2019 Quantum Leaps. All rights reserved.
//!
//! This program is open source software: you can redistribute it and/or
//! modify it under the terms of the GNU General Public License as published
//! by the Free Software Foundation, either version 3 of the License, or
//! (at your option) any later version.
//!
//! Alternatively, this program may be distributed and modified under the
//! terms of Quantum Leaps commercial licenses, which expressly supersede
//! the GNU General Public License and are specifically designed for
//! licensees interested in retaining the proprietary status of their code.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//! GNU General Public License for more details.
//!
//! You should have received a copy of the GNU General Public License
//! along with this program. If not, see <www.gnu.org/licenses>.
//!
//! Contact information:
//! <www.state-machine.com/licensing>
//! <info@state-machine.com>
//============================================================================
//! @endcond

#ifndef pixel_label_h
#define pixel_label_h

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

#endif // pixel_label_h
