/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef SHUTDOWNDLG_H
#define SHUTDOWNDLG_H

#include <QDialog>
#include <QPushButton>
#include <kworkspace/kworkspace.h>

class QMenu;
class QTimer;
class QTimeLine;
class QLabel;
class LogoutEffect;

namespace Plasma
{
    class Svg;
    class FrameSvg;
}

// The (singleton) widget that makes the desktop gray.
class KSMShutdownFeedback : public QWidget
{
    Q_OBJECT

public:
    static void start();
    static void stop();
    static void logoutCanceled();

protected:
    ~KSMShutdownFeedback() {}

    virtual void paintEvent( QPaintEvent* );

private Q_SLOTS:
    void slotPaintEffect();
    void slotPaintEffectInitialized();

private:
    static KSMShutdownFeedback * s_pSelf;
    KSMShutdownFeedback();
    int m_currentY;
    QPixmap m_pixmap;
    LogoutEffect *effect;
    bool initialized;
};

class QDeclarativeView;

// The confirmation dialog
class KSMShutdownDlg : public QDialog
{
    Q_OBJECT

public:
    static bool confirmShutdown(
            bool maysd, bool choose, KWorkSpace::ShutdownType& sdtype, QString& bopt, const QString& theme );
    bool eventFilter( QObject* watched, QEvent* event );

public Q_SLOTS:
    void slotLogout();
    void slotHalt();
    void slotReboot();
    void slotReboot(int);
    void slotSuspend(int);
    void slotLockScreen();

protected:
    void resizeEvent(QResizeEvent *e);

private:
    KSMShutdownDlg( QWidget* parent, bool maysd, bool choose, KWorkSpace::ShutdownType sdtype, const QString& theme );
    KWorkSpace::ShutdownType m_shutdownType;
    QString m_bootOption;
    QStringList rebootOptions;
    QDeclarativeView* m_view;
};

#endif
