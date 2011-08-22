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

#include "FontViewPart.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "FcEngine.h"
#include "PreviewSelectAction.h"
#include "FontInstInterface.h"
#include "FontInst.h"
#include <QtGui/QGridLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QFrame>
#include <QtCore/QFile>
#include <QtGui/QLabel>
#include <QtGui/QValidator>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGroupBox>
#include <QtCore/QProcess>
#include <KDE/KLocale>
#include <KDE/KIO/NetAccess>
#include <KDE/KIO/Job>
#include <KDE/KIO/JobUiDelegate>
#include <KDE/KGlobal>
#include <KDE/KActionCollection>
#include <KDE/KComponentData>
#include <KDE/KMessageBox>
#include <KDE/KIntNumInput>
#include <KDE/KInputDialog>
#include <KDE/KDialog>
#include <KDE/KIcon>
#include <KDE/KMimeType>
//#include <KDE/KFileMetaInfo>
#include <KDE/KZip>
#include <KDE/KTempDir>
#include <KDE/KStandardDirs>
#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KStandardAction>
#include "config-fontinst.h"

// Enable the following to allow printing of non-installed fonts. Does not seem to work :-(
//#define KFI_PRINT_APP_FONTS

namespace KFI
{
static QString getFamily(const QString &font)
{
    int     commaPos=font.lastIndexOf(',');
    return -1==commaPos ? font : font.left(commaPos);
}

K_PLUGIN_FACTORY(CFontViewPartFactory, registerPlugin<CFontViewPart>();)
K_EXPORT_PLUGIN(CFontViewPartFactory("kfontview"))

CFontViewPart::CFontViewPart(QWidget *parentWidget, QObject *parent, const QList<QVariant> &)
             : KParts::ReadOnlyPart(parent),
               itsConfig(KGlobal::config()),
               itsProc(NULL),
               itsTempDir(NULL),
               itsInterface(new FontInstInterface()),
               itsOpening(false)
{
    // create browser extension (for printing when embedded into browser)
    itsExtension = new BrowserExtension(this);

    itsFrame=new QFrame(parentWidget);

    QFrame    *previewFrame=new QFrame(itsFrame);
    QWidget   *controls=new QWidget(itsFrame);
//     QGroupBox *metaBox=new QGroupBox(i18n("Information:"), controls);

    itsFaceWidget=new QWidget(controls);

    QBoxLayout *mainLayout=new QBoxLayout(QBoxLayout::TopToBottom, itsFrame);

    mainLayout->setMargin(KDialog::marginHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    QBoxLayout *previewLayout=new QBoxLayout(QBoxLayout::LeftToRight, previewFrame),
               *controlsLayout=new QBoxLayout(QBoxLayout::LeftToRight, controls),
               *faceLayout=new QBoxLayout(QBoxLayout::LeftToRight, itsFaceWidget);
//    QBoxLayout *metaLayout=new QBoxLayout(QBoxLayout::LeftToRight, metaBox);

//     itsMetaLabel=new QLabel(metaBox);
//     itsMetaLabel->setAlignment(Qt::AlignTop);
//     metaLayout->addWidget(itsMetaLabel);
    previewLayout->setMargin(0);
    previewLayout->setSpacing(0);
    faceLayout->setMargin(0);
    faceLayout->setSpacing(KDialog::spacingHint());
    controlsLayout->setMargin(0);
    previewLayout->setSpacing(0);

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(Qt::ClickFocus);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setFrameShadow(QFrame::Sunken);
    setComponentData(KComponentData(KFI_NAME));

    itsPreview=new CFontPreview(previewFrame);
    itsPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFaceLabel=new QLabel(i18n("Show Face:"), itsFaceWidget);
    itsFaceSelector=new KIntNumInput(1, itsFaceWidget);
    itsFaceSelector->setSliderEnabled(false);
    itsInstallButton=new QPushButton(i18n("Install..."), controls);
    itsInstallButton->setEnabled(false);
    previewLayout->addWidget(itsPreview);
    faceLayout->addWidget(itsFaceLabel);
    faceLayout->addWidget(itsFaceSelector);
    faceLayout->addItem(new QSpacerItem(KDialog::spacingHint(), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    itsFaceWidget->hide();

    itsPreview->engine()->readConfig(*itsConfig);

    //controlsLayout->addWidget(metaBox);
    //controlsLayout->addStretch(2);
    controlsLayout->addWidget(itsFaceWidget);
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(itsInstallButton);
    mainLayout->addWidget(previewFrame);
    mainLayout->addWidget(controls);
    connect(itsPreview, SIGNAL(status(bool)), SLOT(previewStatus(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), SLOT(install()));
    connect(itsFaceSelector, SIGNAL(valueChanged(int)), SLOT(showFace(int)));

    itsChangeTextAction=actionCollection()->addAction("changeText");
    itsChangeTextAction->setIcon(KIcon("edit-rename"));
    itsChangeTextAction->setText(i18n("Change Text..."));
    connect(itsChangeTextAction, SIGNAL(triggered(bool)), SLOT(changeText()));

    CPreviewSelectAction *displayTypeAction=new CPreviewSelectAction(this, CPreviewSelectAction::BlocksAndScripts);
    actionCollection()->addAction("displayType", displayTypeAction);
    connect(displayTypeAction, SIGNAL(range(QList<CFcEngine::TRange>)),
            SLOT(displayType(QList<CFcEngine::TRange>)));

    QAction *zoomIn=actionCollection()->addAction(KStandardAction::ZoomIn, itsPreview, SLOT(zoomIn())),
            *zoomOut=actionCollection()->addAction(KStandardAction::ZoomOut, itsPreview, SLOT(zoomOut()));

    connect(itsPreview, SIGNAL(atMax(bool)), zoomIn, SLOT(setDisabled(bool)));
    connect(itsPreview, SIGNAL(atMin(bool)), zoomOut, SLOT(setDisabled(bool)));

    setXMLFile("kfontviewpart.rc");
    setWidget(itsFrame);
    itsExtension->enablePrint(false);

    FontInst::registerTypes();
    connect(itsInterface, SIGNAL(status(int,int)), SLOT(dbusStatus(int,int)));
    connect(itsInterface, SIGNAL(fontStat(int,KFI::Family)), SLOT(fontStat(int,KFI::Family)));
}

CFontViewPart::~CFontViewPart()
{
    delete itsTempDir;
    itsTempDir=NULL;
    delete itsInterface;
    itsInterface=NULL;
}

bool CFontViewPart::openUrl(const KUrl &url)
{
    if (!url.isValid() || !closeUrl())
        return false;

//     itsMetaLabel->setText(QString());
//     itsMetaInfo.clear();

    itsFontDetails=FC::decode(url);
    if(!itsFontDetails.family.isEmpty() ||
       KFI_KIO_FONTS_PROTOCOL==url.protocol() || KIO::NetAccess::mostLocalUrl(url, itsFrame).isLocalFile())
    {
        setUrl(url);
        emit started(0);
        setLocalFilePath(this->url().path());
        bool ret=openFile();
        if (ret)
            emit completed();
        return ret;
    }
    else
        return ReadOnlyPart::openUrl(url);
}

bool CFontViewPart::openFile()
{
    // NOTE: Can't do the real open here, as we don't seem to be able to use KIO::NetAccess functions
    // during initial start-up. Bug report 111535 indicates that calling "konqueror <font>" crashes.
    itsInstallButton->setEnabled(false);
    QTimer::singleShot(0, this, SLOT(timeout()));
    return true;
}

void CFontViewPart::timeout()
{
    if(!itsInstallButton)
        return;

    bool    isFonts(KFI_KIO_FONTS_PROTOCOL==url().protocol()),
            showFs(false),
            package(false);
    int     fileIndex(-1);
    QString fontFile;

//    itsMetaUrl=url();
    delete itsTempDir;
    itsTempDir=NULL;

    itsOpening=true;

    if(!itsFontDetails.family.isEmpty())
    {
        emit setWindowCaption(FC::createName(itsFontDetails.family, itsFontDetails.styleInfo));
        fontFile=FC::getFile(url());
        fileIndex=FC::getIndex(url());
    }
    else if(isFonts)
    {
        KIO::UDSEntry udsEntry;
        bool          found=KIO::NetAccess::stat(url(), udsEntry, NULL);

        if(!found)
        {
            // Check if url is "fonts:/<font> if so try fonts:/System/<font>, then fonts:/Personal<font>
            QStringList pathList(url().path(KUrl::RemoveTrailingSlash).split('/', QString::SkipEmptyParts));

            if(pathList.count()==1)
            {
                found=KIO::NetAccess::stat(QString("fonts:/"+i18n(KFI_KIO_FONTS_SYS)+'/'+pathList[0]), udsEntry, NULL);
                if(!found)
                    found=KIO::NetAccess::stat(QString("fonts:/"+i18n(KFI_KIO_FONTS_USER)+'/'+pathList[0]), udsEntry, NULL);
            }
        }
        
        if(found)
        {
            if(udsEntry.numberValue(KIO::UDSEntry::UDS_HIDDEN, 0))
            {
                fontFile=udsEntry.stringValue(UDS_EXTRA_FILE_NAME);
                fileIndex=udsEntry.numberValue(UDS_EXTRA_FILE_FACE, 0);
            }
            itsFontDetails.family=getFamily(udsEntry.stringValue(KIO::UDSEntry::UDS_NAME));
            itsFontDetails.styleInfo=udsEntry.numberValue(UDS_EXTRA_FC_STYLE);
            emit setWindowCaption(udsEntry.stringValue(KIO::UDSEntry::UDS_NAME));
        }
        else
        {
            previewStatus(false);
            return;
        }
    }
    else
    {
        QString path(localFilePath());

        // Is this a application/vnd.kde.fontspackage file? If so, extract 1 scalable font...
        if((package=Misc::isPackage(path)))
        {
            KZip zip(path);

            if(zip.open(QIODevice::ReadOnly))
            {
                const KArchiveDirectory *zipDir=zip.directory();

                if(zipDir)
                {
                    QStringList fonts(zipDir->entries());

                    if(fonts.count())
                    {
                        QStringList::ConstIterator it(fonts.begin()),
                                                   end(fonts.end());

                        for(; it!=end; ++it)
                        {
                            const KArchiveEntry *entry=zipDir->entry(*it);

                            if(entry && entry->isFile())
                            {
                                delete itsTempDir;
                                itsTempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                                itsTempDir->setAutoRemove(true);

                                ((KArchiveFile *)entry)->copyTo(itsTempDir->name());

                                QString mime(KMimeType::findByPath(itsTempDir->name()+entry->name())->name());

                                if(mime=="application/x-font-ttf" || mime=="application/x-font-otf" ||
                                   mime=="application/x-font-type1")
                                {
                                    fontFile=itsTempDir->name()+entry->name();
                                    //setLocalFilePath(itsTempDir->name()+entry->name());
//                                    itsMetaUrl=KUrl::fromPath(localFilePath());
                                    break;
                                }
                                else
                                    ::unlink(QFile::encodeName(itsTempDir->name()+entry->name()).data());
                            }
                        }
                    }
                }
            }
        }
    }

    itsInstallButton->setEnabled(false);

    if(itsFontDetails.family.isEmpty())
        emit setWindowCaption(url().prettyUrl());
    else
        FcInitReinitialize();

    itsPreview->showFont(!package && itsFontDetails.family.isEmpty()
                            ? localFilePath()
                            : fontFile.isEmpty()
                                ? itsFontDetails.family
                                : fontFile,
                         itsFontDetails.styleInfo, fileIndex);

    if(!isFonts && itsPreview->engine()->getNumIndexes()>1)
    {
        showFs=true;
        itsFaceSelector->setRange(1, itsPreview->engine()->getNumIndexes(), 1);
        itsFaceSelector->blockSignals(true);
        itsFaceSelector->setValue(1);
        itsFaceSelector->blockSignals(false);
    }

    itsFaceWidget->setVisible(showFs);
}

void CFontViewPart::previewStatus(bool st)
{
    if(itsOpening)
    {
        bool printable(false);

        if(st)
        {
            checkInstallable();
            if(Misc::app(KFI_PRINTER).isEmpty())
                printable=false;
            if(KFI_KIO_FONTS_PROTOCOL==url().protocol())
                printable=!Misc::isHidden(url());
            else if(!FC::decode(url()).family.isEmpty())
                printable=!Misc::isHidden(FC::getFile(url()));
#ifdef KFI_PRINT_APP_FONTS
            else
            {
                // TODO: Make this work! Plus, printing of disabled TTF/OTF's should also be possible!
                KMimeType::Ptr mime=KMimeType::findByUrl(KUrl::fromPath(localFilePath()), 0, false, true);

                printable=mime->is("application/x-font-ttf") || mime->is("application/x-font-otf");
            }
#endif
        }
        itsExtension->enablePrint(st && printable);
        itsOpening=false;
    }
    itsChangeTextAction->setEnabled(st);

//     if(st)
//         getMetaInfo(itsFaceSelector->isVisible() && itsFaceSelector->value()>0
//                                           ? itsFaceSelector->value()-1 : 0);
//     else
    if(!st)
        KMessageBox::error(itsFrame, i18n("Could not read font."));
}

void CFontViewPart::install()
{
    if(!itsProc || QProcess::NotRunning==itsProc->state())
    {
        QStringList args;

        if(!itsProc)
            itsProc=new QProcess(this);
        else
            itsProc->kill();

        args << "--embed" <<  QString().sprintf("0x%x", (unsigned int)(itsFrame->window()->winId()))
             << "--caption" << KGlobal::caption().toUtf8()
             << "--icon" << "kfontview"
             << url().prettyUrl();

        connect(itsProc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(installlStatus()));
        itsProc->start(Misc::app(KFI_INSTALLER), args);
        itsInstallButton->setEnabled(false);
    }
}

void CFontViewPart::installlStatus()
{
    checkInstallable();
}

void CFontViewPart::dbusStatus(int pid, int status)
{
    if(pid==getpid() && FontInst::STATUS_OK!=status)
        itsInstallButton->setEnabled(false);
}

void CFontViewPart::fontStat(int pid, const KFI::Family &font)
{
    if(pid==getpid())
        itsInstallButton->setEnabled(!Misc::app(KFI_INSTALLER).isEmpty() && font.styles().count()==0);
}
    
void CFontViewPart::changeText()
{
    bool             status;
    QRegExpValidator validator(QRegExp(".*"), 0L);
    QString          oldStr(itsPreview->engine()->getPreviewString()),
                     newStr(KInputDialog::getText(i18n("Preview String"),
                                                  i18n("Please enter new string:"),
                                                  oldStr, &status, itsFrame, &validator));

    if(status && newStr!=oldStr)
    {
        itsPreview->engine()->setPreviewString(newStr);
        itsPreview->engine()->writeConfig(*itsConfig);
        itsPreview->showFont();
    }
}

void CFontViewPart::print()
{
    QStringList args;

    if(!itsFontDetails.family.isEmpty())
    {
        args << "--embed" << QString().sprintf("0x%x", (unsigned int)(itsFrame->window()->winId()))
             << "--caption" << KGlobal::caption().toUtf8()
             << "--icon" << "kfontview"
             << "--size" << "0"
             << "--pfont" << QString(itsFontDetails.family+','+QString().setNum(itsFontDetails.styleInfo));
    }
#ifdef KFI_PRINT_APP_FONTS
    else
        args << "--embed" << QString().sprintf("0x%x", (unsigned int)(itsFrame->window()->winId()))
             << "--caption" << KGlobal::caption().toUtf8()
             << "--icon" << "kfontview"
             << "--size " << "0"
             << localFilePath()
             << QString().setNum(KFI_NO_STYLE_INFO);
#endif

    if(args.count())
        QProcess::startDetached(Misc::app(KFI_PRINTER), args);
}

void CFontViewPart::displayType(const QList<CFcEngine::TRange> &range)
{
    itsPreview->setUnicodeRange(range);
    itsChangeTextAction->setEnabled(0==range.count());
}

void CFontViewPart::showFace(int face)
{
    itsPreview->showFace(face-1);
}

void CFontViewPart::checkInstallable()
{
    if(itsFontDetails.family.isEmpty())
    {
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(OrgKdeFontinstInterface::staticInterfaceName()))
            QProcess::startDetached(QLatin1String(KFONTINST_LIB_EXEC_DIR"/fontinst"));
        itsInstallButton->setEnabled(false);
        itsInterface->stat(itsPreview->engine()->descriptiveName(), FontInst::SYS_MASK|FontInst::USR_MASK, getpid());
    }
}

#if 0
void CFontViewPart::getMetaInfo(int face)
{
    if(itsMetaInfo[face].isEmpty())
    {
        // Pass as much inofmration as possible to analyzer...
        if(KFI_KIO_FONTS_PROTOCOL!=itsMetaUrl.protocol())
        {
            itsMetaUrl.removeQueryItem(KFI_KIO_FACE);
            if(face>0)
                itsMetaUrl.addQueryItem(KFI_KIO_FACE, QString().setNum(face));
        }

        KFileMetaInfo meta(itsMetaUrl);

        if(meta.isValid() && meta.keys().count())
        {
            QStringList           keys(meta.keys());
            QStringList::const_iterator it(keys.begin()),
                                  end(keys.end());

            itsMetaInfo[face]="<table>";
            for(; it!=end; ++it)
            {
                KFileMetaInfoItem mi(meta.item(*it));

                itsMetaInfo[face]+="<tr><td><b>"+mi.name()+"</b></td></tr><tr><td>"+
                                   mi.value().toString()+"</td></tr>";
            }

            itsMetaInfo[face]+="</table>";
            itsMetaLabel->setText(itsMetaInfo[face]);
        }
        else
            itsMetaLabel->setText(i18n("<p>No information</p>"));
    }
    else
        itsMetaLabel->setText(itsMetaInfo[face]);
}
#endif

BrowserExtension::BrowserExtension(CFontViewPart *parent)
                : KParts::BrowserExtension(parent)
{
    setURLDropHandlingEnabled(true);
}

void BrowserExtension::enablePrint(bool enable)
{
    if(enable!=isActionEnabled("print") && (!enable || !Misc::app(KFI_PRINTER).isEmpty()))
        emit enableAction("print", enable);
}

void BrowserExtension::print()
{
    if(!Misc::app(KFI_PRINTER).isEmpty())
        static_cast<CFontViewPart*>(parent())->print();
}

}

#include "FontViewPart.moc"
