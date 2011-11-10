/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009, 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
// own
#include "layoutconfig.h"
#include "thumbnailitem.h"
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtGui/QGraphicsObject>
#include <kdeclarative.h>
#include <KDE/KDesktopFile>
#include <KDE/KGlobal>
#include <KDE/KIcon>
#include <KDE/KIconEffect>
#include <KDE/KIconLoader>
#include <KDE/KLocalizedString>
#include <KDE/KService>
#include <KDE/KStandardDirs>

namespace KWin
{
namespace TabBox
{

LayoutConfig::LayoutConfig(QWidget* parent)
    : QDeclarativeView(parent)
    , m_layoutsModels(new LayoutModel(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);
    setMinimumSize(QSize(500, 500));
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    foreach (const QString &importPath, KGlobal::dirs()->findDirs("module", "imports")) {
        engine()->addImportPath(importPath);
    }
    foreach (const QString &importPath, KGlobal::dirs()->findDirs("data", "kwin/tabbox")) {
        engine()->addImportPath(importPath);
    }
    ExampleClientModel *model = new ExampleClientModel(this);
    engine()->addImageProvider(QLatin1String("client"), new TabBoxImageProvider(model));
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.initialize();
    kdeclarative.setupBindings();
    qmlRegisterType<ThumbnailItem>("org.kde.kwin", 0, 1, "ThumbnailItem");
    rootContext()->setContextProperty("clientModel", model);
    rootContext()->setContextProperty("layoutModel", m_layoutsModels);
    setSource(KStandardDirs::locate("data", "kwin/kcm_kwintabbox/main.qml"));
}

LayoutConfig::~LayoutConfig()
{
}

void LayoutConfig::setLayout(const QString &layoutName)
{
    const QModelIndex index = m_layoutsModels->indexForLayoutName(layoutName);
    const int row = (index.isValid()) ? index.row() : -1;
    if (QObject *item = rootObject()->findChild<QObject*>("view")) {
        item->setProperty("currentIndex", row);
    }
}

QString LayoutConfig::selectedLayout() const
{
    int row = 0;
    if (QObject *item = rootObject()->findChild<QObject*>("view")) {
        row = item->property("currentIndex").toInt();
    }
    const QModelIndex index = m_layoutsModels->index(row);
    if (!index.isValid()) {
        return QString();
    }
    return m_layoutsModels->data(index, Qt::UserRole+2).toString();
}

TabBoxImageProvider::TabBoxImageProvider(QAbstractListModel* model)
    : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
    , m_model(model)
{
}

QPixmap TabBoxImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    bool ok = false;
    QStringList parts = id.split('/');
    const int index = parts.first().toInt(&ok);
    if (!ok) {
        return QDeclarativeImageProvider::requestPixmap(id, size, requestedSize);
    }
    QSize s(32, 32);
    if (requestedSize.isValid()) {
        s = requestedSize;
    }
    *size = s;
    QPixmap icon(KIcon(m_model->data(m_model->index(index), Qt::UserRole+3).toString()).pixmap(s));
    if (parts.size() > 2) {
        KIconEffect *effect = KIconLoader::global()->iconEffect();
        KIconLoader::States state = KIconLoader::DefaultState;
        if (parts.at(2) == QLatin1String("selected")) {
            state = KIconLoader::ActiveState;
        } else if (parts.at(2) == QLatin1String("disabled")) {
            state = KIconLoader::DisabledState;
        }
        icon = effect->apply(icon, KIconLoader::Desktop, state);
    }
    return icon;
}

ExampleClientModel::ExampleClientModel (QObject* parent)
    : QAbstractListModel (parent)
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "caption";
    roles[Qt::UserRole+1] = "minimized";
    roles[Qt::UserRole+2] = "desktopName";
    roles[Qt::UserRole+4] = "windowId";
    setRoleNames(roles);
    init();
}

ExampleClientModel::~ExampleClientModel()
{
}

void ExampleClientModel::init()
{
    QList<QString> applications;
    applications << "konqbrowser" << "KMail2" << "systemsettings" << "dolphin";

    foreach (const QString& application, applications) {
        KService::Ptr service = KService::serviceByStorageId("kde4-" + application + ".desktop");
        if (service) {
            m_nameList << service->entryPath();
        }
    }
}

QVariant ExampleClientModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::UserRole:
        return KDesktopFile(m_nameList.at(index.row())).readName();
    case Qt::UserRole+1:
        return false;
    case Qt::UserRole+2:
        return i18nc("An example Desktop Name", "Desktop 1");
    case Qt::UserRole+3:
        return KDesktopFile(m_nameList.at(index.row())).readIcon();
    case Qt::UserRole+4:
        const QString desktopFile = KDesktopFile(m_nameList.at(index.row())).fileName().split('/').last();
        if (desktopFile == "konqbrowser.desktop") {
            return ThumbnailItem::Konqueror;
        } else if (desktopFile == "KMail2.desktop") {
            return ThumbnailItem::KMail;
        } else if (desktopFile == "systemsettings.desktop") {
            return ThumbnailItem::Systemsettings;
        } else if (desktopFile == "dolphin.desktop") {
            return ThumbnailItem::Dolphin;
        }
        return 0;
    }
    return QVariant();
}

int ExampleClientModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_nameList.size();
}

LayoutModel::LayoutModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "name";
    roles[Qt::UserRole+1] = "sourcePath";
    setRoleNames(roles);
    init();
}

LayoutModel::~LayoutModel()
{
}

void LayoutModel::init()
{
    QStringList layouts;
    layouts << "thumbnails" << "informative" << "compact" << "text" << "big_icons" << "small_icons";
    QStringList descriptions;
    descriptions << i18nc("Name for a window switcher layout showing live window thumbnails", "Thumbnails");
    descriptions << i18nc("Name for a window switcher layout showing icon, name and desktop", "Informative");
    descriptions << i18nc("Name for a window switcher layout showing only icon and name", "Compact");
    descriptions << i18nc("Name for a window switcher layout showing only the name", "Text");
    descriptions << i18nc("Name for a window switcher layout showing large icons", "Large Icons");
    descriptions << i18nc("Name for a window switcher layout showing small icons", "Small Icons");

    for (int i=0; i<layouts.size(); ++i) {
        const QString path = KStandardDirs::locate("data", "kwin/tabbox/" + layouts.at(i) + ".qml");
        if (!path.isNull()) {
            m_nameList << descriptions.at(i);
            m_pathList << path;
            m_layoutList << layouts.at(i);
        }
    }
}

QVariant LayoutModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::UserRole:
        return m_nameList.at(index.row());
    case Qt::UserRole + 1:
        return m_pathList.at(index.row());
    case Qt::UserRole + 2:
        return m_layoutList.at(index.row());
    }
    return QVariant();
}

int LayoutModel::rowCount (const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_nameList.size();
}

QModelIndex LayoutModel::indexForLayoutName(const QString &name) const
{
    // fallback for default
    if (name == "Default" || name.isEmpty()) {
        return index(0);
    }
    for (int i=0; i<m_layoutList.size(); ++i) {
        if (name.toLower().replace(' ', '_') == m_layoutList.at(i)) {
            return index(i);
        }
    }
    return QModelIndex();
}

} // namespace KWin
} // namespace TabBox

