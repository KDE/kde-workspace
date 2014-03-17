/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "hotkeys_widget_base.h"

#include "action_data/action_data_group.h"

#include <KDebug>


HotkeysWidgetBase::HotkeysWidgetBase( QWidget *parent )
    : HotkeysWidgetIFace(parent)
    {
    ui.setupUi( this );

    connect(
        ui.comment, SIGNAL(textChanged()),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.comment, "comment" );
    }


HotkeysWidgetBase::~HotkeysWidgetBase()
    {
    }


void HotkeysWidgetBase::apply()
    {
    HotkeysWidgetIFace::apply();
    emit changed(_data);
    }


void HotkeysWidgetBase::extend(QWidget *w, const QString &label)
    {
    ui.tabs->addTab(w, label);
    }


bool HotkeysWidgetBase::isChanged() const
    {
    return _data->comment() != ui.comment->toPlainText();
    }


void HotkeysWidgetBase::doCopyFromObject()
    {
    ui.comment->setText( _data->comment() );
    }


void HotkeysWidgetBase::doCopyToObject()
    {
    _data->set_comment( ui.comment->toPlainText() );
    }


#include "moc_hotkeys_widget_base.cpp"
