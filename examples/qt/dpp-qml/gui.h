//****************************************************************************
// Product: QP/C++ example for Qt with GUI
// Last Updated for Version: QP 6.3.0/Qt 5.x
// Date of the Last Update:  May 16, 2018
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
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
//                          https://state-machine.com
// e-mail:                  info@quantum-leaps.com
//****************************************************************************
#ifndef GUI_H
#define GUI_H

#include <QObject>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

class Gui : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QString> state READ State WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString button READ Button WRITE setButton NOTIFY buttonChanged)

public:
    Gui(QObject *parent = 0);
    static Gui *instance();
    void show();

public:
    Q_INVOKABLE void onPausePressed();
    Q_INVOKABLE void onPauseReleased();
    Q_INVOKABLE void onQuit();

    QList<QString> State() const;
    void setState(const QList<QString>& state);

    QString Button() const;
    void setButton(const QString& button);

private:
    QQmlApplicationEngine engine;
    QList<QString> m_State;
    QString m_Button;

signals:
    void stateChanged();
    void buttonChanged();
};

#endif                                                                // GUI_H
