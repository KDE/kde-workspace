/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>

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
#include "decorationmodel.h"
#include "preview.h"
// kwin
#include <kdecorationfactory.h>
// Qt
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QtGui/QPainter>
#include <QStyle>
#include <QtGui/QTextDocument>
// KDE
#include <KConfigGroup>
#include <KDesktopFile>
#include <KGlobalSettings>
#include <KIcon>
#include <KDE/KLocalizedString>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KPluginInfo>
#include "kwindecoration.h"

/* WARNING -------------------------------------------------------------------------
* it is *ABSOLUTELY* mandatory to manage loadPlugin() and destroyPreviousPlugin()
* using disablePreview()
*
* loadPlugin() moves the present factory pointer to "old_fact" which is then deleted
* by the succeeding destroyPreviousPlugin()
*
* So if you loaded a new plugin and that changed the current factory, call disablePreview()
* BEFORE the following destroyPreviousPlugin() destroys the factory for the current m_preview->deco->factory
* (which is invoked on deco deconstruction)
* WARNING ------------------------------------------------------------------------ */

namespace KWin
{

DecorationModel::DecorationModel(KSharedConfigPtr config, QObject* parent)
    : QAbstractListModel(parent)
    , m_plugins(new KDecorationPreviewPlugins(config))
    , m_preview(new KDecorationPreview())
    , m_customButtons(false)
    , m_leftButtons(QString())
    , m_rightButtons(QString())
{
    QHash<int, QByteArray> roleNames;
    roleNames[Qt::DisplayRole] = "display";
    roleNames[DecorationModel::PixmapRole] = "preview";
    roleNames[TypeRole] = "type";
    roleNames[AuroraeNameRole] = "auroraeThemeName";
    roleNames[QmlMainScriptRole] = "mainScript";
    roleNames[BorderSizeRole] = "borderSize";
    roleNames[ButtonSizeRole] = "buttonSize";
    setRoleNames(roleNames);
    m_config = KSharedConfig::openConfig("auroraerc");
    findDecorations();
}

DecorationModel::~DecorationModel()
{
    delete m_preview;
    delete m_plugins;
}

void DecorationModel::reload()
{
    m_decorations.clear();
    findDecorations();
}

// Find all theme desktop files in all 'data' dirs owned by kwin.
// And insert these into a DecorationInfo structure
void DecorationModel::findDecorations()
{
    beginResetModel();
    const QStringList dirList = KGlobal::dirs()->findDirs("data", "kwin");

    foreach (const QString & dir, dirList) {
        QDir d(dir);
        if (d.exists()) {
            foreach (const QFileInfo & fi, d.entryInfoList()) {
                const QString filename(fi.absoluteFilePath());
                if (KDesktopFile::isDesktopFile(filename)) {
                    const KDesktopFile desktopFile(filename);
                    const QString libName = desktopFile.desktopGroup().readEntry("X-KDE-Library");

                    if (!libName.isEmpty() && libName.startsWith(QLatin1String("kwin3_"))) {
                        if (libName == "kwin3_aurorae") {
                            // read the Aurorae themes
                            findAuroraeThemes();
                            continue;
                        }
                        if (!m_plugins->canLoad(libName))
                            continue;
                        DecorationModelData data;
                        data.name = desktopFile.readName();
                        data.libraryName = libName;
                        data.type = DecorationModelData::NativeDecoration;
                        data.borderSize = KDecorationDefines::BorderNormal;
                        data.closeDblClick = false;
                        metaData(data, desktopFile);
                        m_decorations.append(data);
                    }
                }
            }
        }
    }
    KService::List offers = KServiceTypeTrader::self()->query("KWin/Decoration");
    foreach (KService::Ptr service, offers) {
        DecorationModelData data;
        data.name = service->name();
        data.libraryName = "kwin3_aurorae";
        data.type = DecorationModelData::QmlDecoration;
        data.auroraeName = service->property("X-KDE-PluginInfo-Name").toString();
        QString scriptName = service->property("X-Plasma-MainScript").toString();
        data.qmlPath = KStandardDirs::locate("data", "kwin/decorations/" + data.auroraeName + "/contents/" + scriptName);
        if (data.qmlPath.isEmpty()) {
            // not a valid QML theme
            continue;
        }
        KConfigGroup config(m_config, data.auroraeName);
        data.borderSize = (KDecorationDefines::BorderSize)config.readEntry< int >("BorderSize", KDecorationDefines::BorderNormal);
        data.buttonSize = (KDecorationDefines::BorderSize)config.readEntry< int >("ButtonSize", KDecorationDefines::BorderNormal);
        data.closeDblClick = config.readEntry< bool >("CloseOnDoubleClickMenuButton", true);
        data.comment = service->comment();
        KPluginInfo info(service);
        data.author = info.author();
        data.email= info.email();
        data.version = info.version();
        data.license = info.license();
        data.website = info.website();
        m_decorations.append(data);
    }
    qSort(m_decorations.begin(), m_decorations.end(), DecorationModelData::less);
    endResetModel();
}

void DecorationModel::findAuroraeThemes()
{
    // get all desktop themes
    QStringList themes = KGlobal::dirs()->findAllResources("data",
                         "aurorae/themes/*/metadata.desktop",
                         KStandardDirs::NoDuplicates);
    foreach (const QString & theme, themes) {
        int themeSepIndex = theme.lastIndexOf('/', -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);

        KDesktopFile df(theme);
        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }

        DecorationModelData data;
        data.name = name;
        data.libraryName = "kwin3_aurorae";
        data.type = DecorationModelData::AuroraeDecoration;
        data.auroraeName = packageName;
        KConfigGroup config(m_config, data.auroraeName);
        data.borderSize = (KDecorationDefines::BorderSize)config.readEntry< int >("BorderSize", KDecorationDefines::BorderNormal);
        data.buttonSize = (KDecorationDefines::BorderSize)config.readEntry< int >("ButtonSize", KDecorationDefines::BorderNormal);
        data.closeDblClick = config.readEntry< bool >("CloseOnDoubleClickMenuButton", true);
        metaData(data, df);
        m_decorations.append(data);
    }
}

void DecorationModel::metaData(DecorationModelData& data, const KDesktopFile& df)
{
    data.comment = df.readComment();
    data.author = df.desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
    data.email = df.desktopGroup().readEntry("X-KDE-PluginInfo-Email", QString());
    data.version = df.desktopGroup().readEntry("X-KDE-PluginInfo-Version", QString());
    data.license = df.desktopGroup().readEntry("X-KDE-PluginInfo-License", QString());
    data.website = df.desktopGroup().readEntry("X-KDE-PluginInfo-Website", QString());
}

int DecorationModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_decorations.count();
}

QVariant DecorationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role) {
    case Qt::DisplayRole:
    case NameRole:
        return m_decorations[ index.row()].name;
    case LibraryNameRole:
        return m_decorations[ index.row()].libraryName;
    case PixmapRole:
        return m_decorations[ index.row()].preview;
    case TypeRole:
        return m_decorations[ index.row()].type;
    case AuroraeNameRole:
        return m_decorations[ index.row()].auroraeName;
    case PackageDescriptionRole:
        return m_decorations[ index.row()].comment;
    case PackageAuthorRole:
        return m_decorations[ index.row()].author;
    case PackageEmailRole:
        return m_decorations[ index.row()].email;
    case PackageWebsiteRole:
        return m_decorations[ index.row()].website;
    case PackageVersionRole:
        return m_decorations[ index.row()].version;
    case PackageLicenseRole:
        return m_decorations[ index.row()].license;
    case BorderSizeRole:
        return static_cast< int >(m_decorations[ index.row()].borderSize);
    case BorderSizesRole: {
        QList< QVariant > sizes;
        const bool mustDisablePreview = m_plugins->factory() && m_plugins->factory() == m_preview->factory();
        if (m_plugins->loadPlugin(m_decorations[index.row()].libraryName) && m_plugins->factory()) {
            foreach (KDecorationDefines::BorderSize size, m_plugins->factory()->borderSizes())   // krazy:exclude=foreach
                sizes << int(size);
            if (mustDisablePreview) // it's nuked with destroyPreviousPlugin()
                m_preview->disablePreview(); // so we need to get rid of m_preview->deco first
            m_plugins->destroyPreviousPlugin();
        }
        return sizes;
    }
    case ButtonSizeRole:
        if (m_decorations[ index.row()].type == DecorationModelData::AuroraeDecoration ||
            m_decorations[ index.row()].type == DecorationModelData::QmlDecoration)
            return static_cast< int >(m_decorations[ index.row()].buttonSize);
        else
            return QVariant();
    case QmlMainScriptRole:
        return m_decorations[ index.row()].qmlPath;
    case CloseOnDblClickRole:
        return m_decorations[index.row()].closeDblClick;
    default:
        return QVariant();
    }
}

bool DecorationModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (role != BorderSizeRole && role != ButtonSizeRole && role != CloseOnDblClickRole))
        return QAbstractItemModel::setData(index, value, role);

    const DecorationModelData::DecorationType type = m_decorations[ index.row()].type;

    if (role == BorderSizeRole) {
        m_decorations[ index.row()].borderSize = (KDecorationDefines::BorderSize)value.toInt();
        if (type == DecorationModelData::AuroraeDecoration || type == DecorationModelData::QmlDecoration) {
            KConfigGroup config(m_config, m_decorations[ index.row()].auroraeName);
            config.writeEntry("BorderSize", value.toInt());
            config.sync();
        }
        emit dataChanged(index, index);
        emit configChanged(m_decorations[ index.row()].auroraeName);
        regeneratePreview(index);
        return true;
    }
    if (role == ButtonSizeRole && (type == DecorationModelData::AuroraeDecoration || type == DecorationModelData::QmlDecoration)) {
        m_decorations[ index.row()].buttonSize = (KDecorationDefines::BorderSize)value.toInt();
        KConfigGroup config(m_config, m_decorations[ index.row()].auroraeName);
        config.writeEntry("ButtonSize", value.toInt());
        config.sync();
        emit dataChanged(index, index);
        emit configChanged(m_decorations[ index.row()].auroraeName);
        regeneratePreview(index);
        return true;
    }
    if (role == CloseOnDblClickRole && (type == DecorationModelData::AuroraeDecoration || type == DecorationModelData::QmlDecoration)) {
        if (m_decorations[ index.row()].closeDblClick == value.toBool()) {
            return false;
        }
        m_decorations[ index.row()].closeDblClick = value.toBool();
        KConfigGroup config(m_config, m_decorations[ index.row()].auroraeName);
        config.writeEntry("CloseOnDoubleClickMenuButton", value.toBool());
        config.sync();
        emit dataChanged(index, index);
        emit configChanged(m_decorations[ index.row()].auroraeName);
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}


void DecorationModel::changeButtons(const KWin::DecorationButtons *buttons)
{
    bool regenerate = (buttons->customPositions() != m_customButtons);
    if (!regenerate && buttons->customPositions())
        regenerate = (buttons->leftButtons() != m_leftButtons) || (buttons->rightButtons() != m_rightButtons);
    m_customButtons = buttons->customPositions();
    m_leftButtons = buttons->leftButtons();
    m_rightButtons = buttons->rightButtons();
    if (regenerate)
        regeneratePreviews();
}

void DecorationModel::setButtons(bool custom, const QString& left, const QString& right)
{
    m_customButtons = custom;
    m_leftButtons = left;
    m_rightButtons = right;
}

void DecorationModel::regenerateNextPreview()
{
    if (m_nextPreviewIndex < m_lastUpdateIndex && m_nextPreviewIndex < m_decorations.count())
        regeneratePreview(index(m_nextPreviewIndex),
                          QSize(qobject_cast<KWinDecorationModule*>(QObject::parent())->itemWidth(), 150));
    ++m_nextPreviewIndex;
    if (m_nextPreviewIndex >= m_lastUpdateIndex && m_firstUpdateIndex > 0) {
        // do the above ones
        m_lastUpdateIndex = qMin(m_firstUpdateIndex, m_decorations.count());
        m_firstUpdateIndex = m_nextPreviewIndex = 0;
    }
    if (m_nextPreviewIndex < m_lastUpdateIndex)
        QMetaObject::invokeMethod(this, "regenerateNextPreview", Qt::QueuedConnection);
}

void DecorationModel::regeneratePreviews(int firstIndex)
{
    m_firstUpdateIndex = firstIndex;
    m_lastUpdateIndex = m_decorations.count();
    m_nextPreviewIndex = firstIndex;
    regenerateNextPreview();
}

void DecorationModel::stopPreviewGeneration()
{
    m_firstUpdateIndex = m_lastUpdateIndex = m_nextPreviewIndex = 0;
}

void DecorationModel::regeneratePreview(const QModelIndex& index, const QSize& size)
{
    DecorationModelData& data = m_decorations[ index.row()];

    switch(data.type) {
    case DecorationModelData::NativeDecoration: {
        bool enabled = false;
        bool loaded;
        // m_preview->deco management is not required
        // either the deco loads and the following recreateDecoration will sanitize decos (on new factory)
        // or the deco does not load and destroyPreviousPlugin() is not called
        if ((loaded = m_plugins->loadPlugin(data.libraryName)) && m_preview->recreateDecoration(m_plugins)) {
            enabled = true;
        } else {
            m_preview->disablePreview();
        }
        if (loaded)
            m_plugins->destroyPreviousPlugin();
        if (enabled) {
            m_preview->resize(size);
            m_preview->setTempButtons(m_plugins, m_customButtons, m_leftButtons, m_rightButtons);
            m_preview->setTempBorderSize(m_plugins, data.borderSize);
            data.preview = m_preview->preview();
        } else {
            m_decorations.removeAt(index.row());
        }
        break;
    }
    default:
        // nothing
        break;
    }
    emit dataChanged(index, index);
}

void DecorationModel::regeneratePreview(const QModelIndex& index)
{
    regeneratePreview(index, m_decorations.at(index.row()).preview.size());
}

QModelIndex DecorationModel::indexOfLibrary(const QString& libraryName) const
{
    for (int i = 0; i < m_decorations.count(); i++) {
        if (m_decorations.at(i).libraryName.compare(libraryName) == 0)
            return index(i);
    }
    return QModelIndex();
}

QModelIndex DecorationModel::indexOfName(const QString& decoName) const
{
    for (int i = 0; i < m_decorations.count(); i++) {
        if (m_decorations.at(i).name.compare(decoName) == 0)
            return index(i);
    }
    return QModelIndex();
}

QModelIndex DecorationModel::indexOfAuroraeName(const QString &auroraeName, const QString &type) const
{
    for (int i = 0; i < m_decorations.count(); i++) {
        const DecorationModelData& data = m_decorations.at(i);
        if (type == "aurorae" && data.type == DecorationModelData::AuroraeDecoration &&
                data.auroraeName.compare(auroraeName) == 0)
            return index(i);
        if (type == "qml" && data.type == DecorationModelData::QmlDecoration &&
                data.auroraeName.compare(auroraeName) == 0)
            return index(i);
    }
    return QModelIndex();
}

void DecorationModel::setBorderSize(const QModelIndex& index, KDecorationDefines::BorderSize size)
{
    if (!index.isValid() || m_decorations[ index.row()].type == DecorationModelData::AuroraeDecoration || m_decorations[ index.row()].type == DecorationModelData::QmlDecoration)
        return;
    m_decorations[ index.row()].borderSize = size;
}

QVariant DecorationModel::readConfig(const QString &themeName, const QString &key, const QVariant &defaultValue)
{
    return m_config->group(themeName).readEntry(key, defaultValue);
}

void DecorationModel::notifyConfigChanged(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    emit configChanged(m_decorations[index.row()].auroraeName);
}

} // namespace KWin
