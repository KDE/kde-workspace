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

#ifndef treeview_h
#define treeview_h

#include <QTreeWidget>
#include <QMimeData>

#include <KService>
#include <KServiceGroup>

class QMenu;
class QDropEvent;
class QSignalMapper;

class KActionCollection;
class KDesktopFile;
class MenuFile;
class MenuFolderInfo;
class MenuEntryInfo;
class MenuSeparatorInfo;
class KShortcut;

static const QString SAVE_ACTION_NAME = "file_save";
static const QString NEW_ITEM_ACTION_NAME = "new_item";
static const QString NEW_SUBMENU_ACTION_NAME = "new_submenu";
static const QString NEW_SEPARATOR_ACTION_NAME = "new_separator";
static const QString CUT_ACTION_NAME = "edit_cut";
static const QString COPY_ACTION_NAME = "edit_copy";
static const QString PASTE_ACTION_NAME = "edit_paste";
static const QString DELETE_ACTION_NAME = "delete";
static const QString SORT_ACTION_NAME = "sort";
static const QString SORT_BY_NAME_ACTION_NAME = "sort_by_name";
static const QString SORT_BY_DESCRIPTION_ACTION_NAME = "sort_by_description";
static const QString SORT_ALL_BY_NAME_ACTION_NAME = "sort_all_by_name";
static const QString SORT_ALL_BY_DESCRIPTION_ACTION_NAME = "sort_all_by_description";
static const QString MOVE_UP_ACTION_NAME = "move_up";
static const QString MOVE_DOWN_ACTION_NAME = "move_down";

class TreeItem : public QTreeWidgetItem
{
public:
    TreeItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, const QString &menuId, bool __init = false);
    TreeItem(QTreeWidget *parent, QTreeWidgetItem *after, const QString &menuId, bool __init = false);
    ~TreeItem();
    static bool itemNameLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2);
    static bool itemDescriptionLessThan(QTreeWidgetItem *item1, QTreeWidgetItem *item2);

    QString menuId() const { return m_menuId; }

    QString directory() const { return m_directoryPath; }
    void setDirectoryPath(const QString& path) { m_directoryPath = path; }

    MenuFolderInfo *folderInfo() { return m_folderInfo; }
    void setMenuFolderInfo(MenuFolderInfo *folderInfo) { m_folderInfo = folderInfo; }

    MenuEntryInfo *entryInfo() const { return m_entryInfo; }
    void setMenuEntryInfo(MenuEntryInfo *entryInfo) { m_entryInfo = entryInfo; }

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString description() const;

    bool isDirectory() const { return m_folderInfo; }
    bool isEntry() const { return m_entryInfo; }
    bool isSeparator() const { return !isDirectory() && !isEntry(); }

    bool isHiddenInMenu() const { return m_hidden; }
    void setHiddenInMenu(bool b);

    bool isLayoutDirty() const;
    void setLayoutDirty() { m_layoutDirty = true; }
    void saveLayout(MenuFile *menuFile);

    void load();

    //virtual void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
    void update();

    bool m_hidden : 1;
    bool m_init : 1;
    bool m_layoutDirty : 1;
    QString m_menuId;
    QString m_name;
    QString m_directoryPath;
    MenuFolderInfo *m_folderInfo;
    MenuEntryInfo *m_entryInfo;
};

class TreeView : public QTreeWidget
{
    friend class TreeItem;
    Q_OBJECT
public:
    explicit TreeView(KActionCollection *ac, QWidget *parent=0, const char *name=0);
    ~TreeView();

    void readMenuFolderInfo(MenuFolderInfo *folderInfo=0, KServiceGroup::Ptr folder=KServiceGroup::Ptr(), const QString &prefix=QString());
    void setViewMode(bool showHidden);
    bool save();

    bool dirty();

    void selectMenu(const QString &menu);
    void selectMenuEntry(const QString &menuEntry);

    void restoreMenuSystem();

    void updateTreeView(bool showHidden);

public Q_SLOTS:
    void currentDataChanged(MenuFolderInfo *folderInfo);
    void currentDataChanged(MenuEntryInfo *entryInfo);
    void findServiceShortcut(const KShortcut&, KService::Ptr &);

Q_SIGNALS:
    void entrySelected(MenuFolderInfo *folderInfo);
    void entrySelected(MenuEntryInfo *entryInfo);
    void disableAction();

protected Q_SLOTS:
    void itemSelected(QTreeWidgetItem *);
    bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);

    void newsubmenu();
    void newitem();
    void newsep();

    void cut();
    void copy();
    void paste();
    void del();
    void sort(const int sortCmd);
    void moveUpItem();
    void moveDownItem();

protected:
    enum SortType {
    SortByName = 0,
    SortByDescription,
    SortAllByName,
    SortAllByDescription
    };

    void contextMenuEvent(QContextMenuEvent *event);
    void dropEvent(QDropEvent *event);
    void startDrag(Qt::DropActions supportedActions);
    QTreeWidgetItem *selectedItem();
    TreeItem *createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuFolderInfo *folderInfo, bool _init = false);
    TreeItem *createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuEntryInfo *entryInfo, bool _init = false);
    TreeItem *createTreeItem(TreeItem *parent, QTreeWidgetItem *after, MenuSeparatorInfo *sepInfo, bool _init = false);

    void del(TreeItem *, bool deleteInfo);
    void fill();
    void fillBranch(MenuFolderInfo *folderInfo, TreeItem *parent);
    QString findName(KDesktopFile *df, bool deleted);
    void sortItem(TreeItem *item, const SortType& sortType);
    void sortItemChildren(const QList<QTreeWidgetItem*>::iterator& begin, const QList<QTreeWidgetItem*>::iterator& end, const SortType& sortType);
    TreeItem* getParentItem(QTreeWidgetItem *item) const;
    void moveUpOrDownItem(bool isMovingUpAction);

    void closeAllItems(QTreeWidgetItem *item);
    TreeItem *expandPath(TreeItem *item, const QString &path);

    // moving = src will be removed later
    void copy( bool moving );

    void cleanupClipboard();

    bool isLayoutDirty();
    void setLayoutDirty(TreeItem *);
    void saveLayout();

    QStringList fileList(const QString& relativePath);
    QStringList dirList(const QString& relativePath);

    virtual QStringList mimeTypes() const;
    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const;
    Qt::DropActions supportedDropActions() const;

    void sendReloadMenu();

private:
    KActionCollection *m_ac;
    QMenu             *m_popupMenu;
    int                m_clipboard;
    MenuFolderInfo    *m_clipboardFolderInfo;
    MenuEntryInfo     *m_clipboardEntryInfo;
    bool               m_showHidden;
    MenuFile          *m_menuFile;
    MenuFolderInfo    *m_rootFolder;
    MenuSeparatorInfo *m_separator;
    QStringList        m_newMenuIds;
    QStringList        m_newDirectoryList;
    bool               m_layoutDirty;
    bool               m_detailedMenuEntries;
    bool               m_detailedEntriesNamesFirst;
    QStringList        m_dropMimeTypes;
    QSignalMapper     *m_sortSignalMapper;
};

class MenuItemMimeData : public QMimeData
{
public:
    MenuItemMimeData(TreeItem *item);
    virtual QStringList formats() const;
    virtual bool hasFormat(const QString &mimeType) const;
    TreeItem *item() const;

protected:
    virtual QVariant retrieveData(const QString &mimeType, QVariant::Type type) const;

private:
    TreeItem *m_item;
};

Q_DECLARE_METATYPE(TreeItem *)

#endif
