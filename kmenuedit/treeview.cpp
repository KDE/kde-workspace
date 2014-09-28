/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org>
 *   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *   Copyright (C) 2008 Laurent Montel <montel@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "treeview.h"

#include <unistd.h>

#include <QDir>
#include <QHeaderView>
#include <QPainter>
#include <QRegExp>
#include <QPixmap>
#include <QDropEvent>
#include <QMenu>
#include <QApplication>
#include <QtDBus/QtDBus>
#include <QSignalMapper>

#include <KAction>
#include <KActionCollection>
#include <KBuildSycocaProgressDialog>
#include <KDebug>
#include <KDesktopFile>
#include <KGlobal>
#include <KIconLoader>
#include <KInputDialog>
#include <KLocale>
#include <KMessageBox>
#include <KService>
#include <KServiceGroup>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KIO/NetAccess>

#include "treeview.moc"
#include "menufile.h"
#include "menuinfo.h"

#define MOVE_FOLDER 'M'
#define COPY_FOLDER 'C'
#define MOVE_FILE   'm'
#define COPY_FILE   'c'
#define COPY_SEPARATOR 'S'

static const char *s_internalMimeType = "application/x-kmenuedit-internal";

class SeparatorWidget : public QWidget
{
public:
    SeparatorWidget()
        : QWidget(0)
    {
    }

protected:
    void paintEvent(QPaintEvent * /*event*/)
    {
        QPainter p(this);
        // Draw Separator
        int h = (height() / 2) -1;
//        if (isSelected()) {
//            p->setPen( palette().color( QPalette::HighlightedText ) );
//        } else {
//            p->setPen( palette().color( QPalette::Text ) );
//        }

        p.drawLine(2, h, width() - 4, h);
    }
};


TreeItem::TreeItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, const QString& menuId, bool _m_init)
    : QTreeWidgetItem(parent, after),
      m_hidden(false),
      m_init(_m_init),
      m_layoutDirty(false),
      m_menuId(menuId),
      m_folderInfo(0),
      m_entryInfo(0)
{
}

TreeItem::TreeItem(QTreeWidget *parent, QTreeWidgetItem *after, const QString& menuId, bool _m_init)
    : QTreeWidgetItem(parent, after),
      m_hidden(false),
      m_init(_m_init),
      m_layoutDirty(false),
      m_menuId(menuId),
      m_folderInfo(0),
      m_entryInfo(0)
{
    load();
}

TreeItem::~TreeItem()
{
}

/**
 * @brief Return the description.
 * @return Description, or an empty string if none.
 */
QString TreeItem::description() const
{
    QString description;
    if (isEntry()) {
        description = entryInfo()->description;
    }
    return description;
}

/**
 * @brief Compare two items using their names.
 * @param item1 First item.
 * @param item2 Second item.
 * @return Integer less than, equal to, or greater than zero if item1 is less than, equal to, or greater than item2.
 */
bool TreeItem::itemNameLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2)
{
    TreeItem *treeItem1 = static_cast<TreeItem*>(item1);
    TreeItem *treeItem2 = static_cast<TreeItem*>(item2);
    return treeItem1->name().toLower() < treeItem2->name().toLower();
}

/**
 * @brief Compare two items using their descriptions. If both are empty, sort them by name.
 * @param item1 First item.
 * @param item2 Second item.
 * @return Integer less than, equal to, or greater than zero if item1 is less than, equal to, or greater than item2.
 */
bool TreeItem::itemDescriptionLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2)
{
    // extract descriptions in lower case
    TreeItem *treeItem1 = static_cast<TreeItem*>(item1);
    TreeItem *treeItem2 = static_cast<TreeItem*>(item2);
    const QString description1 = treeItem1->description().toLower();
    const QString description2 = treeItem2->description().toLower();

    // if description is missing for both items, sort them using their names
    if (description1.isEmpty() && description2.isEmpty()) {
        return itemNameLessThan(item1, item2);
    }
    else {
        return description1 < description2;
    }
}

void TreeItem::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    update();
}

void TreeItem::setHiddenInMenu(bool b)
{
    if (m_hidden == b) {
        return;
    }

    m_hidden = b;
    update();
}

void TreeItem::update()
{
    QString s = m_name;
    if (m_hidden) {
       s += i18n(" [Hidden]");
    }

    setText(0, s);
}

void TreeItem::load()
{
    if (m_folderInfo && !m_init) {
       m_init = true;
       TreeView *tv = static_cast<TreeView *>(treeWidget());
       tv->fillBranch(m_folderInfo, this);
    }
}

bool TreeItem::isLayoutDirty() const
{
    if (m_layoutDirty) {
        return true;
    }

    for (int i = 0; i < childCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(child(i));
        if (!item) {
            continue;
        }

        if (item->isLayoutDirty()) {
            return true;
        }
    }

    return false;
}

static QPixmap appIcon(const QString &iconName)
{
    QPixmap normal = KIconLoader::global()->loadIcon(iconName, KIconLoader::Small, 0, KIconLoader::DefaultState, QStringList(), 0L, true);
    return normal;
}


TreeView::TreeView( KActionCollection *ac, QWidget *parent, const char *name )
    : QTreeWidget(parent), m_ac(ac), m_popupMenu(0), m_clipboard(0),
      m_clipboardFolderInfo(0), m_clipboardEntryInfo(0),
      m_layoutDirty(false),
      m_detailedMenuEntries(true), m_detailedEntriesNamesFirst(true)
{
    m_dropMimeTypes << s_internalMimeType << KUrl::List::mimeDataTypes();
    qRegisterMetaType<TreeItem *>("TreeItem");
    setObjectName(name);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);
    setSortingEnabled(false);
    setDragEnabled(true);
    setAcceptDrops(true);
    setMinimumWidth(240);

    setHeaderLabels(QStringList() << QString(""));
    header()->hide();

    // listen for creation
    connect(m_ac->action(NEW_ITEM_ACTION_NAME), SIGNAL(activated()), SLOT(newitem()));
    connect(m_ac->action(NEW_SUBMENU_ACTION_NAME), SIGNAL(activated()), SLOT(newsubmenu()));
    connect(m_ac->action(NEW_SEPARATOR_ACTION_NAME), SIGNAL(activated()), SLOT(newsep()));

    // listen for copy
    connect(m_ac->action(CUT_ACTION_NAME), SIGNAL(activated()), SLOT(cut()));
    connect(m_ac->action(COPY_ACTION_NAME), SIGNAL(activated()), SLOT(copy()));
    connect(m_ac->action(PASTE_ACTION_NAME), SIGNAL(activated()), SLOT(paste()));

    // listen for deleting
    connect(m_ac->action(DELETE_ACTION_NAME), SIGNAL(activated()), SLOT(del()));

    // listen for sorting
    m_sortSignalMapper = new QSignalMapper(this);
    QAction *action = m_ac->action(SORT_BY_NAME_ACTION_NAME);
    connect(action, SIGNAL(activated()), m_sortSignalMapper, SLOT(map()));
    m_sortSignalMapper->setMapping(action, SortByName);
    action = m_ac->action(SORT_BY_DESCRIPTION_ACTION_NAME);
    connect(action, SIGNAL(activated()), m_sortSignalMapper, SLOT(map()));
    m_sortSignalMapper->setMapping(action, SortByDescription);
    action = m_ac->action(SORT_ALL_BY_NAME_ACTION_NAME);
    connect(action, SIGNAL(activated()), m_sortSignalMapper, SLOT(map()));
    m_sortSignalMapper->setMapping(action, SortAllByName);
    action = m_ac->action(SORT_ALL_BY_DESCRIPTION_ACTION_NAME);
    connect(action, SIGNAL(activated()), m_sortSignalMapper, SLOT(map()));
    m_sortSignalMapper->setMapping(action, SortAllByDescription);
    connect(m_sortSignalMapper, SIGNAL(mapped(const int)), this, SLOT(sort(const int)));

    // connect moving up/down actions
    connect(m_ac->action(MOVE_UP_ACTION_NAME), SIGNAL(activated()), SLOT(moveUpItem()));
    connect(m_ac->action(MOVE_DOWN_ACTION_NAME), SIGNAL(activated()), SLOT(moveDownItem()));

    // listen for selection
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(itemSelected(QTreeWidgetItem*)));

    m_menuFile = new MenuFile(KStandardDirs::locateLocal("xdgconf-menu", "applications-kmenuedit.menu"));
    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;
}

TreeView::~TreeView()
{
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;
}

void TreeView::setViewMode(bool showHidden)
{
    // setup popup menu
    delete m_popupMenu;
    m_popupMenu = new QMenu(this);

    // creation
    m_popupMenu->addAction(m_ac->action(NEW_ITEM_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(NEW_SUBMENU_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(NEW_SEPARATOR_ACTION_NAME));
    m_popupMenu->addSeparator();

    // copy
    m_popupMenu->addAction(m_ac->action(CUT_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(COPY_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(PASTE_ACTION_NAME));
    m_popupMenu->addSeparator();

    // delete
    m_popupMenu->addAction( m_ac->action(DELETE_ACTION_NAME));
    m_popupMenu->addSeparator();

    // move
    m_popupMenu->addAction(m_ac->action(MOVE_UP_ACTION_NAME));
    m_popupMenu->addAction(m_ac->action(MOVE_DOWN_ACTION_NAME));
    m_popupMenu->addSeparator();

    // sort
    m_popupMenu->addAction(m_ac->action(SORT_ACTION_NAME));

    m_showHidden = showHidden;
    readMenuFolderInfo();
    fill();
}

void TreeView::readMenuFolderInfo(MenuFolderInfo *folderInfo, KServiceGroup::Ptr folder, const QString &prefix)
{
    if (!folderInfo)
    {
       folderInfo = m_rootFolder;
       folder = KServiceGroup::root();
    }

    if (!folder || !folder->isValid())
        return;

    folderInfo->caption = folder->caption();
    folderInfo->comment = folder->comment();

    // Item names may contain ampersands. To avoid them being converted
    // to accelerators, replace them with two ampersands.
    folderInfo->hidden = folder->noDisplay();
    folderInfo->directoryFile = folder->directoryEntryPath();
    folderInfo->icon = folder->icon();
    QString id = folder->relPath();
    int i = id.lastIndexOf('/', -2);
    id = id.mid(i+1);
    folderInfo->id = id;
    folderInfo->fullId = prefix + id;

    foreach(const KSycocaEntry::Ptr &e, folder->entries(true, !m_showHidden, true, m_detailedMenuEntries && !m_detailedEntriesNamesFirst))
    {
        if (e->isType(KST_KServiceGroup))
        {
            KServiceGroup::Ptr g(KServiceGroup::Ptr::staticCast(e));
            MenuFolderInfo *subFolderInfo = new MenuFolderInfo();
            readMenuFolderInfo(subFolderInfo, g, folderInfo->fullId);
            folderInfo->add(subFolderInfo, true);
        }
        else if (e->isType(KST_KService))
        {
            folderInfo->add(new MenuEntryInfo(KService::Ptr::staticCast(e)), true);
        }
        else if (e->isType(KST_KServiceSeparator))
        {
            folderInfo->add(m_separator, true);
        }
    }
}

void TreeView::fill()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    clear();
    fillBranch(m_rootFolder, 0);
    QApplication::restoreOverrideCursor();
}

QString TreeView::findName(KDesktopFile *df, bool deleted)
{
    QString name = df->readName();
    if (deleted)
    {
       if (name == "empty")
          name.clear();
       if (name.isEmpty())
       {
          QString file = df->fileName();
          QString res = df->resource();

          bool isLocal = true;
          const QStringList files = KGlobal::dirs()->findAllResources(res.toLatin1(), file);
          for(QStringList::ConstIterator it = files.constBegin();
              it != files.constEnd();
              ++it)
          {
             if (isLocal)
             {
                isLocal = false;
                continue;
             }

             KDesktopFile df2(*it);
             name = df2.readName();

             if (!name.isEmpty() && (name != "empty"))
                return name;
          }
       }
    }
    return name;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuFolderInfo *folderInfo, bool m_init)
{
    TreeItem *item;
    if (parent) {
        item = new TreeItem(parent, after, QString(), m_init);
    } else {
        item = new TreeItem(this, after, QString(), m_init);
    }

    item->setMenuFolderInfo(folderInfo);
    item->setName(folderInfo->caption);
    item->setIcon(0, appIcon(folderInfo->icon));
    item->setDirectoryPath(folderInfo->fullId);
    item->setHiddenInMenu(folderInfo->hidden);
    item->load();
    return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuEntryInfo *entryInfo, bool m_init)
{
    bool hidden = entryInfo->hidden;

    TreeItem* item;
    if (parent) {
        item = new TreeItem(parent, after, entryInfo->menuId(),m_init);
    } else {
        item = new TreeItem(this, after, entryInfo->menuId(), m_init);
    }

    QString name;

    if (m_detailedMenuEntries && entryInfo->description.length() != 0) {
        if (m_detailedEntriesNamesFirst) {
            name = entryInfo->caption + " (" + entryInfo->description + ')';
        } else {
            name = entryInfo->description + " (" + entryInfo->caption + ')';
        }
    } else {
        name = entryInfo->caption;
    }

    //kDebug() << parent << after << name;
    item->setMenuEntryInfo(entryInfo);
    item->setName(name);
    item->setIcon(0, appIcon(entryInfo->icon));
    item->setHiddenInMenu(hidden);
    item->load();

    return item;
}

TreeItem *TreeView::createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuSeparatorInfo *, bool init)
{
    TreeItem* item;
    if (parent) {
        item = new TreeItem(parent, after, QString(), init);
    } else {
        item = new TreeItem(this, after, QString(), init);
    }

    setItemWidget(item, 0, new SeparatorWidget);
    return item;
}

void TreeView::fillBranch(MenuFolderInfo *folderInfo, TreeItem *parent)
{
    QString relPath = parent ? parent->directory() : QString();
    TreeItem *after = 0;
    foreach (MenuInfo *info, folderInfo->initialLayout)
    {
       MenuEntryInfo *entry = dynamic_cast<MenuEntryInfo*>(info);
       if (entry)
       {
          after = createTreeItem(parent, after, entry);
          continue;
       }

       MenuFolderInfo *subFolder = dynamic_cast<MenuFolderInfo*>(info);
       if (subFolder)
       {
          after = createTreeItem(parent, after, subFolder);
          continue;
       }
       MenuSeparatorInfo *separator = dynamic_cast<MenuSeparatorInfo*>(info);
       if (separator)
       {
          after = createTreeItem(parent, after, separator);
          continue;
       }
    }
}

void TreeView::closeAllItems(QTreeWidgetItem *item)
{
    item->setExpanded(false);
    for (int i = 0; i < item->childCount(); ++i) {
        closeAllItems(item->child(i));
    }
}

TreeItem *TreeView::expandPath(TreeItem *item, const QString &path)
{
   int i = path.indexOf("/");
   QString subMenu = path.left(i+1);
   QString restMenu = path.mid(i+1);

   for (int i = 0; i < item->childCount(); ++i) {
       TreeItem *childItem = dynamic_cast<TreeItem *>(item->child(i));
       if (!childItem) {
           continue;
       }

       MenuFolderInfo *folderInfo = childItem->folderInfo();
       if (folderInfo && (folderInfo->id == subMenu)) {
           childItem->setExpanded(true);
           if (!restMenu.isEmpty()) {
               return expandPath(childItem, restMenu);
           } else {
               return childItem;
           }
       }
   }

   return 0;
}

void TreeView::selectMenu(const QString &menu)
{
   for (int i = 0; i < topLevelItemCount(); ++i) {
       closeAllItems(topLevelItem(i));
   }

   if (menu.length() <= 1)
   {
      setCurrentItem(topLevelItem(0));
      clearSelection();
      return; // Root menu
   }

   QString restMenu = menu;
   if ( menu.startsWith( '/' ) )
       restMenu = menu.mid(1);
   if (!restMenu.endsWith('/'))
       restMenu += '/';

   TreeItem *item = 0;
   int i = restMenu.indexOf("/");
   QString subMenu = restMenu.left(i+1);
   restMenu = restMenu.mid(i+1);

   for (int i = 0; i < topLevelItemCount(); ++i) {
       item = dynamic_cast<TreeItem *>(topLevelItem(i));
       if (!item) {
           continue;
       }

       MenuFolderInfo *folderInfo = item->folderInfo();
       if (folderInfo && (folderInfo->id == subMenu)) {
           if (!restMenu.isEmpty()) {
               item = expandPath(item, restMenu);
           }
           break;
       }
   }

   if (item)
   {
      setCurrentItem(item);
      scrollToItem(item);
   }
}

void TreeView::selectMenuEntry(const QString &menuEntry)
{
    TreeItem *item = static_cast<TreeItem *>(selectedItem());
    if (!item) {
        item = static_cast<TreeItem *>(currentItem());
    }

    if (!item) {
        return;
    }

    QTreeWidgetItem *parent = item->parent();
    if (parent) {
        for (int i = 0; i < parent->childCount(); ++i) {
            TreeItem *item = dynamic_cast<TreeItem *>(parent->child(i));
            if (!item || item->isDirectory()) {
                continue;
            }

            MenuEntryInfo *entry = item->entryInfo();
            if (entry && entry->menuId() == menuEntry) {
                setCurrentItem(item);
                scrollToItem(item);
                return;
            }
        }
    } else {
        // top level
        for (int i = 0; i < topLevelItemCount(); ++i) {
            TreeItem *item = dynamic_cast<TreeItem *>(topLevelItem(i));
            if (!item || item->isDirectory()) {
                continue;
            }

            MenuEntryInfo *entry = item->entryInfo();
            if (entry && entry->menuId() == menuEntry) {
                setCurrentItem(item);
                scrollToItem(item);
                return;
            }
        }
    }
}

void TreeView::itemSelected(QTreeWidgetItem *item)
{
    // ensure the item is visible as selected
    setItemSelected(item, true);

    TreeItem *_item = static_cast<TreeItem*>(item);
    TreeItem *parentItem = 0;
    bool selected = false;
    bool dselected = false;
    if (_item) {
        selected = true;
        dselected = _item->isHiddenInMenu();
        parentItem = getParentItem(_item);
    }

    // change actions activation
    m_ac->action(CUT_ACTION_NAME)->setEnabled(selected);
    m_ac->action(COPY_ACTION_NAME)->setEnabled(selected);
    m_ac->action(PASTE_ACTION_NAME)->setEnabled(m_clipboard != 0);

    if (m_ac->action(DELETE_ACTION_NAME)) {
        m_ac->action(DELETE_ACTION_NAME)->setEnabled(selected && !dselected);
    }

    m_ac->action(SORT_BY_NAME_ACTION_NAME)->setEnabled(selected && _item->isDirectory() && (_item->childCount() > 0));
    m_ac->action(SORT_BY_DESCRIPTION_ACTION_NAME)->setEnabled(m_ac->action(SORT_BY_NAME_ACTION_NAME)->isEnabled());

    m_ac->action(MOVE_UP_ACTION_NAME)->setEnabled(selected && (parentItem->indexOfChild(_item) > 0));
    m_ac->action(MOVE_DOWN_ACTION_NAME)->setEnabled(selected && (parentItem->indexOfChild(_item) < parentItem->childCount() - 1));

    if (!item) {
        emit disableAction();
        return;
    }

    if (_item->isDirectory()) {
       emit entrySelected(_item->folderInfo());
    } else {
       emit entrySelected(_item->entryInfo());
    }
}

void TreeView::currentDataChanged(MenuFolderInfo *folderInfo)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if (item == 0 || folderInfo == 0) {
        return;
    }

    item->setName(folderInfo->caption);
    item->setIcon(0, appIcon(folderInfo->icon));
}

void TreeView::currentDataChanged(MenuEntryInfo *entryInfo)
{
    TreeItem *item = (TreeItem*)selectedItem();
    if (item == 0 || entryInfo == 0) {
        return;
    }

    QString name;

    if (m_detailedMenuEntries && entryInfo->description.length() != 0) {
        if (m_detailedEntriesNamesFirst) {
            name = entryInfo->caption + " (" + entryInfo->description + ')';
        } else {
            name = entryInfo->description + " (" + entryInfo->caption + ')';
        }
    } else {
        name = entryInfo->caption;
    }

    item->setName(name);
    item->setIcon(0, appIcon(entryInfo->icon));
}

QStringList TreeView::fileList(const QString& rPath)
{
    QString relativePath = rPath;

    // truncate "/.directory"
    int pos = relativePath.lastIndexOf("/.directory");
    if (pos > 0) relativePath.truncate(pos);

    QStringList filelist;

    // loop through all resource dirs and build a file list
    const QStringList resdirlist = KGlobal::dirs()->resourceDirs("apps");
    for (QStringList::ConstIterator it = resdirlist.constBegin(); it != resdirlist.constEnd(); ++it)
    {
        QDir dir((*it) + '/' + relativePath);
        if(!dir.exists()) continue;

        dir.setFilter(QDir::Files);
        dir.setNameFilters(QStringList() << "*.desktop;*.kdelnk");

        // build a list of files
        const QStringList files = dir.entryList();
        for (QStringList::ConstIterator it = files.constBegin(); it != files.constEnd(); ++it) {
            // does not work?!
            //if (filelist.contains(*it)) continue;

            if (relativePath.isEmpty()) {
                filelist.removeAll(*it); // hack
                filelist.append(*it);
            }
            else {
                filelist.removeAll(relativePath + '/' + *it); //hack
                filelist.append(relativePath + '/' + *it);
            }
        }
    }
    return filelist;
}

QStringList TreeView::dirList(const QString& rPath)
{
    QString relativePath = rPath;

    // truncate "/.directory"
    int pos = relativePath.lastIndexOf("/.directory");
    if (pos > 0) relativePath.truncate(pos);

    QStringList dirlist;

    // loop through all resource dirs and build a subdir list
    const QStringList resdirlist = KGlobal::dirs()->resourceDirs("apps");
    for (QStringList::ConstIterator it = resdirlist.constBegin(); it != resdirlist.constEnd(); ++it)
    {
        QDir dir((*it) + '/' + relativePath);
        if(!dir.exists()) continue;
        dir.setFilter(QDir::Dirs);

        // build a list of subdirs
        const QStringList subdirs = dir.entryList();
        for (QStringList::ConstIterator it = subdirs.constBegin(); it != subdirs.constEnd(); ++it) {
            if ((*it) == "." || (*it) == "..") continue;
            // does not work?!
            // if (dirlist.contains(*it)) continue;

            if (relativePath.isEmpty()) {
                dirlist.removeAll(*it); //hack
                dirlist.append(*it);
            }
            else {
                dirlist.removeAll(relativePath + '/' + *it); //hack
                dirlist.append(relativePath + '/' + *it);
            }
        }
    }
    return dirlist;
}

Qt::DropActions TreeView::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TreeView::mimeTypes() const
{
    return m_dropMimeTypes;
}

void TreeView::startDrag(Qt::DropActions supportedActions)
{
    QList<QTreeWidgetItem *> items;
    items << selectedItem();
    QMimeData *data = mimeData(items);
    if (!data) {
        return;
    }

    QDrag *drag = new QDrag(this);
    int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);
    drag->setPixmap(selectedItem()->icon(0).pixmap(iconSize, iconSize));
    drag->setMimeData(data);
    drag->exec(supportedActions, Qt::MoveAction);
}

QMimeData *TreeView::mimeData(const QList<QTreeWidgetItem *> items) const
{
    if (items.isEmpty()) {
        return 0;
    }

    return new MenuItemMimeData(dynamic_cast<TreeItem *>(items.first()));
}

static QString createDesktopFile(const QString &file, QString *menuId, QStringList *excludeList)
{
   QString base = file.mid(file.lastIndexOf('/')+1);
   base = base.left(base.lastIndexOf('.'));

   QRegExp r("(.*)(?=-\\d+)");
   base = (r.indexIn(base) > -1) ? r.cap(1) : base;

   QString result = KService::newServicePath(true, base, menuId, excludeList);
   excludeList->append(*menuId);
   // Todo for Undo-support: Undo menuId allocation:

   return result;
}

static KDesktopFile *copyDesktopFile(MenuEntryInfo *entryInfo, QString *menuId, QStringList *excludeList)
{
   QString result = createDesktopFile(entryInfo->file(), menuId, excludeList);
   KDesktopFile *df = entryInfo->desktopFile()->copyTo(result);
   df->desktopGroup().deleteEntry("Categories"); // Don't set any categories!

   return df;
}

static QString createDirectoryFile(const QString &file, QStringList *excludeList)
{
   QString base = file.mid(file.lastIndexOf('/')+1);
   base = base.left(base.lastIndexOf('.'));

   QString result;
   int i = 1;
   while(true)
   {
      if (i == 1)
         result = base + ".directory";
      else
         result = base + QString("-%1.directory").arg(i);

      if (!excludeList->contains(result))
      {
         if (KStandardDirs::locate("xdgdata-dirs", result).isEmpty())
            break;
      }
      i++;
   }
   excludeList->append(result);
   result = KStandardDirs::locateLocal("xdgdata-dirs", result);
   return result;
}


bool TreeView::dropMimeData(QTreeWidgetItem *item, int index, const QMimeData *data, Qt::DropAction action)
{
   // get destination folder
    TreeItem *titem = item ? dynamic_cast<TreeItem*>(item) : 0;
    if (item && !titem) {
        return false;
    }

    TreeItem *parentItem = 0;
    QTreeWidgetItem *after = titem;
    // find the parent item and which item the dropped item should go after
    if (titem) {
        if (titem->isDirectory()) {
            parentItem = titem;
            after = titem->child(index);
            if (!after) {
                after = titem->child(titem->childCount() - 1);
            }
        } else {
            parentItem = dynamic_cast<TreeItem *>(titem->parent());
            if (titem->parent() && !parentItem) {
                return false;
            }
        }
    } else if (index > 0) {
        after = topLevelItem(index);
        if (!after) {
            after = topLevelItem(topLevelItemCount() - 1);
        }
    }

    QString folder = parentItem ? parentItem->directory() : "/";
    MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
    kDebug() << "think we're dropping on" << (parentItem ? parentItem->text(0) : "Top Level") <<  index;

    if (!data->hasFormat(s_internalMimeType)) {
        // External drop
        if (!KUrl::List::canDecode(data)) {
            return false;
        }

        KUrl::List urls = KUrl::List::fromMimeData(data);;
        if (urls.isEmpty() || !urls[0].isLocalFile()) {
            return false;
        }

        //FIXME: this should really support multiple DnD
        QString path = urls[0].path();
        if (!path.endsWith(QLatin1String(".desktop"))) {
            return false;
        }

        QString menuId;
        QString result = createDesktopFile(path, &menuId, &m_newMenuIds);
        KDesktopFile orig_df(path);
        KDesktopFile *df = orig_df.copyTo(result);
        df->desktopGroup().deleteEntry("Categories"); // Don't set any categories!

        KService::Ptr s(new KService(df));
        s->setMenuId(menuId);

        MenuEntryInfo *entryInfo = new MenuEntryInfo(s, df);

        QString oldCaption = entryInfo->caption;
        QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
        entryInfo->setCaption(newCaption);

        // Add file to menu
        // m_menuFile->addEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

        // create the TreeItem
        if (parentItem) {
            parentItem->setExpanded(true);
        }

        // update fileInfo data
        parentFolderInfo->add(entryInfo);

        TreeItem *newItem = createTreeItem(parentItem, after, entryInfo, true);
        setCurrentItem(newItem);

        setLayoutDirty(parentItem);
        return true;
    }

    QVariant p(data->data(s_internalMimeType));
    const MenuItemMimeData *itemData = dynamic_cast<const MenuItemMimeData *>(data);
    if (!itemData) {
        return false;
    }

    TreeItem *dragItem = itemData->item();
    if (!dragItem || dragItem == after) {
        return false; // Nothing to do
    }

    //kDebug() << "an internal drag of" << dragItem->text(0) << (parentItem ? parentItem->text(0) : "Top level");
    if (dragItem->isDirectory()) {
        MenuFolderInfo *folderInfo = dragItem->folderInfo();
        if (action == Qt::CopyAction) {
            // FIXME:
            // * Create new .directory file
        } else {
            TreeItem *tmpItem = static_cast<TreeItem*>(parentItem);
            while (tmpItem) {
                if (tmpItem == dragItem) {
                    return false;
                }

                tmpItem = static_cast<TreeItem*>(tmpItem->parent());
            }

            // Remove MenuFolderInfo
            TreeItem *oldParentItem = static_cast<TreeItem*>(dragItem->parent());
            MenuFolderInfo *oldParentFolderInfo = oldParentItem ? oldParentItem->folderInfo() : m_rootFolder;
            oldParentFolderInfo->take(folderInfo);

            // Move menu
            QString oldFolder = folderInfo->fullId;
            QString folderName = folderInfo->id;
            QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
            folderInfo->id = newFolder;

            // Add file to menu
            //m_menuFile->moveMenu(oldFolder, folder + newFolder);
            kDebug() << "moving" << dragItem->text(0) << "to" << folder + newFolder;
            m_menuFile->pushAction(MenuFile::MOVE_MENU, oldFolder, folder + newFolder);

            // Make sure caption is unique
            QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
            if (newCaption != folderInfo->caption) {
                folderInfo->setCaption(newCaption);
            }

            // create the TreeItem
            if (parentItem) {
                parentItem->setExpanded(true);
            }

            // update fileInfo data
            folderInfo->updateFullId(parentFolderInfo->fullId);
            folderInfo->setInUse(true);
            parentFolderInfo->add(folderInfo);

            if (parentItem != oldParentItem) {
                if (oldParentItem) {
                    oldParentItem->takeChild(oldParentItem->indexOfChild(dragItem));
                } else {
                    takeTopLevelItem(indexOfTopLevelItem(dragItem));
                }
            }

            if (parentItem) {
                parentItem->insertChild(after ? parentItem->indexOfChild(after) + 1 : parentItem->childCount(), dragItem);
            } else {
                insertTopLevelItem(after ? indexOfTopLevelItem(after) : topLevelItemCount(), dragItem);
            }

            dragItem->setName(folderInfo->caption);
            dragItem->setDirectoryPath(folderInfo->fullId);
            setCurrentItem(dragItem);
        }
    } else if (dragItem->isEntry()) {
        MenuEntryInfo *entryInfo = dragItem->entryInfo();
        QString menuId = entryInfo->menuId();

        if (action == Qt::CopyAction) {
            // Need to copy file and then add it
            KDesktopFile *df = copyDesktopFile(entryInfo, &menuId, &m_newMenuIds); // Duplicate
            //UNDO-ACTION: NEW_MENU_ID (menuId)

            KService::Ptr s(new KService(df));
            s->setMenuId(menuId);

            entryInfo = new MenuEntryInfo(s, df);

            QString oldCaption = entryInfo->caption;
            QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
            entryInfo->setCaption(newCaption);
        } else {
            del(dragItem, false);
            QString oldCaption = entryInfo->caption;
            QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
            entryInfo->setCaption(newCaption);
            entryInfo->setInUse(true);
        }

        // Add file to menu
        // m_menuFile->addEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

        // create the TreeItem
        if (parentItem) {
            parentItem->setExpanded(true);
        }

        // update fileInfo data
        parentFolderInfo->add(entryInfo);

        TreeItem *newItem = createTreeItem(parentItem, after, entryInfo);
        setCurrentItem(newItem);
    } else  {
        // copying a separator
        if (action != Qt::CopyAction) {
            del(dragItem, false);
        }

        TreeItem *newItem = createTreeItem(parentItem, after, m_separator);
        setCurrentItem(newItem);
    }

    kDebug() << "setting the layout to be dirty at" << parentItem;
    setLayoutDirty(parentItem);
    return true;
}


QTreeWidgetItem *TreeView::selectedItem()
{
    QList<QTreeWidgetItem *> selection = selectedItems();

    if (selection.isEmpty()) {
        return 0;
    }

    return selection.first();
}

void TreeView::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_popupMenu && itemAt(event->pos())) {
        m_popupMenu->exec(event->globalPos());
    }
}

void TreeView::dropEvent(QDropEvent *event)
{
    // this prevents QTreeWidget from interfering with our moves
    QTreeView::dropEvent(event);
}

void TreeView::newsubmenu()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   bool ok;
   QString caption = KInputDialog::getText( i18n( "New Submenu" ),
        i18n( "Submenu name:" ), QString(), &ok, this );

   if (!ok) return;

   QString file = caption;
   file.replace('/', '-');

   file = createDirectoryFile(file, &m_newDirectoryList); // Create

   // get destination folder
   QString folder;

   if(!item)
   {
      parentItem = 0;
      folder.clear();
   }
   else if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
      folder = parentItem->directory();
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
      folder = parentItem ? parentItem->directory() : QString();
   }

   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
   MenuFolderInfo *folderInfo = new MenuFolderInfo();
   folderInfo->caption = parentFolderInfo->uniqueMenuCaption(caption);
   folderInfo->id = m_menuFile->uniqueMenuName(folder, caption, parentFolderInfo->existingMenuIds());
   folderInfo->directoryFile = file;
   folderInfo->icon = "package";
   folderInfo->hidden = false;
   folderInfo->setDirty();

   KDesktopFile *df = new KDesktopFile(file);
   KConfigGroup desktopGroup = df->desktopGroup();
   desktopGroup.writeEntry("Name", folderInfo->caption);
   desktopGroup.writeEntry("Icon", folderInfo->icon);
   df->sync();
   delete df;
   // Add file to menu
   // m_menuFile->addMenu(folder + folderInfo->id, file);
   m_menuFile->pushAction(MenuFile::ADD_MENU, folder + folderInfo->id, file);

   folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;

   // create the TreeItem
   if (parentItem)
      parentItem->setExpanded(true);

   // update fileInfo data
   parentFolderInfo->add(folderInfo);

   TreeItem *newItem = createTreeItem(parentItem, item, folderInfo, true);

   setCurrentItem(newItem);
   setLayoutDirty(parentItem);
}

void TreeView::newitem()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   bool ok;
   QString caption = KInputDialog::getText( i18n( "New Item" ),
        i18n( "Item name:" ), QString(), &ok, this );

   if (!ok) return;

   QString menuId;
   QString file = caption;
   file.replace('/', '-');

   file = createDesktopFile(file, &menuId, &m_newMenuIds); // Create

   KDesktopFile *df = new KDesktopFile(file);
   KConfigGroup desktopGroup = df->desktopGroup();
   desktopGroup.writeEntry("Name", caption);
   desktopGroup.writeEntry("Type", "Application");

   // get destination folder
   QString folder;

   if(!item)
   {
      parentItem = 0;
      folder.clear();
   }
   else if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
      folder = parentItem->directory();
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
      folder = parentItem ? parentItem->directory() : QString();
   }

   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;

   // Add file to menu
   // m_menuFile->addEntry(folder, menuId);
   m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

   KService::Ptr s(new KService(df));
   s->setMenuId(menuId);

   MenuEntryInfo *entryInfo = new MenuEntryInfo(s, df);

   // create the TreeItem
   if(parentItem)
      parentItem->setExpanded(true);

   // update fileInfo data
   parentFolderInfo->add(entryInfo);

   TreeItem *newItem = createTreeItem(parentItem, item, entryInfo, true);

   setCurrentItem(newItem);
   setLayoutDirty(parentItem);
}

void TreeView::newsep()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   if(!item)
   {
      parentItem = 0;
   }
   else if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
   }

   // create the TreeItem
   if(parentItem)
      parentItem->setExpanded(true);

   TreeItem *newItem = createTreeItem(parentItem, item, m_separator, true);

   setCurrentItem(newItem);
   setLayoutDirty(parentItem);
}

void TreeView::cut()
{
    copy( true );

    // Select new current item
    // TODO: is this completely redundant?
    setCurrentItem(currentItem());
}

void TreeView::copy()
{
    copy( false );
}

void TreeView::copy( bool cutting )
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to copy
    if (item == 0) return;

    if (cutting)
       setLayoutDirty((TreeItem*)item->parent());

    // clean up old stuff
    cleanupClipboard();

    // is item a folder or a file?
    if(item->isDirectory())
    {
        QString folder = item->directory();
        if (cutting)
        {
           // Place in clipboard
           m_clipboard = MOVE_FOLDER;
           m_clipboardFolderInfo = item->folderInfo();

           del(item, false);
        }
        else
        {
           // Place in clipboard
           m_clipboard = COPY_FOLDER;
           m_clipboardFolderInfo = item->folderInfo();
        }
    }
    else if (item->isEntry())
    {
        if (cutting)
        {
           // Place in clipboard
           m_clipboard = MOVE_FILE;
           m_clipboardEntryInfo = item->entryInfo();

           del(item, false);
        }
        else
        {
           // Place in clipboard
           m_clipboard = COPY_FILE;
           m_clipboardEntryInfo = item->entryInfo();
        }
    }
    else
    {
        // Place in clipboard
        m_clipboard = COPY_SEPARATOR;
        if (cutting)
           del(item, false);
    }

    m_ac->action(PASTE_ACTION_NAME)->setEnabled(true);
}


void TreeView::paste()
{
   TreeItem *parentItem = 0;
   TreeItem *item = (TreeItem*)selectedItem();

   // nil selected? -> nil to paste to
   if (item == 0) return;

   // is there content in the clipboard?
   if (!m_clipboard) return;

  // get destination folder
   QString folder;

   if(item->isDirectory())
   {
      parentItem = item;
      item = 0;
      folder = parentItem->directory();
   }
   else
   {
      parentItem = static_cast<TreeItem*>(item->parent());
      folder = parentItem ? parentItem->directory() : QString();
   }

   MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
   int command = m_clipboard;
   if ((command == COPY_FOLDER) || (command == MOVE_FOLDER))
   {
      MenuFolderInfo *folderInfo = m_clipboardFolderInfo;
      if (command == COPY_FOLDER)
      {
         // Ugh.. this is hard :)
         // * Create new .directory file
         // Add
      }
      else if (command == MOVE_FOLDER)
      {
         // Move menu
         QString oldFolder = folderInfo->fullId;
         QString folderName = folderInfo->id;
         QString newFolder = m_menuFile->uniqueMenuName(folder, folderName, parentFolderInfo->existingMenuIds());
         folderInfo->id = newFolder;

         // Add file to menu
         // m_menuFile->moveMenu(oldFolder, folder + newFolder);
         m_menuFile->pushAction(MenuFile::MOVE_MENU, oldFolder, folder + newFolder);

         // Make sure caption is unique
         QString newCaption = parentFolderInfo->uniqueMenuCaption(folderInfo->caption);
         if (newCaption != folderInfo->caption)
         {
            folderInfo->setCaption(newCaption);
         }
         // create the TreeItem
         if(parentItem)
             parentItem->setExpanded(true);

         // update fileInfo data
         folderInfo->fullId = parentFolderInfo->fullId + folderInfo->id;
         folderInfo->setInUse(true);
         parentFolderInfo->add(folderInfo);

         TreeItem *newItem = createTreeItem(parentItem, item, folderInfo);

         setCurrentItem(newItem);
      }

      m_clipboard = COPY_FOLDER; // Next one copies.
   }
   else if ((command == COPY_FILE) || (command == MOVE_FILE))
   {
      MenuEntryInfo *entryInfo = m_clipboardEntryInfo;
      QString menuId;

      if (command == COPY_FILE)
      {
         // Need to copy file and then add it
         KDesktopFile *df = copyDesktopFile(entryInfo, &menuId, &m_newMenuIds); // Duplicate

         KService::Ptr s(new KService(df));
         s->setMenuId(menuId);
         entryInfo = new MenuEntryInfo(s, df);

         QString oldCaption = entryInfo->caption;
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption, oldCaption);
         entryInfo->setCaption(newCaption);
      }
      else if (command == MOVE_FILE)
      {
         menuId = entryInfo->menuId();
         m_clipboard = COPY_FILE; // Next one copies.

         QString oldCaption = entryInfo->caption;
         QString newCaption = parentFolderInfo->uniqueItemCaption(oldCaption);
         entryInfo->setCaption(newCaption);
         entryInfo->setInUse(true);
      }
      // Add file to menu
      // m_menuFile->addEntry(folder, menuId);
      m_menuFile->pushAction(MenuFile::ADD_ENTRY, folder, menuId);

      // create the TreeItem
      if(parentItem)
         parentItem->setExpanded(true);

      // update fileInfo data
      parentFolderInfo->add(entryInfo);

      TreeItem *newItem = createTreeItem(parentItem, item, entryInfo, true);

      setCurrentItem(newItem);
   }
   else
   {
      // create separator
      if(parentItem)
         parentItem->setExpanded(true);

      TreeItem *newItem = createTreeItem(parentItem, item, m_separator, true);

      setCurrentItem(newItem);
   }
   setLayoutDirty(parentItem);
}

/**
 * This slot is called from the signal mapper to sort children contained in an item.
 * This item is determinated according to the chosen sort type.
 *
 * @brief Determine which item is to sort, and do it.
 * @param sortCmd Sort type.
 */
void TreeView::sort(const int sortCmd)
{
    // determine the chosen sort type and the selected item
    SortType sortType = (SortType) sortCmd;
    TreeItem *itemToSort;
    if (sortType == SortByName || sortType == SortByDescription) {
        itemToSort = static_cast<TreeItem*>(selectedItem());
    } else if (sortType == SortAllByDescription) {
        sortType = SortByDescription;
        itemToSort = static_cast<TreeItem*>(invisibleRootItem());
    } else /* if (sortType == SortAllByName) */ {
        sortType = SortByName;
        itemToSort = static_cast<TreeItem*>(invisibleRootItem());
    }

    // proceed to the sorting
    sortItem(itemToSort, sortType);
}

/**
 * Sort children of the given item, according to the sort type.
 * The sorting is done on children groups, splited by separator items.
 *
 * @brief Sort item children.
 * @param item Item to sort.
 * @param sortType Sort type.
 */
void TreeView::sortItem(TreeItem *item, const SortType& sortType)
{
    // sort the selected item only if contains children
    if ( (!item->isDirectory()) || (item->childCount() == 0) ) {
        return;
    }

    // remove contained children
    QList<QTreeWidgetItem*> children = item->takeChildren();

    // sort children groups, splited by separator items
    QList<QTreeWidgetItem*>::iterator startIt = children.begin();
    QList<QTreeWidgetItem*>::iterator currentIt = children.begin();
    while (currentIt != children.end()) {
        TreeItem *child = static_cast<TreeItem*>(*currentIt);
        // if it's a separator, sort previous items and continue on following items
        if (child->isSeparator() && startIt != currentIt) {
            sortItemChildren(startIt, currentIt, sortType);
            startIt = currentIt + 1;
        }
        ++currentIt;
    }
    sortItemChildren(startIt, currentIt, sortType);

    // insert sorted children in the tree
    item->addChildren(children);
    foreach (QTreeWidgetItem *child, children) {
        // recreate item widget for separators
        TreeItem *treeItem = static_cast<TreeItem*>(child);
        if (treeItem->isSeparator()) {
            setItemWidget(treeItem, 0, new SeparatorWidget);
        }

        // try to sort sub-children
        sortItem(static_cast<TreeItem*>(child), sortType);
    }

    // flag current item as dirty
    TreeItem *itemToFlagAsDirty = item;
    // if tree root item, set the entire layout as dirty
    if (item == invisibleRootItem()) {
        itemToFlagAsDirty = 0;
    }
    setLayoutDirty(itemToFlagAsDirty);
}

/**
 * Sort a children range defined with two list iterators, according to the sort type.
 *
 * @brief Sort a children range.
 * @param begin First child iterator.
 * @param end Last child iterator (exclusive, pointed child won't be affected).
 * @param sortType Sort type.
 */
void TreeView::sortItemChildren(const QList<QTreeWidgetItem*>::iterator& begin, const QList<QTreeWidgetItem*>::iterator& end, const SortType& sortType)
{
    // sort by name
    if (sortType == SortByName) {
        qSort(begin, end, TreeItem::itemNameLessThan);
    }
    // sort by description
    else if (sortType == SortByDescription) {
        qSort(begin, end, TreeItem::itemDescriptionLessThan);
    }
}

/**
 * @brief Move up the selected item.
 */
void TreeView::moveUpItem() {
    moveUpOrDownItem(true);
}

/**
 * @brief Move down the selected item.
 */
void TreeView::moveDownItem() {
    moveUpOrDownItem(false);
}

/**
 * Move the selected item on desired direction (up or down).
 *
 * @brief Move up/down the selected item.
 * @param isMovingUpAction True to move up, false to move down.
 */
void TreeView::moveUpOrDownItem(bool isMovingUpAction)
{
    // get the selected item and its parent
    TreeItem *sourceItem = static_cast<TreeItem*>(selectedItem());
    if (!sourceItem)
       return;
    TreeItem *parentItem = getParentItem(sourceItem);

    // get selected item index
    int sourceItemIndex = parentItem->indexOfChild(sourceItem);

    // find the second item to swap
    TreeItem *destItem = 0;
    int destIndex;
    if (isMovingUpAction) {
        destIndex = sourceItemIndex - 1;
        destItem = static_cast<TreeItem*>(parentItem->child(destIndex));
    }
    else {
        destIndex = sourceItemIndex + 1;
        destItem = static_cast<TreeItem*>(parentItem->child(destIndex));
    }

    // swap items
    parentItem->removeChild(sourceItem);
    parentItem->insertChild(destIndex, sourceItem);

    // recreate item widget for separators
    if (sourceItem->isSeparator()) {
        setItemWidget(sourceItem, 0, new SeparatorWidget);
    }
    if (destItem->isSeparator()) {
        setItemWidget(destItem, 0, new SeparatorWidget);
    }

    // set the focus on the source item
    setCurrentItem(sourceItem);

    // flag parent item as dirty (if the parent is the root item, set the entire layout as dirty)
    if (parentItem == invisibleRootItem()) {
        parentItem = 0;
    }
    setLayoutDirty(parentItem);
}

/**
 * For a given item, return its parent. For top items, return the invisible root item.
 *
 * @brief Get the parent item.
 * @param item Item.
 * @return Parent item.
 */
TreeItem* TreeView::getParentItem(QTreeWidgetItem *item) const
{
    QTreeWidgetItem *parentItem = item->parent();
    if (!parentItem) {
        parentItem = invisibleRootItem();
    }
    return static_cast<TreeItem*>(parentItem);
}

void TreeView::del()
{
    TreeItem *item = (TreeItem*)selectedItem();

    // nil selected? -> nil to delete
    if (item == 0) return;

    del(item, true);

    // Select new current item
    // TODO: is this completely redundant?
    setCurrentItem(currentItem());
}

void TreeView::del(TreeItem *item, bool deleteInfo)
{
    TreeItem *parentItem = static_cast<TreeItem*>(item->parent());
    // is file a .directory or a .desktop file
    if(item->isDirectory())
    {
        if ( KMessageBox::warningYesNo(this, i18n("All submenus of '%1' will be removed. Do you want to continue?", item->name() ) ) == KMessageBox::No )
             return;

        MenuFolderInfo *folderInfo = item->folderInfo();

        // Remove MenuFolderInfo
        MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
        parentFolderInfo->take(folderInfo);
        folderInfo->setInUse(false);

        if (m_clipboard == COPY_FOLDER && (m_clipboardFolderInfo == folderInfo))
        {
           // Copy + Del == Cut
           m_clipboard = MOVE_FOLDER; // Clipboard now owns folderInfo

        }
        else
        {
           if (folderInfo->takeRecursive(m_clipboardFolderInfo))
              m_clipboard = MOVE_FOLDER; // Clipboard now owns m_clipboardFolderInfo

           if (deleteInfo)
              delete folderInfo; // Delete folderInfo
        }

        // Remove from menu
        // m_menuFile->removeMenu(item->directory());
        m_menuFile->pushAction(MenuFile::REMOVE_MENU, item->directory(), QString());

        // Remove tree item
        delete item;
    }
    else if (item->isEntry())
    {
        MenuEntryInfo *entryInfo = item->entryInfo();
        QString menuId = entryInfo->menuId();

        // Remove MenuFolderInfo
        MenuFolderInfo *parentFolderInfo = parentItem ? parentItem->folderInfo() : m_rootFolder;
        parentFolderInfo->take(entryInfo);
        entryInfo->setInUse(false);

        if (m_clipboard == COPY_FILE && (m_clipboardEntryInfo == entryInfo))
        {
           // Copy + Del == Cut
           m_clipboard = MOVE_FILE; // Clipboard now owns entryInfo
        }
        else
        {
           if (deleteInfo)
              delete entryInfo; // Delete entryInfo
        }

        // Remove from menu
        QString folder = parentItem ? parentItem->directory() : QString();
        // m_menuFile->removeEntry(folder, menuId);
        m_menuFile->pushAction(MenuFile::REMOVE_ENTRY, folder, menuId);

        // Remove tree item
        delete item;
    }
    else
    {
        // Remove separator
        delete item;
    }

    setLayoutDirty(parentItem);
}

void TreeView::cleanupClipboard() {
    if (m_clipboard == MOVE_FOLDER)
       delete m_clipboardFolderInfo;
    m_clipboardFolderInfo = 0;

    if (m_clipboard == MOVE_FILE)
       delete m_clipboardEntryInfo;
    m_clipboardEntryInfo = 0;

    m_clipboard = 0;
}

static QStringList extractLayout(QTreeWidget *tree, QTreeWidgetItem *parent)
{
    QStringList layout;
    if (!parent && !tree) {
        return layout;
    }

    bool firstFolder = true;
    bool firstEntry = true;
    int max = parent ? parent->childCount() : tree->topLevelItemCount();
    for (int i = 0; i < max; ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(parent ? parent->child(i) : tree->topLevelItem(i));
        if (!item) {
            continue;
        }

        if (item->isDirectory()) {
            if (firstFolder) {
                firstFolder = false;
                layout << ":M"; // Add new folders here...
            }
            layout << (item->folderInfo()->id);
        } else if (item->isEntry()) {
            if (firstEntry) {
                firstEntry = false;
                layout << ":F"; // Add new entries here...
            }
            layout << (item->entryInfo()->menuId());
        } else {
            layout << ":S";
        }
    }

    return layout;
}

void TreeItem::saveLayout(MenuFile *menuFile)
{
    if (m_layoutDirty) {
        QStringList layout = extractLayout(0, this);
        menuFile->setLayout(folderInfo()->fullId, layout);
        m_layoutDirty = false;
    }

    for (int i = 0; i < childCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(child(i));
        if (item) {
            item->saveLayout(menuFile);
        }
    }
}

void TreeView::saveLayout()
{
    if (m_layoutDirty) {
       QStringList layout = extractLayout(this, 0);
       m_menuFile->setLayout(m_rootFolder->fullId, layout);
       m_layoutDirty = false;
    }

    for (int i = 0; i < topLevelItemCount(); ++i) {
        TreeItem *item = dynamic_cast<TreeItem *>(topLevelItem(i));
        if (item) {
            item->saveLayout(m_menuFile);
        }
    }
}

bool TreeView::save()
{
    saveLayout();
    m_rootFolder->save(m_menuFile);

    bool success = m_menuFile->performAllActions();

    m_newMenuIds.clear();
    m_newDirectoryList.clear();

    if (success)
    {
       KBuildSycocaProgressDialog::rebuildKSycoca(this);
    }
    else
    {
       KMessageBox::sorry(this, "<qt>"+i18n("Menu changes could not be saved because of the following problem:")+"<br><br>"+
                                m_menuFile->error()+"</qt>");
    }

    sendReloadMenu();

    return success;
}

void TreeView::setLayoutDirty(TreeItem *parentItem)
{
    if (parentItem)
       parentItem->setLayoutDirty();
    else
       m_layoutDirty = true;
}

bool TreeView::isLayoutDirty()
{
   for (int i = 0; i < topLevelItemCount(); ++i) {
       TreeItem *item = dynamic_cast<TreeItem *>(topLevelItem(i));
       if (!item) {
           continue;
       }

       if (item->isLayoutDirty()) {
           return true;
       }
   }

   return false;
}

bool TreeView::dirty()
{
    return m_layoutDirty || m_rootFolder->hasDirt() || m_menuFile->dirty() || isLayoutDirty();
}

void TreeView::findServiceShortcut(const KShortcut&cut, KService::Ptr &service)
{
    service = m_rootFolder->findServiceShortcut(cut);
}

void TreeView::restoreMenuSystem()
{
    if ( KMessageBox::warningYesNo( this, i18n( "Do you want to restore the system menu? Warning: This will remove all custom menus." ) )==KMessageBox::No )
        return;
    QString kmenueditfile = KStandardDirs::locateLocal("xdgconf-menu", "applications-kmenuedit.menu");
    if ( QFile::exists( kmenueditfile ) )
    {
        if ( !QFile::remove( kmenueditfile ) )
            qWarning()<<"Could not delete "<<kmenueditfile;
    }

    QString xdgdir = KGlobal::dirs()->KStandardDirs::localxdgdatadir();
    if ( !KIO::NetAccess::del( QString(xdgdir + "/applications") , this) )
        qWarning()<<"Could not delete dir :"<<( xdgdir+"/applications" );
    if ( !KIO::NetAccess::del( QString(xdgdir +"/desktop-directories") , this) )
        qWarning()<<"Could not delete dir :"<<( xdgdir + "/desktop-directories");

    KBuildSycocaProgressDialog::rebuildKSycoca(this);
    clear();
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;

    m_layoutDirty = false;
    m_newMenuIds.clear();
    m_newDirectoryList.clear();
    m_menuFile->restoreMenuSystem(kmenueditfile);

    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;

    readMenuFolderInfo();
    fill();
    sendReloadMenu();
    emit disableAction();
    emit entrySelected(( MenuEntryInfo* ) 0 );
}

void TreeView::updateTreeView(bool showHidden)
{
    m_showHidden = showHidden;
    clear();
    cleanupClipboard();
    delete m_rootFolder;
    delete m_separator;

    m_layoutDirty = false;
    m_newMenuIds.clear();
    m_newDirectoryList.clear();

    m_rootFolder = new MenuFolderInfo;
    m_separator = new MenuSeparatorInfo;

    readMenuFolderInfo();
    fill();
    sendReloadMenu();
    emit disableAction();
    emit entrySelected(( MenuEntryInfo* ) 0 );
}

void TreeView::sendReloadMenu()
{
    QDBusMessage message = QDBusMessage::createSignal("/kickoff", "org.kde.plasma", "reloadMenu");
    QDBusConnection::sessionBus().send(message);
}

MenuItemMimeData::MenuItemMimeData(TreeItem *item)
    : QMimeData(),
      m_item(item)
{
}

TreeItem *MenuItemMimeData::item() const
{
    return m_item;
}

QStringList MenuItemMimeData::formats() const
{
    QStringList formats;
    if (!m_item) {
        return formats;
    }

    formats << s_internalMimeType;
    return formats;
}

bool MenuItemMimeData::hasFormat(const QString &mimeType) const
{
    return m_item && mimeType == s_internalMimeType;
}

QVariant MenuItemMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
    Q_UNUSED(type);

    if (m_item && mimeType == s_internalMimeType) {
        return qVariantFromValue<TreeItem*>(m_item);
    }

    return QVariant();
}

