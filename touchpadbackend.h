/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOUCHPADBACKEND_H
#define TOUCHPADBACKEND_H

#include <QObject>

#include <kdemacros.h>

class TouchpadParameters;

class KDE_EXPORT TouchpadBackend : public QObject
{
    Q_OBJECT
public:
    explicit TouchpadBackend(QObject *parent);

    static TouchpadBackend *self();

    virtual bool applyConfig(const TouchpadParameters *) = 0;
    virtual bool getConfig(TouchpadParameters *) = 0;
    virtual const QStringList &supportedParameters() const = 0;
    virtual const QString &errorString() const = 0;

    enum TouchpadState {
        TouchpadEnabled, TouchpadFullyDisabled, TouchpadTapAndScrollDisabled
    };
    virtual void setTouchpadState(TouchpadState) = 0;
    virtual TouchpadState getTouchpadState() = 0;

    virtual bool isMousePluggedIn() = 0;

    virtual void watchForEvents(bool keyboard) = 0;

Q_SIGNALS:
    void touchpadStateChanged();
    void mousesChanged();
    void keyboardActivityStarted();
    void keyboardActivityFinished();
};

#endif // TOUCHPADBACKEND_H
