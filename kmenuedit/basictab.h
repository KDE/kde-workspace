/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
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

#ifndef basictab_h
#define basictab_h

#include <KTabWidget>
#include <KShortcut>
#include <KService>

class KKeySequenceWidget;
class KLineEdit;
class KIconButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class KUrlRequester;
class KService;
class KLineSpellChecking;

class MenuFolderInfo;
class MenuEntryInfo;

class BasicTab : public KTabWidget
{
    Q_OBJECT

public:
    BasicTab( QWidget *parent=0 );

    void apply();

    void updateHiddenEntry( bool show );

Q_SIGNALS:
    void changed( MenuFolderInfo * );
    void changed( MenuEntryInfo * );
    void findServiceShortcut(const KShortcut&, KService::Ptr &);

public Q_SLOTS:
    void setFolderInfo(MenuFolderInfo *folderInfo);
    void setEntryInfo(MenuEntryInfo *entryInfo);
    void slotDisableAction();
protected Q_SLOTS:
    void slotChanged();
    void launchcb_clicked();
    void systraycb_clicked();
    void termcb_clicked();
    void uidcb_clicked();
    void slotCapturedKeySequence(const QKeySequence&);
    void slotExecSelected();
    void onlyshowcb_clicked();
    void hiddenentrycb_clicked();

protected:
    /**
     * @brief Initializes the general tab.
     */
    void initGeneralTab();

    /**
     * @brief Initializes the advanced tab.
     */
    void initAdvancedTab();

    /**
     * @brief Initializes connections.
     */
    void initConnections();
    void enableWidgets(bool isDF, bool isDeleted);

protected:
    KLineEdit    *_nameEdit;
    KLineSpellChecking*_commentEdit;
    KLineSpellChecking   *_descriptionEdit;
    KKeySequenceWidget *_keyBindingEdit;
    KUrlRequester *_execEdit, *_pathEdit;
    KLineEdit    *_terminalOptionsEdit, *_userNameEdit;
    QCheckBox    *_terminalCB, *_userCB, *_launchCB, *_systrayCB, *_onlyShowInKdeCB, *_hiddenEntryCB;
    KIconButton  *_iconButton;
    QGroupBox    *_workPathGroup, *_terminalGroup, *_userGroup, *_keyBindingGroup;
    QLabel *_terminalOptionsLabel, *_userNameLabel, *_pathLabel, *_nameLabel, *_commentLabel, *_execLabel, *_keyBindingLabel;
    QLabel      *_descriptionLabel;

    MenuFolderInfo *_menuFolderInfo;
    MenuEntryInfo  *_menuEntryInfo;
};

#endif
