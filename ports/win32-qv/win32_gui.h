/// \file
/// \brief Win32 GUI facilities for building realistic embedded front panels
/// \cond
///***************************************************************************
/// Last updated for version 5.4.2
/// Last updated on  2015-06-05
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
/// \endcond

#ifndef win32_gui_h
#define win32_gui_h

#ifndef WIN32_GUI
    #error The pre-processor macro WIN32_GUI must be defined
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Win32 API

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

// GraphicDisplay "class" for drawing graphic displays with up to 24-bit color
class GraphicDisplay {
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
    virtual ~GraphicDisplay();
    void clear(void);
    void setPixel(UINT x, UINT y, BYTE const color[3]);
    void clearPixel(UINT x, UINT y) {
        setPixel(x, y, m_bgColor);
    }
    void redraw(void);
};

// SegmentDisplay "class" for drawing segment displays, LEDs, etc.............
class SegmentDisplay {
    HWND    *m_hSegment;   // array of segment controls
    UINT     m_segmentNum; // number of segments
    HBITMAP *m_hBitmap;    // array of bitmap handles
    UINT     m_bitmapNum;  // number of bitmaps

public:
    void init(UINT segNum, UINT bitmapNum);
    virtual ~SegmentDisplay();
    bool initSegment(UINT segmentNum, HWND hSegment);
    bool initBitmap(UINT bitmapNum, HBITMAP hBitmap);
    bool setSegment(UINT segmentNum, UINT bitmapNum);
};

// useful helper functions ...................................................
void DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart);

#endif // win32_gui_h
