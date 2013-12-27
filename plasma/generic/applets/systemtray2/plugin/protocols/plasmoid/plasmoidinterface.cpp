/*
 *   Copyright 2008-2013 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010-2013 Marco Martin <mart@kde.org>
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasmoidinterface.h"
#include "debug.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQmlComponent>
#include <QSignalMapper>
#include <QTimer>

#include <KActionCollection>
#include <QLoggingCategory>
#include <KGlobalSettings>
#include <KService>
#include <KServiceTypeTrader>
#include <KLocalizedString>

#include <Plasma/Plasma>
#include <Plasma/PackageStructure>
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include <kdeclarative/configpropertymap.h>
#include <kdeclarative/qmlobject.h>

#include <PlasmaQuick/PackageUrlInterceptor>

Q_DECLARE_METATYPE(PlasmoidInterface*)

#define DEFAULTITEMSIZE 22

PlasmoidInterface::PlasmoidInterface(const QString &plugin, const QString &systrayPackageRoot,  QQuickItem *parent)
    : QQuickItem(parent),
//       m_actionSignals(0),
      m_backgroundHints(Plasma::Types::StandardBackground),
      m_status(Plasma::Types::ActiveStatus),
      m_qmlObject(0),
      m_defaultRepresentation(0),
      m_busy(false),
      m_expanded(false),
      m_plugin(plugin),
      m_systrayPackageRoot(systrayPackageRoot),
      m_pluginInfo(KPluginInfo()),
      m_isUserConfiguring(false)
{
    qmlRegisterType<PlasmoidInterface>();
//     qmlRegisterType<QAction>();

//     connect(this, &PlasmoidInterface::configNeedsSaving,
//             applet(), &Plasma::Applet::configNeedsSaving);
//     connect(applet(), &Plasma::Applet::immutabilityChanged,
//             this, &PlasmoidInterface::immutableChanged);
//     connect(applet(), &Plasma::Applet::userConfiguringChanged,
//             this, &PlasmoidInterface::userConfiguringChanged);
//
//     connect(applet(), &Plasma::Applet::statusChanged,
//             this, &PlasmoidInterface::statusChanged);
//
//     connect(m_appletScriptEngine, &DeclarativeAppletScript::formFactorChanged,
//             this, &PlasmoidInterface::formFactorChanged);
//     connect(m_appletScriptEngine, &DeclarativeAppletScript::locationChanged,
//             this, &PlasmoidInterface::locationChanged);
//     connect(m_appletScriptEngine, &DeclarativeAppletScript::contextChanged,
//             this, &PlasmoidInterface::contextChanged);
//
//     if (applet()->containment()) {
//         connect(applet()->containment(), &Plasma::Containment::screenChanged,
//                 this, &ContainmentInterface::screenChanged);
//     }
//
//     m_qmlObject = new QmlObject(this);
//     m_qmlObject->setInitializationDelayed(true);

    init();
}

PlasmoidInterface::~PlasmoidInterface()
{
    qCDebug(SYSTEMTRAY) << "!!! Plasmoid is gone";
}



QQuickItem* PlasmoidInterface::defaultRepresentation()
{
    if (!m_expanded) {
        return 0;
    }
    return m_defaultRepresentation;
}

KPluginInfo PlasmoidInterface::pluginInfo() const
{
    return m_pluginInfo;
}


void PlasmoidInterface::init()
{
    if (m_qmlObject && m_qmlObject->rootObject()) {
        return;
    }
    qCDebug(SYSTEMTRAY) << "INIT" << m_plugin;
    // Set up the runtime: security, url-based schemes, etc
    m_qmlObject = new QmlObject(parent());
    //qCDebug(SYSTEMTRAY) << " rootitem: " << rootItem->objectName();
    m_qmlObject->setInitializationDelayed(true);


    // Load the package
    Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");
    pkg.setPath(m_plugin);

    QQmlEngine *engine = m_qmlObject->engine();

    //Hook generic url resolution to the applet package as well
    //TODO: same thing will have to be done for every qqmlengine: PackageUrlInterceptor is material for plasmaquick?
    engine->setUrlInterceptor(new PackageUrlInterceptor(engine, pkg));

    QVariantHash initialProperties;
    initialProperties["width"] = DEFAULTITEMSIZE;
    initialProperties["height"] = DEFAULTITEMSIZE;

    const QString mainScript = pkg.filePath("mainscript");
    m_qmlObject->setSource(QUrl::fromLocalFile(mainScript));

    KPluginInfo i = pkg.metadata();
    if (!i.isValid()) {
        qCDebug(SYSTEMTRAY) << (i18n("Error: Can't find plugin metadata: %1", m_plugin));
        m_qmlObject->completeInitialization(initialProperties);
        return;
    }
    m_pluginInfo = i;

    if (!m_qmlObject->engine() || !m_qmlObject->engine()->rootContext() || !m_qmlObject->engine()->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
            qCDebug(SYSTEMTRAY) << error.toString();
        }
        reason = i18n("Error loading QML file: %1", reason);
        const QUrl appletError = QUrl::fromLocalFile(m_systrayPackageRoot+QStringLiteral("/contents/ui/CompactApplet.qml"));

        m_qmlObject->setSource(appletError);
        m_qmlObject->completeInitialization();

        //even the error message QML may fail
        if (m_qmlObject->mainComponent()->isError()) {
            return;
        } else {
            m_qmlObject->rootObject()->setProperty("reason", reason);
        }

        //m_appletScriptEngine->setLaunchErrorMessage(reason);
        qCDebug(SYSTEMTRAY) << "ERROR: " << reason;
    }

    m_qmlObject->engine()->rootContext()->setContextProperty("plasmoid", this);

    //initialize size, so an useless resize less
    //QVariantHash initialProperties;
    //initialProperties["width"] = width();
    //initialProperties["height"] = height();
    m_qmlObject->completeInitialization(initialProperties);

    //m_qmlObject->rootObject()->setParent(rootItem);
    //m_taskItem->setProperty("parent", QVariant::fromValue(rootItem));
    //qCDebug(SYSTEMTRAY) << " ST2 Parent object : " << parent()->objectName() << parent();
    qCDebug(SYSTEMTRAY) << " ST2 Plasmoidobject: " << m_qmlObject->rootObject();
    if (!m_qmlObject->rootObject()) {
        qCDebug(SYSTEMTRAY) << " PROBLEM!";
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            //reason += error.toString()+'\n';
            qCDebug(SYSTEMTRAY) << " ERROR: " << error.toString();
        }

    }

//     m_configuration = new ConfigPropertyMap(applet()->configScheme(), this);

    //use our own custom network access manager that will access Plasma packages and to manage security (i.e. deny access to remote stuff when the proper extension isn't enabled
//     QQmlEngine *engine = m_qmlObject->engine();
//     QQmlNetworkAccessManagerFactory *factory = engine->networkAccessManagerFactory();
//     engine->setNetworkAccessManagerFactory(0);
//     delete factory;
//     engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory(m_appletScriptEngine->package()));
//
//     //Hook generic url resolution to the applet package as well
//     //TODO: same thing will have to be done for every qqmlengine: PackageUrlInterceptor is material for plasmaquick?
//     engine->setUrlInterceptor(new PackageUrlInterceptor(engine, m_appletScriptEngine->package()));
//
//     m_qmlObject->setSource(QUrl::fromLocalFile(m_appletScriptEngine->mainScript()));
//
//     if (!m_qmlObject->engine() || !m_qmlObject->engine()->rootContext() || !m_qmlObject->engine()->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
//         QString reason;
//         foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
//             reason += error.toString()+'\n';
//         }
//         reason = i18n("Error loading QML file: %1", reason);
//
//         m_qmlObject->setSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("appleterror")));
//         m_qmlObject->completeInitialization();
//
//
//         //even the error message QML may fail
//         if (m_qmlObject->mainComponent()->isError()) {
//             return;
//         } else {
//             m_qmlObject->rootObject()->setProperty("reason", reason);
//         }
//
//         m_appletScriptEngine->setLaunchErrorMessage(reason);
//     }
//
//
//     m_qmlObject->engine()->rootContext()->setContextProperty("plasmoid", this);
//
//     //initialize size, so an useless resize less
//     QVariantHash initialProperties;
//     initialProperties["width"] = width();
//     initialProperties["height"] = height();
//     m_qmlObject->completeInitialization(initialProperties);
//
//     qCDebug(SYSTEMTRAY) << "Graphic object created:" << applet() << applet()->property("graphicObject");
//
//     //Create the ToolBox
//     Plasma::Containment *pc = qobject_cast<Plasma::Containment *>(applet());
//     if (pc && !qobject_cast<Plasma::Applet *>(pc->parent())) {
//         KConfigGroup defaults;
//         if (pc->containmentType() == Plasma::Types::DesktopContainment) {
//             defaults = KConfigGroup(KSharedConfig::openConfig(pc->corona()->package().filePath("defaults")), "Desktop");
//         } else if (pc->containmentType() == Plasma::Types::PanelContainment) {
//             defaults = KConfigGroup(KSharedConfig::openConfig(pc->corona()->package().filePath("defaults")), "Panel");
//         }
//
//         Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
//         if (defaults.isValid()) {
//             pkg.setPath(defaults.readEntry("ToolBox", "org.kde.desktoptoolbox"));
//         } else {
//             pkg.setPath("org.kde.desktoptoolbox");
//         }
//
//         if (pkg.isValid()) {
//             QObject *toolBoxObject = m_qmlObject->createObjectFromSource(QUrl::fromLocalFile(pkg.filePath("mainscript")));
//
//             QObject *containmentGraphicObject = m_qmlObject->rootObject();
//
//             if (containmentGraphicObject && toolBoxObject) {
//                 toolBoxObject->setProperty("parent", QVariant::fromValue(containmentGraphicObject));
//
//                 containmentGraphicObject->setProperty("toolBox", QVariant::fromValue(toolBoxObject));
//             } else {
//                 delete toolBoxObject;
//             }
//             qCDebug(SYSTEMTRAY) << "Loaded toolbox package" << pkg.path();
//         } else {
//             qWarning() << "Could not load toolbox package." << pkg.path();
//         }
//     }
//
    //set parent, both as object hyerarchy and visually
    if (m_qmlObject->rootObject()) {
        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "parent");
        QQmlProperty prop(m_qmlObject->rootObject(), "anchors.fill");
        prop.write(expr.evaluate());
    }
    geometryChanged(QRectF(), QRectF(x(), y(), width(), height()));
//     emit busyChanged();
//
//     applet()->updateConstraints(Plasma::Types::UiReadyConstraint);
    createCompactRepresentation();
}

Plasma::Types::FormFactor PlasmoidInterface::formFactor() const
{
    return Plasma::Types::Horizontal;
}

Plasma::Types::Location PlasmoidInterface::location() const
{
    return Plasma::Types::BottomEdge;
}

QString PlasmoidInterface::currentActivity() const
{
    return QString("CurrentActivity");
}

QObject* PlasmoidInterface::configuration() const
{
    return m_configuration;
}

uint PlasmoidInterface::id() const
{
    return 1337; // FIXME
}

QString PlasmoidInterface::icon() const
{
    return m_icon;
}

void PlasmoidInterface::setIcon(const QString &icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    emit iconChanged();
}

QString PlasmoidInterface::title() const
{
    return m_title;
}

void PlasmoidInterface::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

bool PlasmoidInterface::isBusy() const
{
    return !m_qmlObject->rootObject() || m_busy;
}

void PlasmoidInterface::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

bool PlasmoidInterface::isExpanded() const
{
    return m_expanded;
}

void PlasmoidInterface::setCollapsed()
{
    m_expanded = false;
}


void PlasmoidInterface::setExpanded(bool expanded)
{
    //if there is no compact representation it means it's always expanded
    //Containnments are always expanded
    if (!m_compactUiObject) {
        return;
    }
    if (m_expanded != expanded) {
        m_expanded = expanded;
        //m_defaultRepresentation = 0;
        QTimer::singleShot(500, this, SLOT(hideDefaultRepresenation()));
        qCDebug(SYSTEMTRAY) << "ST2P PI expandedItem (visible):" << m_plugin << expanded;

        emit expandedChanged();
        //emit defaultRepresentationChanged();
        //m_defaultRepresentation->setProperty("visible", expanded);
        //m_defaultRepresentation
        emit defaultRepresentationChanged();
    }


}
void PlasmoidInterface::hideDefaultRepresenation()
{
    m_defaultRepresentation->setProperty("visible", m_expanded);
}




Plasma::Types::BackgroundHints PlasmoidInterface::backgroundHints() const
{
    return m_backgroundHints;
}

void PlasmoidInterface::setBackgroundHints(Plasma::Types::BackgroundHints hint)
{
    if (m_backgroundHints == hint) {
        return;
    }

    m_backgroundHints = hint;
    emit backgroundHintsChanged();
}

void PlasmoidInterface::setConfigurationRequired(bool needsConfiguring, const QString &reason)
{
    //m_appletScriptEngine->setConfigurationRequired(needsConfiguring, reason);
}

QString PlasmoidInterface::activeConfig() const
{
    return m_currentConfig.isEmpty() ? "main"+id() : m_currentConfig;
}

void PlasmoidInterface::setActiveConfig(const QString &name)
{
    if (name == "main") {
        m_currentConfig.clear();
        return;
    }

    Plasma::ConfigLoader *loader = m_configs.value(name, 0);

    if (!loader) {
        //QString path = m_appletScriptEngine->filePath("config", name + ".xml"); // FIXME
        QString path;

        if (path.isEmpty()) {
            qCDebug(SYSTEMTRAY) << "Path is empty.";
            return;
        }

        QFile f(path);
//         KConfigGroup cg = applet()->config();
//         loader = new Plasma::ConfigLoader(&cg, &f, this);
//         m_configs.insert(name, loader);
    }

    m_currentConfig = name;
}

void PlasmoidInterface::writeConfig(const QString &entry, const QVariant &value)
{
    Plasma::ConfigLoader *config = 0;
    if (m_currentConfig.isEmpty()) {
        //config = applet()->configScheme();
    } else {
        config = m_configs.value(m_currentConfig, 0);
    }

    if (config) {
        KConfigSkeletonItem *item = config->findItemByName(entry);
        if (item) {
            item->setProperty(value);
            config->blockSignals(true);
            config->writeConfig();
            config->blockSignals(false);
            //m_appletScriptEngine->configNeedsSaving();
        }
    } else
        qWarning() << "Couldn't find a configuration entry";
}

QVariant PlasmoidInterface::readConfig(const QString &entry) const
{
    Plasma::ConfigLoader *config = 0;
    QVariant result;

    if (m_currentConfig.isEmpty()) {
        //config = applet()->configScheme();
    } else {
        config = m_configs.value(m_currentConfig, 0);
    }

    if (config) {
        result = config->property(entry);
    }

    return result;
}

// QString PlasmoidInterface::file(const QString &fileType)
// {
//     return m_appletScriptEngine->filePath(fileType, QString());
// }
//
// QString PlasmoidInterface::file(const QString &fileType, const QString &filePath)
// {
//     return m_appletScriptEngine->filePath(fileType, filePath);
// }
//
// QList<QAction*> PlasmoidInterface::contextualActions() const
// {
//     QList<QAction*> actions;
//     Plasma::Applet *a = applet();
//     if (a->failedToLaunch()) {
//         return actions;
//     }
//
//     foreach (const QString &name, m_actions) {
//         QAction *action = a->actions()->action(name);
//
//         if (action) {
//             actions << action;
//         }
//     }
//
//     return actions;
// }
//
// void PlasmoidInterface::setActionSeparator(const QString &name)
// {
//     Plasma::Applet *a = applet();
//     QAction *action = a->actions()->action(name);
//
//     if (action) {
//         action->setSeparator(true);
//     } else {
//         action = new QAction(this);
//         action->setSeparator(true);
//         a->actions()->addAction(name, action);
//         m_actions.append(name);
//     }
// }
//
// void PlasmoidInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
// {
//     Plasma::Applet *a = applet();
//     QAction *action = a->actions()->action(name);
//
//     if (action) {
//         action->setText(text);
//     } else {
//         action = new QAction(text, this);
//         a->actions()->addAction(name, action);
//
//         Q_ASSERT(!m_actions.contains(name));
//         m_actions.append(name);
//
//         if (!m_actionSignals) {
//             m_actionSignals = new QSignalMapper(this);
//             connect(m_actionSignals, SIGNAL(mapped(QString)),
//                     m_appletScriptEngine, SLOT(executeAction(QString)));
//         }
//
//         connect(action, SIGNAL(triggered()), m_actionSignals, SLOT(map()));
//         m_actionSignals->setMapping(action, name);
//     }
//
//     if (!icon.isEmpty()) {
//         action->setIcon(QIcon::fromTheme(icon));
//     }
//
//     if (!shortcut.isEmpty()) {
//         action->setShortcut(shortcut);
//     }
//
//     action->setObjectName(name);
// }
//
// void PlasmoidInterface::removeAction(const QString &name)
// {
//     Plasma::Applet *a = applet();
//     QAction *action = a->actions()->action(name);
//
//     if (action) {
//         if (m_actionSignals) {
//             m_actionSignals->removeMappings(action);
//         }
//
//         delete action;
//     }
//
//     m_actions.removeAll(name);
// }
//
// QAction *PlasmoidInterface::action(QString name) const
// {
//     return applet()->actions()->action(name);
// }

bool PlasmoidInterface::immutable() const
{
    //return m_immutability != Plasma::Types::Mutable; // FIXME
    return false;
}

bool PlasmoidInterface::userConfiguring() const
{
    return m_isUserConfiguring;
}

int PlasmoidInterface::apiVersion() const
{
    const QString constraint("[X-Plasma-API] == 'declarative' and 'Applet' in [X-Plasma-ComponentTypes]");
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
    if (offers.isEmpty()) {
        return -1;
    }

    return offers.first()->property("X-KDE-PluginInfo-Version", QVariant::Int).toInt();
}

bool PlasmoidInterface::fillWidth() const
{
    if (!m_qmlObject->rootObject()) {
        return false;
    }


    QVariant prop;

    if (m_compactUiObject) {
        prop = m_compactUiObject.data()->property("fillWidth");
    } else {
        //prop = m_qmlObject->rootObject()->property("fillWidth"); // FIXME
    }

    if (prop.isValid() && prop.canConvert<bool>()) {
        return prop.toBool();
    } else {
        return false;
    }
}

bool PlasmoidInterface::fillHeight() const
{
    if (!m_qmlObject->rootObject()) {
        return false;
    }


    QVariant prop;

    if (m_compactUiObject) {
        prop = m_compactUiObject.data()->property("fillHeight");
    } else {
        prop = m_qmlObject->rootObject()->property("fillHeight");
    }

    if (prop.isValid() && prop.canConvert<bool>()) {
        return prop.toBool();
    } else {
        return false;
    }
}

//private api, just an helper
qreal PlasmoidInterface::readGraphicsObjectSizeHint(const char *hint) const
{
    if (!m_qmlObject->rootObject()) {
        return -1;
    }


    QVariant prop;

    if (m_compactUiObject) {
        prop = m_compactUiObject.data()->property(hint);
    } else {
        prop = m_qmlObject->rootObject()->property(hint);
    }

    if (prop.isValid() && prop.canConvert<qreal>()) {
        return prop.toReal();
    } else {
        return -1;
    }
}

qreal PlasmoidInterface::minimumWidth() const
{
    return readGraphicsObjectSizeHint("minimumWidth");
}

qreal PlasmoidInterface::minimumHeight() const
{
    return readGraphicsObjectSizeHint("minimumHeight");
}

qreal PlasmoidInterface::maximumWidth() const
{
    return readGraphicsObjectSizeHint("maximumWidth");
}

qreal PlasmoidInterface::maximumHeight() const
{
    return readGraphicsObjectSizeHint("maximumHeight");
}

qreal PlasmoidInterface::implicitWidth() const
{
    return readGraphicsObjectSizeHint("implicitWidth");
}

qreal PlasmoidInterface::implicitHeight() const
{
    return readGraphicsObjectSizeHint("implicitHeight");
}

void PlasmoidInterface::setAssociatedApplication(const QString &string)
{
    //applet()->setAssociatedApplication(string);
}

QString PlasmoidInterface::associatedApplication() const
{
    return QString();
//     return applet()->associatedApplication();
}

void PlasmoidInterface::setStatus(const Plasma::Types::ItemStatus &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

Plasma::Types::ItemStatus PlasmoidInterface::status() const
{
    return m_status;
}

int PlasmoidInterface::screen() const
{
//     if (applet()->containment()) {
//         return applet()->containment()->screen();
//     }
//
    return -1;
}

// QString PlasmoidInterface::downloadPath(const QString &file)
// {
//     const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginInfo().pluginName() + '/';
//
//     if (!QFile::exists(downloadDir)) {
//         QDir dir(QChar('/'));
//         dir.mkpath(downloadDir);
//     }
//
//     return downloadDir;
// }
//
// QStringList PlasmoidInterface::downloadedFiles() const
// {
//     const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginInfo().pluginName() + '/';
//     QDir dir(downloadDir);
//     return dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
// }
//
void PlasmoidInterface::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_UNUSED(oldGeometry)

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void PlasmoidInterface::createCompactRepresentation()
{
    //init();

    //Read the minimum width of the full representation, not our own, since we could be in collapsed mode
    QSizeF minHint(-1, -1);
    if (m_qmlObject->rootObject()->property("minimumWidth").canConvert<qreal>()) {
        minHint.setWidth(m_qmlObject->rootObject()->property("minimumWidth").toReal());
    }

    if (m_qmlObject->rootObject()->property("minimumHeight").canConvert<qreal>()) {
        minHint.setHeight(m_qmlObject->rootObject()->property("minimumHeight").toReal());
    }

    //Create the icon

    m_expanded = false;

    //we are already an icon: nothing to do
    if (m_compactUiObject) {
        return;
    }

//         //Plasma::PackageStructure* structure = new Plasma::PackageStructure;
//         //Plasma::Package package(structure);
//         Plasma::Package package = Plasma::PluginLoader::self()->loadPackage("Plasma/Applet");
//         //package.setPath(path);
//         package.setPath("org.kde.plasma.systemtray");
//
//         qCDebug(SYSTEMTRAY) << "ST2 PackagePathQml: " << package.path();
//         qCDebug(SYSTEMTRAY) << "ST2 PackagePathQml mainScript: " << package.filePath("mainScript");
//
//         const QString _p = package.path();
//         if (_p.isEmpty()) {
//             // show AppletError somehow. This should of course never happen though
//         }
// /*        const QUrl compactQml = QUrl::fromLocalFile("/home/sebas/kf5/src/plasma-framework/src/shell/qmlpackages/desktop/contents/applet/CompactApplet.qml"); // FIXME
//         const QUrl defaultCompactQml = QUrl::fromLocalFile("/home/sebas/kf5/src/plasma-framework/src/shell/qmlpackages/desktop/contents/applet/DefaultCompactRepresentation.qml"); // */
//         const QUrl compactQml = QUrl::fromLocalFile(_p+"contents/ui/CompactApplet.qml");
//         const QUrl defaultCompactQml = QUrl::fromLocalFile(_p+"contents/ui/DefaultCompactRepresentation.qml");

    const QUrl compactQml = QUrl::fromLocalFile(m_systrayPackageRoot+QStringLiteral("/contents/ui/CompactApplet.qml"));
    const QUrl defaultQml = QUrl::fromLocalFile(m_systrayPackageRoot+QStringLiteral("/contents/ui/DefaultCompactRepresentation.qml"));
    m_compactUiObject = m_qmlObject->createObjectFromSource(compactQml);


    // m_compactUiObject = m_qmlObject->createObjectFromSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("compactapplet")));

    qCDebug(SYSTEMTRAY) << "ST2 compactQml: " << compactQml;
    qCDebug(SYSTEMTRAY) << "ST2 defaultcompactQml: " << defaultQml;

    QObject *compactRepresentation = 0;

    //build the icon representation
    if (m_compactUiObject) {
        QQmlComponent *compactComponent = m_qmlObject->rootObject()->property("compactRepresentation").value<QQmlComponent *>();

        if (compactComponent) {
            compactRepresentation = compactComponent->create(compactComponent->creationContext());
        } else {
            //compactRepresentation = m_qmlObject->createObjectFromSource(QUrl::fromLocalFile(applet()->containment()->corona()->package().filePath("defaultcompactrepresentation")));
            compactRepresentation = m_qmlObject->createObjectFromSource(defaultQml);
        }

        if (compactRepresentation && compactComponent) {
            compactComponent->setParent(compactRepresentation);
        } else {
            delete compactComponent;
        }
    }

    if (m_compactUiObject && compactRepresentation) {
        //put compactRepresentation in the icon place
        compactRepresentation->setProperty("parent", QVariant::fromValue(m_compactUiObject.data()));
        m_compactUiObject.data()->setProperty("compactRepresentation", QVariant::fromValue(compactRepresentation));

        //replace the full applet with the collapsed view
        m_compactUiObject.data()->setProperty("visible", true);
        m_compactUiObject.data()->setProperty("parent", QVariant::fromValue(this));

        {
            //set anchors
            QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_compactUiObject.data(), "parent");
            QQmlProperty prop(m_compactUiObject.data(), "anchors.fill");
            prop.write(expr.evaluate());
        }

        m_qmlObject->rootObject()->setProperty("parent", QVariant::fromValue(m_compactUiObject.data()));


        {
            //reset all the anchors
            QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "anchors.fill=undefined;anchors.left=undefined;anchors.right=undefined;anchors.top=undefined;anchors.bottom=undefined;");
            expr.evaluate();
        }

//             KConfigGroup cg = applet()->config();
//             cg = KConfigGroup(&cg, "PopupApplet");
//             int width = cg.readEntry("DialogWidth", 0);
//             int height = cg.readEntry("DialogHeight", 0);
//
//             m_qmlObject->rootObject()->setProperty("width", width);
//             m_qmlObject->rootObject()->setProperty("height", height);
        m_qmlObject->rootObject()->setProperty("width", 400); // FIXME: dynamic sizing
        m_qmlObject->rootObject()->setProperty("height", 300);

        m_compactUiObject.data()->setProperty("applet", QVariant::fromValue(m_qmlObject->rootObject()));

        m_defaultRepresentation = qobject_cast<QQuickItem*>(m_qmlObject->rootObject());
        m_defaultRepresentation->setProperty("visible", m_expanded);
        m_defaultRepresentation->setProperty("y", 48);

        qCDebug(SYSTEMTRAY) << "ST2P defaultRepresentation" << m_defaultRepresentation;
//             QQmlExpression expr(m_qmlObject->engine()->rootContext(), m_qmlObject->rootObject(), "y");
//             QQmlProperty prop(m_qmlObject->rootObject(), "48");
//             prop.write(expr.evaluate());
        //emit defaultRepresentationChanged();

        //hook m_compactUiObject size hints to this size hint
        //Here we have to use the old connect syntax, because we don't have access to the class type
        if (m_qmlObject->rootObject()) {
            disconnect(m_qmlObject->rootObject(), 0, this, 0);
        }

        //resize of the root object means popup resize when iconified
        connect(m_qmlObject->rootObject(), SIGNAL(widthChanged()),
                this, SLOT(updatePopupSize()));
        connect(m_qmlObject->rootObject(), SIGNAL(heightChanged()),
                this, SLOT(updatePopupSize()));

        if (m_compactUiObject.data()->property("minimumWidth").isValid()) {
            connect(m_compactUiObject.data(), SIGNAL(minimumWidthChanged()),
                    this, SIGNAL(minimumWidthChanged()));
        }
        if (m_compactUiObject.data()->property("minimumHeight").isValid()) {
            connect(m_compactUiObject.data(), SIGNAL(minimumHeightChanged()),
                    this, SIGNAL(minimumHeightChanged()));
        }

        if (m_compactUiObject.data()->property("maximumWidth").isValid()) {
            connect(m_compactUiObject.data(), SIGNAL(maximumWidthChanged()),
                    this, SIGNAL(maximumWidthChanged()));
        }
        if (m_compactUiObject.data()->property("maximumHeight").isValid()) {
            connect(m_compactUiObject.data(), SIGNAL(maximumHeightChanged()),
                    this, SIGNAL(maximumHeightChanged()));
        }

        if (m_compactUiObject.data()->property("implicitWidth").isValid()) {
            connect(m_compactUiObject.data(), SIGNAL(implicitWidthChanged()),
                    this, SIGNAL(implicitWidthChanged()));
        }
        if (m_compactUiObject.data()->property("implicitHeight").isValid()) {
            connect(m_compactUiObject.data(), SIGNAL(implicitHeightChanged()),
                    this, SIGNAL(implicitHeightChanged()));
        }

        emit fillWidthChanged();
        emit fillHeightChanged();
        emit minimumWidthChanged();
        emit minimumHeightChanged();
        emit implicitWidthChanged();
        emit implicitHeightChanged();
        emit maximumWidthChanged();
        emit maximumHeightChanged();
        emit defaultRepresentationChanged();
    }

    emit expandedChanged();
}

void PlasmoidInterface::updatePopupSize()
{
//     KConfigGroup cg = applet()->config();
//     cg = KConfigGroup(&cg, "PopupApplet");
//     cg.writeEntry("DialogWidth", m_qmlObject->rootObject()->property("width").toInt());
//     cg.writeEntry("DialogHeight", m_qmlObject->rootObject()->property("height").toInt());
}

void PlasmoidInterface::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        //we have a window: create the 
        if (value.window && !m_qmlObject->rootObject()) {
            init();
        }
    }
    QQuickItem::itemChange(change, value);
}

QmlObject *PlasmoidInterface::qmlObject()
{
    return m_qmlObject;
}

#include "moc_plasmoidinterface.cpp"
