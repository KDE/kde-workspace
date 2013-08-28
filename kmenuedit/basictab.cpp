/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2008 Laurent Montel <montel@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "basictab.h"

#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>

#include <KLocale>
#include <KGlobal>
#include <KDialog>
#include <KKeySequenceWidget>
#include <KLineEdit>
#include <KIconButton>
#include <KDesktopFile>
#include <KUrlRequester>
#include <KShell>

#ifndef Q_WS_WIN
#include "khotkeys.h"
#endif

#include "klinespellchecking.h"
#include "menuinfo.h"

#include "basictab.moc"

BasicTab::BasicTab( QWidget *parent )
    : KTabWidget(parent)
{
    _menuFolderInfo = 0;
    _menuEntryInfo = 0;

    initGeneralTab();
    initAdvancedTab();
    initConnections();

#ifndef Q_WS_WIN
    if (!KHotKeys::present())
        _keyBindingGroup->hide();
#endif
    slotDisableAction();
}

void BasicTab::initGeneralTab() {
    // general tab
    QWidget *generalTab = new QWidget();
    QGridLayout *generalTabLayout = new QGridLayout(generalTab);
    generalTabLayout->setMargin( KDialog::marginHint() );
    generalTabLayout->setSpacing( KDialog::spacingHint() );
    generalTab->setAcceptDrops(false);

    // name
    _nameLabel = new QLabel(i18n("&Name:"));
    generalTabLayout->addWidget(_nameLabel, 0, 0);
    _nameEdit = new KLineEdit();
    _nameEdit->setAcceptDrops(false);
    _nameLabel->setBuddy(_nameEdit);
    generalTabLayout->addWidget(_nameEdit, 0, 1, 1, 1);

    // description
    _descriptionLabel = new QLabel(i18n("&Description:"));
    generalTabLayout->addWidget(_descriptionLabel, 1, 0);
    _descriptionEdit = new KLineSpellChecking();
    _descriptionEdit->setAcceptDrops(false);
    _descriptionLabel->setBuddy(_descriptionEdit);
    generalTabLayout->addWidget(_descriptionEdit, 1, 1, 1, 1);

    // comment
    _commentLabel = new QLabel(i18n("&Comment:"));
    generalTabLayout->addWidget(_commentLabel, 2, 0);
    _commentEdit = new KLineSpellChecking();
    _commentEdit->setAcceptDrops(false);
    _commentLabel->setBuddy(_commentEdit);
    generalTabLayout->addWidget(_commentEdit, 2, 1, 1, 2);

    // command
    _execLabel = new QLabel(i18n("Co&mmand:"));
    generalTabLayout->addWidget(_execLabel, 3, 0);
    _execEdit = new KUrlRequester();
    _execEdit->lineEdit()->setAcceptDrops(false);
    _execEdit->setWhatsThis(i18n(
                                "Following the command, you can have several place holders which will be replaced "
                                "with the actual values when the actual program is run:\n"
                                "%f - a single file name\n"
                                "%F - a list of files; use for applications that can open several local files at once\n"
                                "%u - a single URL\n"
                                "%U - a list of URLs\n"
                                "%d - the folder of the file to open\n"
                                "%D - a list of folders\n"
                                "%i - the icon\n"
                                "%m - the mini-icon\n"
                                "%c - the caption"));
    _execLabel->setBuddy(_execEdit);
    generalTabLayout->addWidget(_execEdit, 3, 1, 1, 2);

    // launch feedback
    _launchCB = new QCheckBox(i18n("Enable &launch feedback"));
    generalTabLayout->addWidget(_launchCB, 4, 0, 1, 3 );

    // systray
    _systrayCB = new QCheckBox(i18n("&Place in system tray"));
    generalTabLayout->addWidget(_systrayCB, 5, 0, 1, 3 );

    // KDE visibility
    _onlyShowInKdeCB = new QCheckBox(i18n("Only show in KDE"));
    generalTabLayout->addWidget(_onlyShowInKdeCB, 6, 0, 1, 3 );

    // hidden entry
    _hiddenEntryCB = new QCheckBox(i18n("Hidden entry"));
    _hiddenEntryCB->hide();
    generalTabLayout->addWidget(_hiddenEntryCB, 7, 0, 1, 3 );

    // icon
    _iconButton = new KIconButton();
    _iconButton->setFixedSize(56,56);
    _iconButton->setIconSize(32);
    generalTabLayout->addWidget(_iconButton, 0, 2, 2, 1);
    generalTabLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 8, 0, 1, 3);

    // add the general group
    addTab(generalTab, i18n("General"));
}

void BasicTab::initAdvancedTab() {
    // advanced tab
    QWidget *advancedTab = new QWidget();
    QVBoxLayout *advancedTabLayout = new QVBoxLayout(advancedTab);

    // work path
    _workPathGroup = new QGroupBox();
    QHBoxLayout *workPathGroupLayout = new QHBoxLayout(_workPathGroup);
    workPathGroupLayout->setSpacing(KDialog::spacingHint());
    workPathGroupLayout->setMargin(KDialog::marginHint());
    _pathLabel = new QLabel(i18n("&Work path:"));
    workPathGroupLayout->addWidget(_pathLabel);
    _pathEdit = new KUrlRequester();
    _pathEdit->setMode(KFile::Directory | KFile::LocalOnly);
    _pathEdit->lineEdit()->setAcceptDrops(false);
    _pathLabel->setBuddy(_pathEdit);
    workPathGroupLayout->addWidget(_pathEdit);
    advancedTabLayout->addWidget(_workPathGroup);

    // terminal CB
    _terminalGroup = new QGroupBox();
    QVBoxLayout *terminalGroupLayout = new QVBoxLayout(_terminalGroup);
    terminalGroupLayout->setMargin(KDialog::marginHint());
    terminalGroupLayout->setSpacing(KDialog::spacingHint());
    _terminalCB = new QCheckBox(i18n("Run in term&inal"));
    terminalGroupLayout->addWidget(_terminalCB);
    // terminal options
    QWidget *terminalOptionsGroup = new QWidget();
    QHBoxLayout *terminalOptionsGroupLayout = new QHBoxLayout(terminalOptionsGroup);
    terminalOptionsGroupLayout->setSpacing(KDialog::spacingHint());
    _terminalOptionsLabel = new QLabel(i18n("Terminal &options:"));
    terminalOptionsGroupLayout->addWidget(_terminalOptionsLabel);
    _terminalOptionsEdit = new KLineEdit();
    _terminalOptionsEdit->setAcceptDrops(false);
    _terminalOptionsEdit->setEnabled(false);
    _terminalOptionsLabel->setBuddy(_terminalOptionsEdit);
    terminalOptionsGroupLayout->addWidget(_terminalOptionsEdit);
    terminalGroupLayout->addWidget(terminalOptionsGroup);
    advancedTabLayout->addWidget(_terminalGroup);

    // user name CB
    _userGroup = new QGroupBox();
    QVBoxLayout *userGroupLayout = new QVBoxLayout(_userGroup);
    userGroupLayout->setMargin(KDialog::marginHint());
    userGroupLayout->setSpacing(KDialog::spacingHint());
    _userCB = new QCheckBox(i18n("&Run as a different user"));
    userGroupLayout->addWidget(_userCB);
    // user name
    QWidget *userNameGroup = new QWidget();
    QHBoxLayout *userNameGroupLayout = new QHBoxLayout(userNameGroup);
    userNameGroupLayout->setSpacing(KDialog::spacingHint());
    _userNameLabel = new QLabel(i18n("&Username:"));
    userNameGroupLayout->addWidget(_userNameLabel);
    _userNameEdit = new KLineEdit();
    _userNameEdit->setAcceptDrops(false);
    _userNameEdit->setEnabled(false);
    _userNameLabel->setBuddy(_userNameEdit);
    userNameGroupLayout->addWidget(_userNameEdit);
    userGroupLayout->addWidget(userNameGroup);
    advancedTabLayout->addWidget(_userGroup);

    // key binding
    _keyBindingGroup = new QGroupBox();
    QHBoxLayout *keyBindingGroupLayout = new QHBoxLayout(_keyBindingGroup);
    keyBindingGroupLayout->setMargin(KDialog::marginHint() );
    keyBindingGroupLayout->setSpacing(KDialog::spacingHint());
    _keyBindingLabel = new QLabel(i18n("Current shortcut &key:"));
    keyBindingGroupLayout->addWidget(_keyBindingLabel);
    _keyBindingEdit = new KKeySequenceWidget();
    _keyBindingEdit->setMultiKeyShortcutsAllowed(false);
    _keyBindingLabel->setBuddy(_keyBindingEdit);
    keyBindingGroupLayout->addWidget(_keyBindingEdit);
    advancedTabLayout->addWidget(_keyBindingGroup);

    // push components to the top
    advancedTabLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    // add the general group
    addTab(advancedTab, i18n("Advanced"));
}

void BasicTab::initConnections() {
    // general tab's components
    connect(_nameEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_descriptionEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_commentEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_execEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_execEdit, SIGNAL(urlSelected(KUrl)), SLOT(slotExecSelected()));
    connect(_launchCB, SIGNAL(clicked()), SLOT(launchcb_clicked()));
    connect(_systrayCB, SIGNAL(clicked()), SLOT(systraycb_clicked()));
    connect(_onlyShowInKdeCB, SIGNAL(clicked()), SLOT(onlyshowcb_clicked()));
    connect(_hiddenEntryCB, SIGNAL(clicked()), SLOT(hiddenentrycb_clicked()));
    connect(_iconButton, SIGNAL(iconChanged(QString)), SLOT(slotChanged()));

    // advanced tab's components
    connect(_pathEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_terminalCB, SIGNAL(clicked()), SLOT(termcb_clicked()));
    connect(_terminalOptionsEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_userCB, SIGNAL(clicked()), SLOT(uidcb_clicked()));
    connect(_userNameEdit, SIGNAL(textChanged(QString)), SLOT(slotChanged()));
    connect(_keyBindingEdit, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(slotCapturedKeySequence(QKeySequence)));
}

void BasicTab::slotDisableAction()
{
    //disable all group at the beginning.
    //because there is not file selected.
    _nameEdit->setEnabled(false);
    _descriptionEdit->setEnabled(false);
    _commentEdit->setEnabled(false);
    _execEdit->setEnabled(false);
    _launchCB->setEnabled(false);
    _systrayCB->setEnabled(false);
    _onlyShowInKdeCB->setEnabled( false );
    _hiddenEntryCB->setEnabled( false );
    _nameLabel->setEnabled(false);
    _descriptionLabel->setEnabled(false);
    _commentLabel->setEnabled(false);
    _execLabel->setEnabled(false);
    _workPathGroup->setEnabled(false);
    _terminalGroup->setEnabled(false);
    _userGroup->setEnabled(false);
    _iconButton->setEnabled(false);
    // key binding part
    _keyBindingGroup->setEnabled( false );
}

void BasicTab::enableWidgets(bool isDF, bool isDeleted)
{
    // set only basic attributes if it is not a .desktop file
    _nameEdit->setEnabled(!isDeleted);
    _descriptionEdit->setEnabled(!isDeleted);
    _commentEdit->setEnabled(!isDeleted);
    _iconButton->setEnabled(!isDeleted);
    _execEdit->setEnabled(isDF && !isDeleted);
    _launchCB->setEnabled(isDF && !isDeleted);
    _systrayCB->setEnabled(isDF && !isDeleted);
    _onlyShowInKdeCB->setEnabled( isDF && !isDeleted );
    _hiddenEntryCB->setEnabled( isDF && !isDeleted );
    _nameLabel->setEnabled(!isDeleted);
    _descriptionLabel->setEnabled(!isDeleted);
    _commentLabel->setEnabled(!isDeleted);
    _execLabel->setEnabled(isDF && !isDeleted);

    _workPathGroup->setEnabled(isDF && !isDeleted);
    _terminalGroup->setEnabled(isDF && !isDeleted);
    _userGroup->setEnabled(isDF && !isDeleted);
    _keyBindingGroup->setEnabled( isDF && !isDeleted );

    _terminalOptionsEdit->setEnabled(isDF && !isDeleted && _terminalCB->isChecked());
    _terminalOptionsLabel->setEnabled(isDF && !isDeleted && _terminalCB->isChecked());

    _userNameEdit->setEnabled(isDF && !isDeleted && _userCB->isChecked());
    _userNameLabel->setEnabled(isDF && !isDeleted && _userCB->isChecked());
}

void BasicTab::setFolderInfo(MenuFolderInfo *folderInfo)
{
    blockSignals(true);
    _menuFolderInfo = folderInfo;
    _menuEntryInfo = 0;

    _nameEdit->setText(folderInfo->caption);
    _descriptionEdit->setText(folderInfo->genericname);
    _descriptionEdit->setCursorPosition(0);
    _commentEdit->setText(folderInfo->comment);
    _commentEdit->setCursorPosition(0);
    _iconButton->setIcon(folderInfo->icon);

    // clean all disabled fields and return
    _execEdit->lineEdit()->clear();
    _pathEdit->lineEdit()->clear();
    _terminalOptionsEdit->clear();
    _userNameEdit->clear();
    _launchCB->setChecked(false);
    _systrayCB->setChecked(false);
    _terminalCB->setChecked(false);
    _onlyShowInKdeCB->setChecked( false );
    _hiddenEntryCB->setChecked( false );
    _userCB->setChecked(false);
    _keyBindingEdit->clearKeySequence();

    enableWidgets(false, folderInfo->hidden);
    blockSignals(false);
}

void BasicTab::setEntryInfo(MenuEntryInfo *entryInfo)
{
    blockSignals(true);
    _menuFolderInfo = 0;
    _menuEntryInfo = entryInfo;

    if (!entryInfo)
    {
       _nameEdit->clear();
       _descriptionEdit->clear();
       _commentEdit->clear();
       _iconButton->setIcon( QString() );

       // key binding part
       _keyBindingEdit->clearKeySequence();

       _execEdit->lineEdit()->clear();
       _systrayCB->setChecked(false);
       _onlyShowInKdeCB->setChecked( false );
       _hiddenEntryCB->setChecked( false );

       _pathEdit->lineEdit()->clear();
       _terminalOptionsEdit->clear();
       _userNameEdit->clear();

       _launchCB->setChecked(false);
       _terminalCB->setChecked(false);
       _userCB->setChecked(false);
       enableWidgets(true, true);
       blockSignals(false);
       return;
    }

    KDesktopFile *df = entryInfo->desktopFile();

    _nameEdit->setText(df->readName());
    _descriptionEdit->setText(df->readGenericName());
    _descriptionEdit->setCursorPosition(0);
    _commentEdit->setText(df->readComment());
    _commentEdit->setCursorPosition(0);
    _iconButton->setIcon(df->readIcon());

    // key binding part
#ifndef Q_WS_WIN
    if( KHotKeys::present())
    {
        if ( !entryInfo->shortcut().isEmpty() )
            _keyBindingEdit->setKeySequence( entryInfo->shortcut().primary() );
        else
            _keyBindingEdit->clearKeySequence();
    }
#endif
    QString temp = df->desktopGroup().readEntry("Exec");
    if (temp.startsWith(QLatin1String("ksystraycmd ")))
    {
      _execEdit->lineEdit()->setText(temp.right(temp.length()-12));
      _systrayCB->setChecked(true);
    }
    else
    {
      _execEdit->lineEdit()->setText(temp);
      _systrayCB->setChecked(false);
    }

    _pathEdit->lineEdit()->setText(df->readPath());
    _terminalOptionsEdit->setText(df->desktopGroup().readEntry("TerminalOptions"));
    _userNameEdit->setText(df->desktopGroup().readEntry("X-KDE-Username"));

    if( df->desktopGroup().hasKey( "StartupNotify" ))
        _launchCB->setChecked(df->desktopGroup().readEntry("StartupNotify", true));
    else // backwards comp.
        _launchCB->setChecked(df->desktopGroup().readEntry("X-KDE-StartupNotify", true));

    _onlyShowInKdeCB->setChecked( df->desktopGroup().readXdgListEntry("OnlyShowIn").contains( "KDE" ) ); // or maybe enable only if it contains nothing but KDE?

    if ( df->desktopGroup().hasKey( "NoDisplay" ) )
        _hiddenEntryCB->setChecked( df->desktopGroup().readEntry( "NoDisplay", true ) );
    else
        _hiddenEntryCB->setChecked( false );

    if(df->desktopGroup().readEntry("Terminal", 0) == 1)
        _terminalCB->setChecked(true);
    else
        _terminalCB->setChecked(false);

    _userCB->setChecked(df->desktopGroup().readEntry("X-KDE-SubstituteUID", false));

    enableWidgets(true, entryInfo->hidden);
    blockSignals(false);
}

void BasicTab::apply()
{
    if (_menuEntryInfo)
    {
        _menuEntryInfo->setDirty();
        _menuEntryInfo->setCaption(_nameEdit->text());
        _menuEntryInfo->setDescription(_descriptionEdit->text());
        _menuEntryInfo->setIcon(_iconButton->icon());

        KDesktopFile *df = _menuEntryInfo->desktopFile();
        KConfigGroup dg = df->desktopGroup();
        dg.writeEntry("Comment", _commentEdit->text());
        if (_systrayCB->isChecked())
          dg.writeEntry("Exec", _execEdit->lineEdit()->text().prepend("ksystraycmd "));
        else
          dg.writeEntry("Exec", _execEdit->lineEdit()->text());

        dg.writePathEntry("Path", _pathEdit->lineEdit()->text());

        if (_terminalCB->isChecked())
            dg.writeEntry("Terminal", 1);
        else
            dg.writeEntry("Terminal", 0);

        dg.writeEntry("TerminalOptions", _terminalOptionsEdit->text());
        dg.writeEntry("X-KDE-SubstituteUID", _userCB->isChecked());
        dg.writeEntry("X-KDE-Username", _userNameEdit->text());
        dg.writeEntry("StartupNotify", _launchCB->isChecked());
        dg.writeEntry( "NoDisplay", _hiddenEntryCB->isChecked() );

        QStringList onlyShowIn = df->desktopGroup().readXdgListEntry("OnlyShowIn");
        /* the exact semantics of this checkbox are unclear if there is more than just KDE in the list...
         * For example: - The list is "Gnome;", the user enables "Only show in KDE" - should we remove Gnome?
         *              - The list is "Gnome;KDE;", the user unchecks the box - should we keep Gnome?
         */
        if ( _onlyShowInKdeCB->isChecked() && !onlyShowIn.contains("KDE"))
            onlyShowIn << "KDE";
        else if ( !_onlyShowInKdeCB->isChecked() && onlyShowIn.contains("KDE"))
            onlyShowIn.removeAll("KDE");
        if (onlyShowIn.isEmpty())
            dg.deleteEntry("OnlyShowIn");
        else
            dg.writeXdgListEntry("OnlyShowIn", onlyShowIn);
    }
    else
    {
        _menuFolderInfo->setCaption(_nameEdit->text());
        _menuFolderInfo->setGenericName(_descriptionEdit->text());
        _menuFolderInfo->setComment(_commentEdit->text());
        _menuFolderInfo->setIcon(_iconButton->icon());
    }
}

void BasicTab::slotChanged()
{
    if (signalsBlocked())
       return;
    apply();
    if (_menuEntryInfo)
       emit changed( _menuEntryInfo );
    else
       emit changed( _menuFolderInfo );
}

void BasicTab::launchcb_clicked()
{
    slotChanged();
}

void BasicTab::systraycb_clicked()
{
    slotChanged();
}

void BasicTab::onlyshowcb_clicked()
{
    slotChanged();
}

void BasicTab::hiddenentrycb_clicked()
{
    slotChanged();
}

void BasicTab::termcb_clicked()
{
    _terminalOptionsEdit->setEnabled(_terminalCB->isChecked());
    _terminalOptionsLabel->setEnabled(_terminalCB->isChecked());
    slotChanged();
}

void BasicTab::uidcb_clicked()
{
    _userNameEdit->setEnabled(_userCB->isChecked());
    _userNameLabel->setEnabled(_userCB->isChecked());
    slotChanged();
}

void BasicTab::slotExecSelected()
{
    QString path = _execEdit->lineEdit()->text();
    if (!path.startsWith('\''))
        _execEdit->lineEdit()->setText(KShell::quoteArg(path));
}

void BasicTab::slotCapturedKeySequence(const QKeySequence& seq)
{
    if (signalsBlocked())
       return;
    KShortcut cut(seq, QKeySequence());
#ifndef Q_WS_WIN
    if (_menuEntryInfo->isShortcutAvailable( cut ) && KHotKeys::present() )
    {
       _menuEntryInfo->setShortcut( cut );
    }
    else
    {
       // We will not assign the shortcut so reset the visible key sequence
       _keyBindingEdit->setKeySequence(QKeySequence());
    }
#endif
    if (_menuEntryInfo)
       emit changed( _menuEntryInfo );
}


void BasicTab::updateHiddenEntry( bool show )
{
    if ( show )
        _hiddenEntryCB->show();
    else
        _hiddenEntryCB->hide();
}
