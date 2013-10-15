/*
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

#include "preferencesdlg.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QGroupBox>

#include <KLocalizedString>
#include <KConfigGroup>
#include <sonnet/configwidget.h>
#include "configurationmanager.h"

PreferencesDialog::PreferencesDialog( QWidget *parent )
    : KPageDialog( parent )
{
    setFaceType( List );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );

    m_pageMisc = new MiscPage( this );
    KPageWidgetItem *page = new KPageWidgetItem( m_pageMisc , i18n( "General options" ) );
    page->setIcon( QIcon::fromTheme( "kmenuedit" ) );
    addPage(page);

    m_pageSpellChecking = new SpellCheckingPage( this );
    page = new KPageWidgetItem( m_pageSpellChecking , i18n( "Spell Checking" ) );
    page->setHeader( i18n( "Spell checking Options" ) );
    page->setIcon( QIcon::fromTheme( "tools-check-spelling" ) );
    addPage(page);

    connect( this, SIGNAL(okClicked()), this, SLOT(slotSave()) );
}

void PreferencesDialog::slotSave()
{
    m_pageSpellChecking->saveOptions();
    m_pageMisc->saveOptions();
}

SpellCheckingPage::SpellCheckingPage( QWidget *parent )
    : QWidget( parent )
{
    QHBoxLayout *lay = new QHBoxLayout( this );
    m_confPage = new Sonnet::ConfigWidget( this );
    lay->addWidget( m_confPage );
    setLayout( lay );
}

void SpellCheckingPage::saveOptions()
{
    m_confPage->save();
}

MiscPage::MiscPage( QWidget *parent )
    : QWidget( parent )
{
    // general group
    QGroupBox *generalGroup = new QGroupBox(i18n("General"));
    QVBoxLayout *generalGroupLayout = new QVBoxLayout(generalGroup);
    m_showHiddenEntries = new QCheckBox(i18n("Show hidden entries"));
    generalGroupLayout->addWidget(m_showHiddenEntries);

    // add groups
    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->addWidget(generalGroup);
    pageLayout->addStretch();

    // update displayed config
    m_showHiddenEntries->setChecked(ConfigurationManager::getInstance()->hiddenEntriesVisible());
}

void MiscPage::saveOptions()
{
    ConfigurationManager::getInstance()->setHiddenEntriesVisible(m_showHiddenEntries->isChecked());
}

#include "preferencesdlg.moc"
