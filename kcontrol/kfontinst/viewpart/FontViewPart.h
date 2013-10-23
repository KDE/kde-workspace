#ifndef __FONT_VIEW_PART_H__
#define __FONT_VIEW_PART_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KDE/KParts/ReadOnlyPart>
#include <KDE/KParts/BrowserExtension>
#include <KDE/KSharedConfig>
#include <KDE/KUrl>
#include <QFrame>
#include <QMap>
#include "KfiConstants.h"
#include "Misc.h"
#include "FontPreview.h"
#include "Family.h"

class QPushButton;
class QLabel;
class QProcess;
class QAction;
class KAction;
class KIntNumInput;
class KTempDir;

namespace KFI
{

class BrowserExtension;
class FontInstInterface;

class CFontViewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

    public:

    CFontViewPart(QWidget *parentWidget, QObject *parent, const QList<QVariant> &args);
    virtual ~CFontViewPart();

    bool openUrl(const KUrl &url);

    protected:

    bool openFile();

    public Q_SLOTS:

    void previewStatus(bool st);
    void timeout();
    void install();
    void installlStatus();
    void dbusStatus(int pid, int status);
    void fontStat(int pid, const KFI::Family &font);
    void changeText();
    void print();
    void displayType(const QList<CFcEngine::TRange> &range);
    void showFace(int face);

    private:

    void checkInstallable();
//    void getMetaInfo(int face);

    private:

//    QMap<int, QString> itsMetaInfo;
    CFontPreview       *itsPreview;
    QPushButton        *itsInstallButton;
    QWidget            *itsFaceWidget;
    QFrame             *itsFrame;
    QLabel             *itsFaceLabel;
//                       *itsMetaLabel;
    KIntNumInput       *itsFaceSelector;
    QAction            *itsChangeTextAction;
    int                itsFace;
    KSharedConfigPtr   itsConfig;
    BrowserExtension   *itsExtension;
    QProcess           *itsProc;
//    KUrl               itsMetaUrl;
    KTempDir           *itsTempDir;
    Misc::TFont        itsFontDetails;
    FontInstInterface  *itsInterface;
    bool               itsOpening;
};

class BrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

    public:

    BrowserExtension(CFontViewPart *parent);

    void enablePrint(bool enable);

    public Q_SLOTS:

    void print();
};

}

#endif
