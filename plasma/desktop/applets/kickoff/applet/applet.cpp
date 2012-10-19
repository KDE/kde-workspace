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
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QGraphicsLinearLayout>
#include <QtDeclarative>

// KDE
#include <KAuthorized>
#include <KIcon>
#include <KDebug>
#include <KConfigDialog>
#include <KProcess>

// Plasma
#include <Plasma/Containment>
#include <Plasma/Package>
#include <Plasma/ToolTipManager>
#include <Plasma/DeclarativeWidget>

// Local
#include "ui_kickoffConfig.h"
#include "core/applicationmodel.h"
#include "core/favoritesmodel.h"
#include "core/krunnermodel.h"
#include "core/leavemodel.h"
#include "core/recentapplications.h"
#include "core/recentlyusedmodel.h"
#include "core/systemmodel.h"
#include "core/urlitemlauncher.h"

class LauncherApplet::Private
{
public:
    Private(LauncherApplet *lApplet)
        : declarativeWidget(0),
          switcher(0),
          q(lApplet),
          switchTabsOnHover(false),
          showAppsByName(false)
    {
    }

    ~Private()
    {
        delete declarativeWidget;
    }
    void createLauncher();
    void initToolTip();

    Plasma::DeclarativeWidget *declarativeWidget;

    QList<QAction*> actions;
    QAction* switcher;
    LauncherApplet *q;
    Ui::kickoffConfig ui;
    bool switchTabsOnHover;
    bool showAppsByName;
};

void LauncherApplet::Private::createLauncher()
{
    if (declarativeWidget) {
        return;
    }

    const char *uri = "org.kde.plasma.kickoff";
    qmlRegisterType<Kickoff::ApplicationModel>(uri, 0, 1, "ApplicationModel");
    qmlRegisterType<Kickoff::FavoritesModel>(uri, 0, 1, "FavoritesModel");
    qmlRegisterType<Kickoff::KRunnerModel>(uri, 0, 1, "KRunnerModel");
    qmlRegisterType<Kickoff::LeaveModel>(uri, 0, 1, "LeaveModel");
    qmlRegisterType<Kickoff::RecentlyUsedModel>(uri, 0, 1, "RecentlyUsedModel");
    qmlRegisterType<Kickoff::SystemModel>(uri, 0, 1, "SystemModel");
    qmlRegisterType<Kickoff::UrlItemLauncher>(uri, 0, 1, "Launcher");
    //for the enum
    qmlRegisterUncreatableType<LauncherApplet>(uri, 0, 1, "Kickoff", QLatin1String("Do not create objects of this type."));

    declarativeWidget = new Plasma::DeclarativeWidget(q);
    declarativeWidget->engine()->rootContext()->setContextProperty("kickoff", q);
    
    Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
    Plasma::Package package(QString(), "org.kde.kickoff", structure);
    declarativeWidget->setQmlPath(package.filePath("mainscript"));

    //QObject::connect(declarativeWidget, SIGNAL(aboutToHide()), q, SLOT(hidePopup()));
    //QObject::connect(declarativeWidget, SIGNAL(configNeedsSaving()), q, SIGNAL(configNeedsSaving()));
    //launcher->resize(launcher->sizeHint());
    //QObject::connect(launcher, SIGNAL(aboutToHide()), icon, SLOT(setUnpressed()));
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
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(true);
}

LauncherApplet::~LauncherApplet()
{
    delete d;
}

void LauncherApplet::init()
{
    if (KService::serviceByStorageId("kde4-kmenuedit.desktop") && KAuthorized::authorize("action/menuedit")) {
        QAction* menueditor = new QAction(i18n("Edit Applications..."), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    Q_ASSERT(! d->switcher);
    d->switcher = new QAction(i18n("Switch to Classic Menu Style"), this);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));

    configChanged();
    Plasma::ToolTipManager::self()->registerWidget(this);
}

void LauncherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::ImmutableConstraint) && d->switcher) {
        d->switcher->setVisible(immutability() == Plasma::Mutable);
    } else if (constraints & Plasma::LocationConstraint) {
        emit locationChanged((Location)location());
    }
}

void LauncherApplet::switchMenuStyle()
{
    if (containment()) {
        Plasma::Applet * simpleLauncher =
                            containment()->addApplet("simplelauncher", QVariantList() << true, geometry());

        //Copy all the config items to the simple launcher
        QMetaObject::invokeMethod(simpleLauncher, "saveConfigurationFromKickoff",
                                  Qt::DirectConnection, Q_ARG(KConfigGroup, config()),
                                  Q_ARG(KConfigGroup, globalConfig()));

        //Switch shortcuts with the new launcher to avoid losing it
        KShortcut currentShortcut = globalShortcut();
        setGlobalShortcut(KShortcut());
        simpleLauncher->setGlobalShortcut(currentShortcut);

        //Destroy this widget
        destroy();
    }
}

void LauncherApplet::startMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void LauncherApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    d->ui.setupUi(widget);
    parent->addPage(widget, i18nc("General configuration page", "General"), icon());

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    d->createLauncher();
    d->ui.iconButton->setIcon(popupIcon());
    d->ui.switchOnHoverCheckBox->setChecked(switchTabsOnHover());
    d->ui.appsByNameCheckBox->setChecked(showAppsByName());
    connect(d->ui.iconButton, SIGNAL(iconChanged(QString)), parent, SLOT(settingsModified()));
    connect(d->ui.switchOnHoverCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(d->ui.appsByNameCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
}

void LauncherApplet::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->clearContent(this);
        d->createLauncher();
        if (d->declarativeWidget->rootObject()) {
            d->declarativeWidget->rootObject()->setProperty("focus", true);
        }
    }
}

void LauncherApplet::toolTipAboutToShow()
{
    if (d->declarativeWidget->isVisible()) {
        Plasma::ToolTipManager::self()->clearContent(this);
    } else {
        d->initToolTip();
    }
}

void LauncherApplet::configChanged()
{
    KConfigGroup cg = config();
    setPopupIcon(cg.readEntry("icon", "start-here-kde"));
    constraintsEvent(Plasma::ImmutableConstraint);
    setShowAppsByName(cg.readEntry("ShowAppsByName", false));

    cg = globalConfig();
    setSwitchTabsOnHover(cg.readEntry("SwitchTabsOnHover", true));
}

void LauncherApplet::configAccepted()
{
    bool switchTabsOnHover = d->ui.switchOnHoverCheckBox->isChecked();
    bool showAppsByName = d->ui.appsByNameCheckBox->isChecked();

    const QString iconname = d->ui.iconButton->icon();

    // TODO: should this be moved into Launcher as well? perhaps even the config itself?
    d->createLauncher();

    KConfigGroup cg = config();
    const QString oldIcon = cg.readEntry("icon", "start-here-kde");
    if (!iconname.isEmpty() && iconname != oldIcon) {
        cg.writeEntry("icon", iconname);

        if (!iconname.isEmpty()) {
            setPopupIcon(iconname);
        }

        emit configNeedsSaving();
    }

    setSwitchTabsOnHover(switchTabsOnHover);
    setShowAppsByName(showAppsByName);
}

QList<QAction*> LauncherApplet::contextualActions()
{
    return d->actions;
}

QGraphicsWidget *LauncherApplet::graphicsWidget()
{
    d->createLauncher();
    return d->declarativeWidget;
}

void LauncherApplet::saveConfigurationFromSimpleLauncher(const KConfigGroup & configGroup, const KConfigGroup & globalConfigGroup)
{
    //Copy configuration values
    KConfigGroup cg = config();
    configGroup.copyTo(&cg);

    KConfigGroup gcg = globalConfig();
    globalConfigGroup.copyTo(&gcg);

    configChanged();
    emit configNeedsSaving();
}

bool LauncherApplet::switchTabsOnHover() const
{
    return d->switchTabsOnHover;
}

void LauncherApplet::setSwitchTabsOnHover(bool on)
{
    if (on == d->switchTabsOnHover) {
        return;
    }

    d->switchTabsOnHover = on;
    KConfigGroup cg = globalConfig();
    cg.writeEntry("SwitchTabsOnHover", on);
    emit switchTabsOnHoverChanged(on);
}


bool LauncherApplet::showAppsByName() const
{
    return d->showAppsByName;
}

void LauncherApplet::setShowAppsByName(bool on)
{
    if (on == d->showAppsByName) {
        return;
    }

    d->showAppsByName = on;
    KConfigGroup cg = config();
    cg.writeEntry("ShowAppsByName", on);
    emit showAppsByNameChanged(on);
}

LauncherApplet::Location LauncherApplet::plasmoidLocation() const
{
    return (Location)location();
}

#include "applet.moc"
