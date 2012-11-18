/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "advanceddialog.h"

#include <QCheckBox>
#include <QVBoxLayout>

#include <KLocale>

AdvancedDialog::AdvancedDialog( QWidget *parent, bool status )
    :KDialog( parent )
{
    QWidget *w = new QWidget( this );
    setButtons( Cancel|Ok );
    QVBoxLayout *lay= new QVBoxLayout;
    w->setLayout( lay );
    m_onlyInKde = new QCheckBox( i18n( "Autostart only in KDE" ), w );
    m_onlyInKde->setChecked( status );
    lay->addWidget( m_onlyInKde );
    setMainWidget( w );
}

AdvancedDialog::~AdvancedDialog()
{
}

bool AdvancedDialog::onlyInKde() const
{
    return m_onlyInKde->isChecked();
}


#include "advanceddialog.moc"

