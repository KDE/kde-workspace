/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "statusnotifiertest.h"

#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kplugininfo.h>
#include <kplugintrader.h>

#include <qcommandlineparser.h>

#include <KLocalizedString>
#include <KStatusNotifierItem>

#include <QStringList>
#include <QTimer>

#include <QPushButton>


static QTextStream cout(stdout);

class StatusNotifierTestPrivate {
public:
    QString pluginName;
    QTimer* timer;
    int interval = 1500;

    KStatusNotifierItem* systemNotifier;

};

StatusNotifierTest::StatusNotifierTest(QWidget* parent) :
    QDialog(parent)
{
    d = new StatusNotifierTestPrivate;

    init();

    setupUi(this);
    connect(updateButton, &QPushButton::clicked, this, &StatusNotifierTest::updateNotifier);
    updateUi();
    show();
}

void StatusNotifierTest::init()
{
    d->systemNotifier = new KStatusNotifierItem(this);
    //d->systemNotifier->setCategory(KStatusNotifierItem::SystemServices);
    //d->systemNotifier->setCategory(KStatusNotifierItem::Hardware);
    d->systemNotifier->setCategory(KStatusNotifierItem::Communications);
    d->systemNotifier->setIconByName("plasma");
    d->systemNotifier->setStatus(KStatusNotifierItem::Active);
    d->systemNotifier->setToolTipTitle(i18nc("tooltip title", "System Service Item"));
    d->systemNotifier->setTitle(i18nc("title", "StatusNotifierTest"));
    d->systemNotifier->setToolTipSubTitle(i18nc("tooltip subtitle", "Some explanation from the beach."));

}

StatusNotifierTest::~StatusNotifierTest()
{
    delete d;
}

void StatusNotifierTest::updateUi()
{
    if (!d->systemNotifier) {
        return;
    }
    statusActive->setChecked(d->systemNotifier->status() == KStatusNotifierItem::Active);
    statusPassive->setChecked(d->systemNotifier->status() == KStatusNotifierItem::Passive);
    statusNeedsAttention->setChecked(d->systemNotifier->status() == KStatusNotifierItem::NeedsAttention);

    statusActive->setEnabled(!statusAuto->isChecked());
    statusPassive->setEnabled(!statusAuto->isChecked());
    statusNeedsAttention->setEnabled(!statusAuto->isChecked());

    iconName->setText(d->systemNotifier->iconName());
    tooltipText->setText(d->systemNotifier->toolTipTitle());
    tooltipSubtext->setText(d->systemNotifier->toolTipSubTitle());

}

void StatusNotifierTest::updateNotifier()
{
    if (!enabledCheck->isChecked()) {
        delete d->systemNotifier;
        return;
    } else {
        init();
    }


    if (!d->systemNotifier) {
        return;
    }
    if (statusAuto->isChecked()) {
        d->timer->start();
    } else {
        d->timer->stop();
    }

    KStatusNotifierItem::ItemStatus s = KStatusNotifierItem::Passive;
    if (statusActive->isChecked()) {
        s = KStatusNotifierItem::Active;
    } else if (statusNeedsAttention->isChecked()) {
        s = KStatusNotifierItem::NeedsAttention;
    }
    d->systemNotifier->setStatus(s);

    d->systemNotifier->setIconByName(iconName->text());

    d->systemNotifier->setToolTip(iconName->text(), tooltipText->text(), tooltipSubtext->text());

    updateUi();
}



int StatusNotifierTest::runMain()
{
    d->timer = new QTimer(this);
    connect(d->timer, &QTimer::timeout, this, &StatusNotifierTest::timeout);
    d->timer->setInterval(d->interval);
    //d->timer->start();
    return 0;
}

void StatusNotifierTest::timeout()
{
    if (!d->systemNotifier) {
        return;
    }

    if (d->systemNotifier->status() == KStatusNotifierItem::Passive) {
        d->systemNotifier->setStatus(KStatusNotifierItem::Active);
        qDebug() << " Now Active";
    } else if (d->systemNotifier->status() == KStatusNotifierItem::Active) {
        d->systemNotifier->setStatus(KStatusNotifierItem::NeedsAttention);
        qDebug() << " Now NeedsAttention";
    } else if (d->systemNotifier->status() == KStatusNotifierItem::NeedsAttention) {
        d->systemNotifier->setStatus(KStatusNotifierItem::Passive);
        qDebug() << " Now passive";
    }
    updateUi();
}

#include "moc_statusnotifiertest.cpp"

