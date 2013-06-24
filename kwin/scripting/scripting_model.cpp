/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

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
#include "scripting_model.h"
#include "activities.h"
#include "client.h"
#include "screens.h"
#include "workspace.h"

#include <KDE/KDebug>

namespace KWin {
namespace ScriptingClientModel {

static quint32 nextId() {
    static quint32 counter = 0;
    return ++counter;
}

ClientLevel::ClientLevel(ClientModel *model, AbstractLevel *parent)
    : AbstractLevel(model, parent)
{
    connect(Workspace::self(), SIGNAL(clientAdded(KWin::Client*)), SLOT(clientAdded(KWin::Client*)));
    connect(Workspace::self(), SIGNAL(clientRemoved(KWin::Client*)), SLOT(clientRemoved(KWin::Client*)));
    connect(model, SIGNAL(exclusionsChanged()), SLOT(reInit()));
}

ClientLevel::~ClientLevel()
{
}

void ClientLevel::clientAdded(Client *client)
{
    setupClientConnections(client);
    checkClient(client);
}

void ClientLevel::clientRemoved(Client *client)
{
    removeClient(client);
}

void ClientLevel::setupClientConnections(Client *client)
{
    connect(client, SIGNAL(desktopChanged()), SLOT(checkClient()));
    connect(client, SIGNAL(screenChanged()), SLOT(checkClient()));
    connect(client, SIGNAL(activitiesChanged(KWin::Toplevel*)), SLOT(checkClient()));
}

void ClientLevel::checkClient()
{
    checkClient(static_cast<Client*>(sender()));
}

void ClientLevel::checkClient(Client *client)
{
    const bool shouldInclude = !exclude(client) && shouldAdd(client);
    const bool contains = containsClient(client);

    if (shouldInclude && !contains) {
        addClient(client);
    } else if (!shouldInclude && contains) {
        removeClient(client);
    }
}

bool ClientLevel::exclude(Client *client) const
{
    ClientModel::Exclusions exclusions = model()->exclusions();
    if (exclusions == ClientModel::NoExclusion) {
        return false;
    }
    if (exclusions & ClientModel::DesktopWindowsExclusion) {
        if (client->isDesktop()) {
            return true;
        }
    }
    if (exclusions & ClientModel::DockWindowsExclusion) {
        if (client->isDock()) {
            return true;
        }
    }
    if (exclusions & ClientModel::UtilityWindowsExclusion) {
        if (client->isUtility()) {
            return true;
        }
    }
    if (exclusions & ClientModel::SpecialWindowsExclusion) {
        if (client->isSpecialWindow()) {
            return true;
        }
    }
    if (exclusions & ClientModel::SkipTaskbarExclusion) {
        if (client->skipTaskbar()) {
            return true;
        }
    }
    if (exclusions & ClientModel::SkipPagerExclusion) {
        if (client->skipPager()) {
            return true;
        }
    }
    if (exclusions & ClientModel::SwitchSwitcherExclusion) {
        if (client->skipSwitcher()) {
            return true;
        }
    }
    if (exclusions & ClientModel::OtherDesktopsExclusion) {
        if (!client->isOnCurrentDesktop()) {
            return true;
        }
    }
    if (exclusions & ClientModel::OtherActivitiesExclusion) {
        if (!client->isOnCurrentActivity()) {
            return true;
        }
    }
    if (exclusions & ClientModel::MinimizedExclusion) {
        if (client->isMinimized()) {
            return true;
        }
    }
    if (exclusions & ClientModel::NonSelectedWindowTabExclusion) {
        if (!client->isCurrentTab()) {
            return true;
        }
    }
    if (exclusions & ClientModel::NotAcceptingFocusExclusion) {
        if (!client->wantsInput()) {
            return true;
        }
    }
    return false;
}

bool ClientLevel::shouldAdd(Client *client) const
{
    if (restrictions() == ClientModel::NoRestriction) {
        return true;
    }
    if (restrictions() & ClientModel::ActivityRestriction) {
        if (!client->isOnActivity(activity())) {
            return false;
        }
    }
    if (restrictions() & ClientModel::VirtualDesktopRestriction) {
        if (!client->isOnDesktop(virtualDesktop())) {
            return false;
        }
    }
    if (restrictions() & ClientModel::ScreenRestriction) {
        if (client->screen() != int(screen())) {
            return false;
        }
    }
    return true;
}

void ClientLevel::addClient(Client *client)
{
    if (containsClient(client)) {
        return;
    }
    emit beginInsert(m_clients.count(), m_clients.count(), id());
    m_clients.insert(nextId(), client);
    emit endInsert();
}

void ClientLevel::removeClient(Client *client)
{
    int index = 0;
    QMap<quint32, Client*>::iterator it = m_clients.begin();
    for (; it != m_clients.end(); ++it, ++index) {
        if (it.value() == client) {
            break;
        }
    }
    if (it == m_clients.end()) {
        return;
    }
    emit beginRemove(index, index, id());
    m_clients.erase(it);
    emit endRemove();
}

void ClientLevel::init()
{
    const ClientList &clients = Workspace::self()->clientList();
    for (ClientList::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        Client *client = *it;
        setupClientConnections(client);
        if (!exclude(client) && shouldAdd(client)) {
            m_clients.insert(nextId(), client);
        }
    }
}

void ClientLevel::reInit()
{
    const ClientList &clients = Workspace::self()->clientList();
    for (ClientList::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        checkClient((*it));
    }
}

quint32 ClientLevel::idForRow(int row) const
{
    if (row >= m_clients.size()) {
        return 0;
    }
    QMap<quint32, Client*>::const_iterator it = m_clients.constBegin();
    for (int i=0; i<row; ++i) {
        ++it;
    }
    return it.key();
}

bool ClientLevel::containsId(quint32 id) const
{
    return m_clients.contains(id);
}

int ClientLevel::rowForId(quint32 id) const
{
    int row = 0;
    for (QMap<quint32, Client*>::const_iterator it = m_clients.constBegin();
            it != m_clients.constEnd();
            ++it, ++row) {
        if (it.key() == id) {
            return row;
        }
    }
    return -1;
}

Client *ClientLevel::clientForId(quint32 child) const
{
    QMap<quint32, Client*>::const_iterator it = m_clients.constFind(child);
    if (it == m_clients.constEnd()) {
        return NULL;
    }
    return it.value();
}

bool ClientLevel::containsClient(Client *client) const
{
    for (QMap<quint32, Client*>::const_iterator it = m_clients.constBegin();
            it != m_clients.constEnd();
            ++it) {
        if (it.value() == client) {
            return true;
        }
    }
    return false;
}

const AbstractLevel *ClientLevel::levelForId(quint32 id) const
{
    if (id == AbstractLevel::id()) {
        return this;
    }
    return NULL;
}

AbstractLevel *ClientLevel::parentForId(quint32 child) const
{
    if (child == id()) {
        return parentLevel();
    }
    if (m_clients.contains(child)) {
        return const_cast<ClientLevel*>(this);
    }
    return NULL;
}

AbstractLevel *AbstractLevel::create(const QList< ClientModel::LevelRestriction > &restrictions, ClientModel::LevelRestrictions parentRestrictions, ClientModel *model, AbstractLevel *parent)
{
    if (restrictions.isEmpty() || restrictions.first() == ClientModel::NoRestriction) {
        ClientLevel *leaf = new ClientLevel(model, parent);
        leaf->setRestrictions(parentRestrictions);
        if (!parent) {
            leaf->setParent(model);
        }
        return leaf;
    }
    // create a level
    QList<ClientModel::LevelRestriction> childRestrictions(restrictions);
    ClientModel::LevelRestriction restriction = childRestrictions.takeFirst();
    ClientModel::LevelRestrictions childrenRestrictions = restriction | parentRestrictions;
    ForkLevel *currentLevel = new ForkLevel(childRestrictions, model, parent);
    currentLevel->setRestrictions(childrenRestrictions);
    currentLevel->setRestriction(restriction);
    if (!parent) {
        currentLevel->setParent(model);
    }
    switch (restriction) {
    case ClientModel::ActivityRestriction: {
#ifdef KWIN_BUILD_ACTIVITIES
        const QStringList &activities = Activities::self()->all();
        for (QStringList::const_iterator it = activities.begin(); it != activities.end(); ++it) {
            AbstractLevel *childLevel = create(childRestrictions, childrenRestrictions, model, currentLevel);
            if (!childLevel) {
                continue;
            }
            childLevel->setActivity(*it);
            currentLevel->addChild(childLevel);
        }
        break;
#else
        return NULL;
#endif
    }
    case ClientModel::ScreenRestriction:
        for (int i=0; i<screens()->count(); ++i) {
            AbstractLevel *childLevel = create(childRestrictions, childrenRestrictions, model, currentLevel);
            if (!childLevel) {
                continue;
            }
            childLevel->setScreen(i);
            currentLevel->addChild(childLevel);
        }
        break;
    case ClientModel::VirtualDesktopRestriction:
        for (uint i=1; i<=VirtualDesktopManager::self()->count(); ++i) {
            AbstractLevel *childLevel = create(childRestrictions, childrenRestrictions, model, currentLevel);
            if (!childLevel) {
                continue;
            }
            childLevel->setVirtualDesktop(i);
            currentLevel->addChild(childLevel);
        }
        break;
    default:
        // invalid
        return NULL;
    }

    return currentLevel;
}

AbstractLevel::AbstractLevel(ClientModel *model, AbstractLevel *parent)
    : QObject(parent)
    , m_model(model)
    , m_parent(parent)
    , m_screen(0)
    , m_virtualDesktop(0)
    , m_activity()
    , m_restriction(ClientModel::ClientModel::NoRestriction)
    , m_restrictions(ClientModel::NoRestriction)
    , m_id(nextId())
{
}

AbstractLevel::~AbstractLevel()
{
}

void AbstractLevel::setRestriction(ClientModel::LevelRestriction restriction)
{
    m_restriction = restriction;
}

void AbstractLevel::setActivity(const QString &activity)
{
    m_activity = activity;
}

void AbstractLevel::setScreen(uint screen)
{
    m_screen = screen;
}

void AbstractLevel::setVirtualDesktop(uint virtualDesktop)
{
    m_virtualDesktop = virtualDesktop;
}

void AbstractLevel::setRestrictions(ClientModel::LevelRestrictions restrictions)
{
    m_restrictions = restrictions;
}

ForkLevel::ForkLevel(const QList<ClientModel::LevelRestriction> &childRestrictions, ClientModel *model, AbstractLevel *parent)
    : AbstractLevel(model, parent)
    , m_childRestrictions(childRestrictions)
{
    connect(VirtualDesktopManager::self(), SIGNAL(countChanged(uint,uint)), SLOT(desktopCountChanged(uint,uint)));
    connect(screens(), SIGNAL(countChanged(int,int)), SLOT(screenCountChanged(int,int)));
#ifdef KWIN_BUILD_ACTIVITIES
    Activities *activities = Activities::self();
    connect(activities, SIGNAL(added(QString)), SLOT(activityAdded(QString)));
    connect(activities, SIGNAL(removed(QString)), SLOT(activityRemoved(QString)));
#endif
}

ForkLevel::~ForkLevel()
{
}

void ForkLevel::desktopCountChanged(uint previousCount, uint newCount)
{
    if (restriction() != ClientModel::ClientModel::VirtualDesktopRestriction) {
        return;
    }
    if (previousCount != uint(count())) {
        return;
    }
    if (previousCount > newCount) {
        // desktops got removed
        emit beginRemove(newCount, previousCount-1, id());
        while (uint(m_children.count()) > newCount) {
            delete m_children.takeLast();
        }
        emit endRemove();
    } else {
        // desktops got added
        emit beginInsert(previousCount, newCount-1, id());
        for (uint i=previousCount+1; i<=newCount; ++i) {
            AbstractLevel *childLevel = AbstractLevel::create(m_childRestrictions, restrictions(), model(), this);
            if (!childLevel) {
                continue;
            }
            childLevel->setVirtualDesktop(i);
            childLevel->init();
            addChild(childLevel);
        }
        emit endInsert();
    }
}

void ForkLevel::screenCountChanged(int previousCount, int newCount)
{
    if (restriction() != ClientModel::ClientModel::ClientModel::ScreenRestriction) {
        return;
    }
    if (newCount == previousCount || previousCount != count()) {
        return;
    }

    if (previousCount > newCount) {
        // screens got removed
        emit beginRemove(newCount, previousCount-1, id());
        while (m_children.count() > newCount) {
            delete m_children.takeLast();
        }
        emit endRemove();
    } else {
        // screens got added
        emit beginInsert(previousCount, newCount-1, id());
        for (int i=previousCount; i<newCount; ++i) {
            AbstractLevel *childLevel = AbstractLevel::create(m_childRestrictions, restrictions(), model(), this);
            if (!childLevel) {
                continue;
            }
            childLevel->setScreen(i);
            childLevel->init();
            addChild(childLevel);
        }
        emit endInsert();
    }
}

void ForkLevel::activityAdded(const QString &activityId)
{
#ifdef KWIN_BUILD_ACTIVITIES
    if (restriction() != ClientModel::ClientModel::ActivityRestriction) {
        return;
    }
    // verify that our children do not contain this activity
    foreach (AbstractLevel *child, m_children) {
        if (child->activity() == activityId) {
            return;
        }
    }
    emit beginInsert(m_children.count(), m_children.count(), id());
    AbstractLevel *childLevel = AbstractLevel::create(m_childRestrictions, restrictions(), model(), this);
    if (!childLevel) {
        emit endInsert();
        return;
    }
    childLevel->setActivity(activityId);
    childLevel->init();
    addChild(childLevel);
    emit endInsert();
#endif
}

void ForkLevel::activityRemoved(const QString &activityId)
{
#ifdef KWIN_BUILD_ACTIVITIES
    if (restriction() != ClientModel::ClientModel::ActivityRestriction) {
        return;
    }
    for (int i=0; i<m_children.length(); ++i) {
        if (m_children.at(i)->activity() == activityId) {
            emit beginRemove(i, i, id());
            delete m_children.takeAt(i);
            emit endRemove();
            break;
        }
    }
#endif
}

int ForkLevel::count() const
{
    return m_children.count();
}

void ForkLevel::addChild(AbstractLevel *child)
{
    m_children.append(child);
    connect(child, SIGNAL(beginInsert(int,int,quint32)), SIGNAL(beginInsert(int,int,quint32)));
    connect(child, SIGNAL(beginRemove(int,int,quint32)), SIGNAL(beginRemove(int,int,quint32)));
    connect(child, SIGNAL(endInsert()), SIGNAL(endInsert()));
    connect(child, SIGNAL(endRemove()), SIGNAL(endRemove()));
}

void ForkLevel::setActivity(const QString &activity)
{
    AbstractLevel::setActivity(activity);
    for (QList<AbstractLevel*>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
        (*it)->setActivity(activity);
    }
}

void ForkLevel::setScreen(uint screen)
{
    AbstractLevel::setScreen(screen);
    for (QList<AbstractLevel*>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
        (*it)->setScreen(screen);
    }
}

void ForkLevel::setVirtualDesktop(uint virtualDesktop)
{
    AbstractLevel::setVirtualDesktop(virtualDesktop);
    for (QList<AbstractLevel*>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
        (*it)->setVirtualDesktop(virtualDesktop);
    }
}

void ForkLevel::init()
{
    for (QList<AbstractLevel*>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
        (*it)->init();
    }
}

quint32 ForkLevel::idForRow(int row) const
{
    if (row >= m_children.length()) {
        return 0;
    }
    return m_children.at(row)->id();
}

const AbstractLevel *ForkLevel::levelForId(quint32 id) const
{
    if (id == AbstractLevel::id()) {
        return this;
    }
    for (QList<AbstractLevel*>::const_iterator it = m_children.constBegin(); it != m_children.constEnd(); ++it) {
        if (const AbstractLevel *child = (*it)->levelForId(id)) {
            return child;
        }
    }
    // not found
    return NULL;
}

AbstractLevel *ForkLevel::parentForId(quint32 child) const
{
    if (child == id()) {
        return parentLevel();
    }
    for (QList<AbstractLevel*>::const_iterator it = m_children.constBegin(); it != m_children.constEnd(); ++it) {
        if (AbstractLevel *parent = (*it)->parentForId(child)) {
            return parent;
        }
    }
    // not found
    return NULL;
}

int ForkLevel::rowForId(quint32 child) const
{
    if (id() == child) {
        return 0;
    }
    for (int i=0; i<m_children.count(); ++i) {
        if (m_children.at(i)->id() == child) {
            return i;
        }
    }
    // do recursion
    for (QList<AbstractLevel*>::const_iterator it = m_children.constBegin(); it != m_children.constEnd(); ++it) {
        int row = (*it)->rowForId(child);
        if (row != -1) {
            return  row;
        }
    }
    // not found
    return -1;
}

Client *ForkLevel::clientForId(quint32 child) const
{
    for (QList<AbstractLevel*>::const_iterator it = m_children.constBegin(); it != m_children.constEnd(); ++it) {
        if (Client *client = (*it)->clientForId(child)) {
            return client;
        }
    }
    // not found
    return NULL;
}

ClientModel::ClientModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_root(NULL)
    , m_exclusions(NoExclusion)
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(Qt::DisplayRole, "display");
    roleNames.insert(ClientRole, "client");
    roleNames.insert(ScreenRole, "screen");
    roleNames.insert(DesktopRole, "desktop");
    roleNames.insert(ActivityRole, "activity");
    setRoleNames(roleNames);
}

ClientModel::~ClientModel()
{
}

void ClientModel::setLevels(QList< ClientModel::LevelRestriction > restrictions)
{
    beginResetModel();
    if (m_root) {
        delete m_root;
    }
    m_root = AbstractLevel::create(restrictions, NoRestriction, this);
    connect(m_root, SIGNAL(beginInsert(int,int,quint32)), SLOT(levelBeginInsert(int,int,quint32)));
    connect(m_root, SIGNAL(beginRemove(int,int,quint32)), SLOT(levelBeginRemove(int,int,quint32)));
    connect(m_root, SIGNAL(endInsert()), SLOT(levelEndInsert()));
    connect(m_root, SIGNAL(endRemove()), SLOT(levelEndRemove()));
    m_root->init();
    endResetModel();
}

void ClientModel::setExclusions(ClientModel::Exclusions exclusions)
{
    if (exclusions == m_exclusions) {
        return;
    }
    m_exclusions = exclusions;
    emit exclusionsChanged();
}

QVariant ClientModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0) {
        return QVariant();
    }
    if (const AbstractLevel *level = getLevel(index)) {
        LevelRestriction restriction = level->restriction();
        if (restriction == ActivityRestriction && (role == Qt::DisplayRole || role == ActivityRole)) {
            return level->activity();
        } else if (restriction == VirtualDesktopRestriction && (role == Qt::DisplayRole || role == DesktopRole)) {
            return level->virtualDesktop();
        } else if (restriction ==ScreenRestriction && (role == Qt::DisplayRole || role == ScreenRole)) {
            return level->screen();
        } else {
            return QVariant();
        }
    }
    if (role == Qt::DisplayRole || role == ClientRole) {
        if (Client *client = m_root->clientForId(index.internalId())) {
            return qVariantFromValue(client);
        }
    }
    return QVariant();
}

int ClientModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

int ClientModel::rowCount(const QModelIndex &parent) const
{
    if (!m_root) {
        return 0;
    }
    if (!parent.isValid()) {
        return m_root->count();
    }
    if (const AbstractLevel *level = getLevel(parent)) {
        if (level->id() != parent.internalId()) {
            // not a real level - no children
            return 0;
        }
        return level->count();
    }
    return 0;
}

QModelIndex ClientModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || child.column() != 0) {
        return QModelIndex();
    }
    return parentForId(child.internalId());
}

QModelIndex ClientModel::parentForId(quint32 childId) const
{
    if (childId == m_root->id()) {
        // asking for parent of our toplevel
        return QModelIndex();
    }
    if (AbstractLevel *parentLevel = m_root->parentForId(childId)) {
        if (parentLevel == m_root) {
            return QModelIndex();
        }
        const int row = m_root->rowForId(parentLevel->id());
        if (row == -1) {
            // error
            return QModelIndex();
        }
        return createIndex(row, 0, parentLevel->id());
    }
    return QModelIndex();
}

QModelIndex ClientModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0 || row < 0 || !m_root) {
        return QModelIndex();
    }
    if (!parent.isValid()) {
        if (row >= rowCount()) {
            return QModelIndex();
        }
        return createIndex(row, 0, m_root->idForRow(row));
    }
    const AbstractLevel *parentLevel = getLevel(parent);
    if (!parentLevel) {
        return QModelIndex();
    }
    if (row >= parentLevel->count()) {
        return QModelIndex();
    }
    const quint32 id = parentLevel->idForRow(row);
    if (id == 0) {
        return QModelIndex();
    }
    return createIndex(row, column, id);
}

const AbstractLevel *ClientModel::getLevel(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_root;
    }
    return m_root->levelForId(index.internalId());
}

void ClientModel::levelBeginInsert(int rowStart, int rowEnd, quint32 id)
{
    const int row = m_root->rowForId(id);
    QModelIndex parent;
    if (row != -1) {
        parent = createIndex(row, 0, id);
    }
    beginInsertRows(parent, rowStart, rowEnd);
}

void ClientModel::levelBeginRemove(int rowStart, int rowEnd, quint32 id)
{
    const int row = m_root->rowForId(id);
    QModelIndex parent;
    if (row != -1) {
        parent = createIndex(row, 0, id);
    }
    beginRemoveRows(parent, rowStart, rowEnd);
}

void ClientModel::levelEndInsert()
{
    endInsertRows();
}

void ClientModel::levelEndRemove()
{
    endRemoveRows();
}

#define CLIENT_MODEL_WRAPPER(name, levels) \
name::name(QObject *parent) \
    : ClientModel(parent) \
{ \
    setLevels(levels); \
} \
name::~name() {}

CLIENT_MODEL_WRAPPER(SimpleClientModel, QList<LevelRestriction>())
CLIENT_MODEL_WRAPPER(ClientModelByScreen, QList<LevelRestriction>() << ScreenRestriction)
CLIENT_MODEL_WRAPPER(ClientModelByScreenAndDesktop, QList<LevelRestriction>() << ScreenRestriction << VirtualDesktopRestriction)
#undef CLIENT_MODEL_WRAPPER

ClientFilterModel::ClientFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_clientModel(NULL)
{
}

ClientFilterModel::~ClientFilterModel()
{
}

void ClientFilterModel::setClientModel(ClientModel *clientModel)
{
    if (clientModel == m_clientModel) {
        return;
    }
    m_clientModel = clientModel;
    setSourceModel(m_clientModel);
    emit clientModelChanged();
}

void ClientFilterModel::setFilter(const QString &filter)
{
    if (filter == m_filter) {
        return;
    }
    m_filter = filter;
    emit filterChanged();
    invalidateFilter();
}

bool ClientFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_clientModel) {
        return false;
    }
    if (m_filter.isEmpty()) {
        return true;
    }
    QModelIndex index = m_clientModel->index(sourceRow, 0, sourceParent);
    if (!index.isValid()) {
        return false;
    }
    QVariant data = index.data();
    if (!data.isValid()) {
        // an invalid QVariant is valid data
        return true;
    }
    // TODO: introduce a type as a data role and properly check, this seems dangerous
    if (data.type() == QVariant::Int || data.type() == QVariant::UInt || data.type() == QVariant::String) {
        // we do not filter out screen, desktop and activity
        return true;
    }
    Client *client = qvariant_cast<KWin::Client *>(data);
    if (!client) {
        return false;
    }
    if (client->caption().contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }
    const QString windowRole(client->windowRole());
    if (windowRole.contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }
    const QString resourceName(client->resourceName());
    if (resourceName.contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }
    const QString resourceClass(client->resourceClass());
    if (resourceClass.contains(m_filter, Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

} // namespace Scripting
} // namespace KWin
