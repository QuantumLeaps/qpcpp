//////////////////////////////////////////////////////////////////////////////
// Product: Direct Video (VGA) screen output
// Last Updated for Version: 4.0.00
// Date of the Last Update:  Apr 07, 2008
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef video_h
#define video_h

class Video {
public:
    enum VideoColor {
        // foreground colors ...
        FGND_BLACK        = 0x00,
        FGND_BLUE         = 0x01,
        FGND_GREEN        = 0x02,
        FGND_CYAN         = 0x03,
        FGND_RED          = 0x04,
        FGND_PURPLE       = 0x05,
        FGND_BROWN        = 0x06,
        FGND_LIGHT_GRAY   = 0x07,
        FGND_DARK_GRAY    = 0x08,
        FGND_LIGHT_BLUE   = 0x09,
        FGND_LIGHT_GREEN  = 0x0A,
        FGND_LIGHT_CYAN   = 0x0B,
        FGND_LIGHT_RED    = 0x0C,
        FGND_LIGHT_PURPLE = 0x0D,
        FGND_YELLOW       = 0x0E,
        FGND_WHITE        = 0x0F,
        // background colors ...
        BGND_BLACK        = 0x00,
        BGND_BLUE         = 0x10,
        BGND_GREEN        = 0x20,
        BGND_CYAN         = 0x30,
        BGND_RED          = 0x40,
        BGND_PURPLE       = 0x50,
        BGND_BROWN        = 0x60,
        BGND_LIGHT_GRAY   = 0x70,

        BGND_BLINK        = 0x80
    };
    static void clearScreen(uint8_t bgColor);
    static void clearRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                          uint8_t bgColor);
    static void drawRect (uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                          uint8_t fgColor, uint8_t line);
    static void printStrAt(uint8_t x, uint8_t y, uint8_t color,
                           char const *str);
    static void printNumAt(uint8_t x, uint8_t y, uint8_t color, uint32_t num);

    static void drawBitmapAt(uint8_t x, uint8_t y,
                        uint8_t const *bitmap, uint8_t width, uint8_t height);

    static void drawStringAt(uint8_t x, uint8_t y, char const *str);
};

#endif                                                              // video_h

