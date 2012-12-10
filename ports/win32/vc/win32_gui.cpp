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
#include "win32_gui.h"
#include <stdlib.h>

//----------------------------------------------------------------------------
HWND CreateCustDialog(HINSTANCE hInst, int iDlg, HWND hParent,
                      WNDPROC lpfnWndProc, LPCTSTR lpWndClass)
{
    WNDCLASSEX wndclass;
    HWND       hWnd;

    wndclass.cbSize        = sizeof(wndclass);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = lpfnWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = DLGWINDOWEXTRA;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = lpWndClass;
    wndclass.hIconSm       = NULL;

    RegisterClassEx(&wndclass);

    hWnd = CreateDialog(hInst, MAKEINTRESOURCE(iDlg), hParent, NULL);

    // NOTE: WM_INITDIALOG provides stimulus for initializing dialog controls.
    // Dialog box procedures typically use this message to initialize controls
    // and carry out any other initialization tasks that affect the appearance
    // of the dialog box.
    //
    SendMessage(hWnd, WM_INITDIALOG, (WPARAM)0, (LPARAM)0);

    return hWnd;
}

//----------------------------------------------------------------------------
void OwnerDrawnButton::init(HBITMAP hBitmapUp, HBITMAP hBitmapDwn,
                            HCURSOR hCursor)
{
    m_hBitmapUp   = hBitmapUp;
    m_hBitmapDown = hBitmapDwn;
    m_hCursor     = hCursor;
}
//............................................................................
OwnerDrawnButton::~OwnerDrawnButton() {
    DeleteObject(m_hBitmapUp);
    DeleteObject(m_hBitmapDown);
}
//............................................................................
OwnerDrawnButton::OwnerDrawnButtonAction OwnerDrawnButton::draw(
                               LPDRAWITEMSTRUCT lpdis)
{
    OwnerDrawnButtonAction ret = BTN_NOACTION;

    if ((lpdis->itemAction & ODA_DRAWENTIRE) != 0U) {
        if (m_hCursor != NULL) {
           SetClassLong(lpdis->hwndItem, GCL_HCURSOR, (LONG)m_hCursor);
           m_hCursor = NULL;                            // mark the cursor set
        }
        DrawBitmap(lpdis->hDC, m_hBitmapUp,
                   lpdis->rcItem.left, lpdis->rcItem.top);
        ret = BTN_PAINTED;
    }
    else if ((lpdis->itemAction & ODA_SELECT) != 0U) {
        if ((lpdis->itemState & ODS_SELECTED) != 0U) {
            DrawBitmap(lpdis->hDC, m_hBitmapDown,
                       lpdis->rcItem.left, lpdis->rcItem.top);
            ret = BTN_DEPRESSED;
        }
        else {
            // NOTE: the bitmap for button "UP" look will be
            // drawn in the ODA_DRAWENTIRE action
            //
            ret = BTN_RELEASED;
        }
    }
    return ret;
}

//----------------------------------------------------------------------------
void DotMatrix::init(UINT width,  UINT xScale,
                     UINT height, UINT yScale,
                     HWND hItem,  BYTE const bgColor[3])
{
    HDC hDC;
    BITMAPINFO bi24BitInfo;

    m_width  = width;
    m_xScale = xScale;
    m_height = height;
    m_yScale = yScale;

    m_bgColor[0] = bgColor[0];
    m_bgColor[1] = bgColor[1];
    m_bgColor[2] = bgColor[2];

    m_hItem = hItem;

    bi24BitInfo.bmiHeader.biBitCount    = 3U*8U;                // 3 RGB bytes
    bi24BitInfo.bmiHeader.biCompression = BI_RGB;                 // RGB color
    bi24BitInfo.bmiHeader.biPlanes      = 1U;
    bi24BitInfo.bmiHeader.biSize        = sizeof(bi24BitInfo.bmiHeader);
    bi24BitInfo.bmiHeader.biWidth       = m_width  * m_xScale;
    bi24BitInfo.bmiHeader.biHeight      = m_height * m_yScale;

    hDC = CreateCompatibleDC(NULL);
    m_hBitmap = CreateDIBSection(hDC, &bi24BitInfo, DIB_RGB_COLORS,
                                   (void **)&m_bits, 0, 0);
    DeleteDC(hDC);

    clear();
    redraw();
}
//............................................................................
DotMatrix::~DotMatrix() {
    DeleteObject(m_hBitmap);
}
//............................................................................
void DotMatrix::clear(void) {
    BYTE r = m_bgColor[0];
    BYTE g = m_bgColor[1];
    BYTE b = m_bgColor[2];
    BYTE *bits = m_bits;

    for (unsigned n = m_width*m_xScale * m_height * m_yScale;
         n != 0U;
         --n, bits += 3)
    {
        bits[0] = b;
        bits[1] = g;
        bits[2] = r;
    }
}
//............................................................................
void DotMatrix::setPixel(UINT x, UINT y, BYTE const color[3]) {
    BYTE *pixelRGB = &m_bits[3*(m_xScale*x
                  + m_xScale*m_width * m_yScale*(m_height - 1U - y))];
    BYTE r = color[0];
    BYTE g = color[1];
    BYTE b = color[2];
    for (UINT sy = m_yScale; sy != 0U;
         --sy, pixelRGB += m_xScale*m_width*3U)
    {
        for (UINT sx = 3U*m_xScale; sx != 0U; sx -= 3U) {
            pixelRGB[sx - 3U] = b;
            pixelRGB[sx - 2U] = g;
            pixelRGB[sx - 1U] = r;
        }
    }
}
//............................................................................
void DotMatrix::redraw(void) {
    SendMessage(m_hItem, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)m_hBitmap);
}

// SegmentDisplay ------------------------------------------------------------
void SegmentDisplay::init(UINT segmentNum, UINT bitmapNum) {
    m_hSegment = new HWND[segmentNum];
    m_segmentNum = segmentNum;
    for (UINT n = 0U; n < segmentNum; ++n) {
        m_hSegment[n] = NULL;
    }
    m_hBitmap = new HBITMAP[bitmapNum];
    m_bitmapNum = bitmapNum;
    for (UINT n = 0U; n < bitmapNum; ++n) {
        m_hBitmap[n] = NULL;
    }
}
//............................................................................
SegmentDisplay::~SegmentDisplay() {
    for (UINT n = 0U; n < m_segmentNum; ++n) {
        DeleteObject(m_hSegment[n]);
    }
    delete[] m_hSegment;

    for (UINT n = 0U; n < m_bitmapNum; ++n) {
        DeleteObject(m_hBitmap[n]);
    }
    delete[] m_hBitmap;
}
//............................................................................
bool SegmentDisplay::initSegment(UINT segmentNum, HWND hSegment) {
    if ((segmentNum < m_segmentNum) && (hSegment != NULL)) {
        m_hSegment[segmentNum] = hSegment;
        return true;
    }
    else {
        return false;
    }
}
//............................................................................
bool SegmentDisplay::initBitmap(UINT bitmapNum, HBITMAP hBitmap) {
    if ((bitmapNum < m_bitmapNum) && (hBitmap != NULL)) {
        m_hBitmap[bitmapNum] = hBitmap;
        return true;
    }
    else {
        return false;
    }
}
//............................................................................
bool SegmentDisplay::setSegment(UINT segmentNum, UINT bitmapNum) {
    if ((segmentNum < m_segmentNum) && (bitmapNum < m_bitmapNum)) {
        SendMessage(m_hSegment[segmentNum], STM_SETIMAGE,
                IMAGE_BITMAP,
                (LPARAM)m_hBitmap[bitmapNum]);
        return true;
    }
    else {
        return false;
    }
}

//----------------------------------------------------------------------------
// DrawBitmap() function adapted from the book "Programming Windows" by
// Charles Petzold.
//
void DrawBitmap(HDC hdc, HBITMAP hBitmap,
                int xStart, int yStart)
{
    BITMAP bm;
    POINT  ptSize, ptOrg;
    HDC    hdcMem = CreateCompatibleDC(hdc);

    SelectObject(hdcMem, hBitmap);
    SetMapMode(hdcMem, GetMapMode(hdc));

    GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP(hdc, &ptSize, 1);

    ptOrg.x = 0;
    ptOrg.y = 0;
    DPtoLP(hdcMem, &ptOrg, 1);

    BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y,
           hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
    DeleteDC(hdcMem);
}


