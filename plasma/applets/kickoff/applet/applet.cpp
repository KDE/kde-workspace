/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// Own
#include "applet/applet.h"

// Qt
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsView>
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QGraphicsLinearLayout>

// KDE
#include <KIcon>
#include <KIconButton>
#include <KDebug>
#include <KConfigDialog>
#include <KProcess>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Containment>
#include <Plasma/View>
#include <Plasma/ToolTipManager>

// Local
#include "ui/launcher.h"
#include "core/recentapplications.h"

class LauncherApplet::Private
{
public:
    Private(LauncherApplet *lApplet) : launcher(0), switcher(0), q(lApplet) { }
    ~Private() {
        delete launcher;
    }
    void createLauncher();
    void initToolTip();

    Kickoff::Launcher *launcher;

    KIconButton *iconButton;
    QCheckBox *switchOnHoverCheckBox;
    QList<QAction*> actions;
    QAction* switcher;
    LauncherApplet *q;
};

void LauncherApplet::Private::createLauncher()
{
    if (launcher) {
        return;
    }

    launcher = new Kickoff::Launcher(q);
    launcher->setAttribute(Qt::WA_NoSystemBackground);
    launcher->setAutoHide(true);
    QObject::connect(launcher, SIGNAL(aboutToHide()), q, SLOT(hidePopup()));
    //launcher->resize(launcher->sizeHint());
    //QObject::connect(launcher, SIGNAL(aboutToHide()), icon, SLOT(setUnpressed()));
    //QObject::connect(launcher, SIGNAL(configNeedsSaving()), q, SIGNAL(configNeedsSaving()));
}

void LauncherApplet::Private::initToolTip()
{
    Plasma::ToolTipContent data(i18n("Kickoff Application Launcher"),
                                i18n("Favorites, applications, computer places, "
                                     "recently used items and desktop sessions"),
                                q->popupIcon().pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(q, data);
}

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
        : Plasma::PopupApplet(parent, args),
        d(new Private(this))
{
    KGlobal::locale()->insertCatalog("plasma_applet_launcher");
    setHasConfigurationInterface(true);
}

LauncherApplet::~LauncherApplet()
{
    delete d;
}

void LauncherApplet::init()
{
    KConfigGroup cg = globalConfig();
    setPopupIcon(cg.readEntry("icon", "start-here-kde"));

    if (KService::serviceByStorageId("kde4-kmenuedit.desktop")) {
        QAction* menueditor = new QAction(i18n("Menu Editor"), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    Q_ASSERT(! d->switcher);
    d->switcher = new QAction(i18n("Switch to Classic Menu Style"), this);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));

    constraintsEvent(Plasma::ImmutableConstraint);
    Plasma::ToolTipManager::self()->registerWidget(this);
}

void LauncherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::ImmutableConstraint) && d->switcher) {
        d->switcher->setVisible(immutability() == Plasma::Mutable);
    }
}

void LauncherApplet::switchMenuStyle()
{
    if (containment()) {
        containment()->addApplet("simplelauncher", QVariantList(), geometry());
        destroy();
    }
}

void LauncherApplet::startMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void LauncherApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget(parent);
    QGridLayout *widgetLayout = new QGridLayout(widget);
    widget->setLayout(widgetLayout);

    QLabel *iconLabel = new QLabel(i18n("Icon:"), widget);
    widgetLayout->addWidget(iconLabel, 0, 0, Qt::AlignRight);
    d->iconButton = new KIconButton(widget);
    d->iconButton->setIcon(popupIcon());
    iconLabel->setBuddy(d->iconButton);
    widgetLayout->addWidget(d->iconButton, 0, 1);

    d->switchOnHoverCheckBox = new QCheckBox(i18n("Switch tabs on hover"), widget);
    widgetLayout->addWidget(d->switchOnHoverCheckBox, 1, 0);

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, i18n("General"), icon());

    d->createLauncher();
    d->switchOnHoverCheckBox->setChecked(d->launcher->switchTabsOnHover());
}

void LauncherApplet::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->clearContent(this);
        d->launcher->setLauncherOrigin(popupPlacement(), location());
        d->createLauncher();
    }
}

void LauncherApplet::toolTipAboutToShow()
{
    if (d->launcher->isVisible()) {
        Plasma::ToolTipManager::self()->clearContent(this);
    } else {
        d->initToolTip();
    }
}

void LauncherApplet::configAccepted()
{
    bool switchTabsOnHover = d->switchOnHoverCheckBox->isChecked();

    const QString iconname = d->iconButton->icon();

    // TODO: should this be moved into Launcher as well? perhaps even the config itself?
    KConfigGroup cg = globalConfig();
    cg.writeEntry("SwitchTabsOnHover", switchTabsOnHover);
    if (!iconname.isEmpty()) {
        cg.writeEntry("icon", iconname);
    }
    emit configNeedsSaving();

    d->createLauncher();
    d->launcher->setSwitchTabsOnHover(switchTabsOnHover);
    if (!iconname.isEmpty()) {
        setPopupIcon(iconname);
        d->initToolTip();
    }
}

QList<QAction*> LauncherApplet::contextualActions()
{
    return d->actions;
}

QWidget *LauncherApplet::widget()
{
    d->createLauncher();
    return d->launcher;
}

#include "applet.moc"
