/*
   Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (c) 2000 Matthias Elter <elter@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <kwin.h>
#include "global.h"
#include <kprocess.h>
#include <krun.h>

#include "kcdialog.h"
#include "kcdialog.moc"
#include "kcmodule.h"
#include <kdebug.h>

KCDialog::KCDialog(KCModule *client, int b, const QString &docpath, QWidget *parent, const char *name, bool modal)
  : KDialogBase(parent, name, modal, QString::null,
                (b & KCModule::Help ? (int)Help : 0) |
                (b & KCModule::Default ? (int)Default : 0) |
                (b & KCModule::Apply ? (Ok | Apply | Cancel) : Close),
                (b & KCModule::Apply ? Ok : Close),
                true),
    DCOPObject("dialog"),
    _client(client),
    _docPath(docpath)
{
    client->reparent(this,0,QPoint(0,0),true);
    setMainWidget(client);
    connect(client, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
    if( client->changed() )
    {
        kdWarning( 1208 ) << "The KCModule \"" << client->className() <<
            "\" called setChanged( true ) in the constructor."
            " Please fix the module." << endl;
        clientChanged( true );
    }

    KCGlobal::repairAccels( topLevelWidget() );
}

void KCDialog::slotDefault()
{
    _client->defaults();
    clientChanged(true);
}

void KCDialog::slotOk()
{
    if( _clientChanged )
        _client->save();
    accept();
}

void KCDialog::clientChanged(bool state)
{
    enableButton(Apply, state);
    _clientChanged = state;
}

void KCDialog::slotApply()
{
    _client->save();
    clientChanged(false);
}

void KCDialog::slotHelp()
{
    KProcess process;
    KURL url( KURL("help:/"), _docPath.local8Bit() );

    if (url.protocol() == "help" || url.protocol() == "man" || url.protocol() == "info") {
        process << "khelpcenter"
                << url.url();
        process.start(KProcess::DontCare);
    } else {
        new KRun(url);
    }
}

void KCDialog::activate()
{
    KWin::setActiveWindow(winId());
}
