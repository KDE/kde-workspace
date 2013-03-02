/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef SCREENLOCKER_GREETERAPP_H
#define SCREENLOCKER_GREETERAPP_H

#include <KDE/KApplication>

#include <Plasma/PackageStructure>

#include <QDeclarativeView>

namespace Plasma {
    class Package;
};

namespace ScreenLocker
{
class Unlocker;
class ScreenSaverWindow;

class UnlockApp : public KApplication
{
    Q_OBJECT
public:
    UnlockApp();
    virtual ~UnlockApp();

    void setTesting(bool enable);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void viewStatusChanged(const QDeclarativeView::Status &status);
    void desktopResized();
    void resetRequestIgnore();
    void suspendToRam();
    void suspendToDisk();
    void shutdown();
    void getFocus();

private:
    void initialize();
    void capsLocked();

    QString m_mainQmlPath;
    QList<QDeclarativeView*> m_views;
    QList<ScreenSaverWindow*> m_screensaverWindows;
    QTimer *m_resetRequestIgnoreTimer;
    Plasma::PackageStructure::Ptr m_structure;
    Plasma::Package *m_package;
    bool m_testing;
    bool m_capsLocked;
    bool m_ignoreRequests;
    bool m_showScreenSaver;
};
} // namespace

#endif // SCREENLOCKER_GREETERAPP_H
