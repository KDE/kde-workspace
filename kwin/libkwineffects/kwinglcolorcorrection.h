/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2012 Casian Andrei <skeletk13@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef KWIN_COLOR_CORRECTION_H
#define KWIN_COLOR_CORRECTION_H

#include "kwinglobals.h"

#include <QObject>

namespace KWin {

class ColorCorrectionPrivate;

/**
 * Implements a color correction mechanism. The settings are obtained
 * asynchronously via D-Bus from kolor-server, which is part of kolor-manager.
 *
 * If it fails to get the settings, nothing should happen (no correction), even
 * if it is set to enabled.
 *
 * Supports per-output and per-region correction (window region).
 *
 * \warning This class is not designed to be used by effects, however
 * it may happen to be useful their case somehow.
 */
class KWIN_EXPORT ColorCorrection : public QObject
{
    Q_OBJECT

public:
    explicit ColorCorrection(QObject *parent = 0);
    virtual ~ColorCorrection();

    /**
     * Prepares color correction for the output number \param screen.
     * Sets up the appropriate color lookup texture for the output.
     */
    void setupForOutput(int screen);

    /**
     * Unsets color correction by using a dummy color lookup texture. This
     * does not disable anything, the CC mechanisms remain in place. Instead, it
     * indicates to draw normally.
     */
    void reset();

    /**
     * Modifies \param sourceCode, making it suitable for performing
     * color correction. This is done by inserting a 3d texture lookup operation
     * just before the output fragment color is returned.
     */
    static QByteArray prepareFragmentShader(const QByteArray &sourceCode);

    /**
     * @return whether color correction is enabled
     */
    bool isEnabled() const;

public slots:
    /**
     * Enables or disables color correction. Compositing should be restarted
     * for changes to take effect.
     *
     * @return true when successful
     */
    bool setEnabled(bool enabled);

signals:
    /**
     * Emitted when some changes happened to the color correction settings, and
     * a full repaint of the scene should be done to make the new settings visible.
     */
    void changed();

    /**
     * Emitted when something failed because of an error (probably a GL error)
     * The description of the error should be found in the logs.
     */
    void errorOccured();

private:
    ColorCorrectionPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(ColorCorrection)
};

} // KWin namespace

#endif // KWIN_COLOR_CORRECTION_H
