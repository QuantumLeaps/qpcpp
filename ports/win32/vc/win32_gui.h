//////////////////////////////////////////////////////////////////////////////
// Product: Win32 GUI facilities for building realistic embedded front panels
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Aug 04, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef win32_gui_h
#define win32_gui_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>                                              // Win32 API

// create the custom dialog hosting the embedded front panel .................
HWND CreateCustDialog(HINSTANCE hInst, int iDlg, HWND hParent,
                      WNDPROC lpfnWndProc, LPCTSTR lpWndClass);

// OwnerDrawnButton "class" ..................................................
class OwnerDrawnButton {
    HBITMAP m_hBitmapUp;
    HBITMAP m_hBitmapDown;
    HCURSOR m_hCursor;

public:
    enum OwnerDrawnButtonAction {
        BTN_NOACTION,
        BTN_PAINTED,
        BTN_DEPRESSED,
        BTN_RELEASED
    };

    void init(HBITMAP hBitmapUp, HBITMAP hBitmapDwn,
              HCURSOR hCursor);
    virtual ~OwnerDrawnButton();
    OwnerDrawnButtonAction draw(LPDRAWITEMSTRUCT lpdis);
};

// DotMatrix "class" for drawing graphic displays with up to 24-bit color.....
class DotMatrix {
    UINT    m_width;
    UINT    m_xScale;
    UINT    m_height;
    UINT    m_yScale;
    HBITMAP m_hBitmap;
    HWND    m_hItem;
    BYTE   *m_bits;
    BYTE    m_bgColor[3];

public:
    void init(UINT width,  UINT xScale,
              UINT height, UINT yScale,
              HWND hItem,  BYTE const bgColor[3]);
    virtual ~DotMatrix();
    void clear(void);
    void setPixel(UINT x, UINT y, BYTE const color[3]);
    void clearPixel(UINT x, UINT y) {
        setPixel(x, y, m_bgColor);
    }
    void redraw(void);
};

// SegmentDisplay "class" for drawing segment displays, LEDs, etc.............
class SegmentDisplay {
    HWND    *m_hSegment;                          // array of segment controls
    UINT     m_segmentNum;                               // number of segments
    HBITMAP *m_hBitmap;                             // array of bitmap handles
    UINT     m_bitmapNum;                                 // number of bitmaps

public:
    void init(UINT segNum, UINT bitmapNum);
    virtual ~SegmentDisplay();
    bool initSegment(UINT segmentNum, HWND hSegment);
    bool initBitmap(UINT bitmapNum, HBITMAP hBitmap);
    bool setSegment(UINT segmentNum, UINT bitmapNum);
};

// useful helper functions ...................................................
void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart);

#endif                                                          // win32_gui_h
