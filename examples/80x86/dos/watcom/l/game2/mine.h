//////////////////////////////////////////////////////////////////////////////
// Product: Product: "Fly'n'Shoot" game
// Last Updated for Version: 4.0.04
// Date of the Last Update:  Sep 30, 2009
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2009 Quantum Leaps, LLC. All rights reserved.
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
#ifndef mine_h
#define mine_h

class Mine : public QHsm {                            // extend the QHsm class
protected:
    uint8_t m_id;
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_exp_ctr;

public:
    Mine(uint8_t id) : QHsm((QStateHandler)&Mine::initial), m_id(id) {}

protected:
    virtual void onInitial(void) = 0;
    virtual void onDrawMine(void) = 0;
    virtual uint8_t onShipCollision(ObjectImageEvt const *e) = 0;
    virtual uint8_t onMissileCollision(ObjectImageEvt const *e) = 0;

private:
    static QState initial  (Mine *me, QEvent const *e);
    static QState unused   (Mine *me, QEvent const *e);
    static QState used     (Mine *me, QEvent const *e);
    static QState planted  (Mine *me, QEvent const *e);
    static QState exploding(Mine *me, QEvent const *e);
};

#endif                                                               // mine_h

