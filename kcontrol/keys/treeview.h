/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __treeview_h__
#define __treeview_h__

#include <qstring.h>
#include <klistview.h>

class QPopupMenu;
class KActionCollection;
class KDesktopFile;

class AppTreeItem : public KListViewItem
{

public:
    AppTreeItem(QListViewItem *parent, const QString& file);
    AppTreeItem(QListViewItem *parent, QListViewItem *after, const QString& file);
    AppTreeItem(QListView *parent, const QString& file);
    AppTreeItem(QListView *parent, QListViewItem* after, const QString& file);

    QString file() const { return m_file; }
    void setFile(const QString& file) { m_file = file; }
    void setDirectoryPath(const QString& path) { m_directoryPath = path; }

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString accel() const { return m_accel; }
    void setAccel(const QString &accel);

    bool isDirectory() const { return !m_directoryPath.isEmpty(); }

    virtual void setOpen(bool o);

private:
    bool m_init : 1;
    QString m_file;
    QString m_name;
    QString m_directoryPath;
    QString m_accel;
};

class AppTreeView : public KListView
{
    friend class AppTreeItem;
    Q_OBJECT
public:
    AppTreeView(QWidget *parent=0, const char *name=0);
    ~AppTreeView();
    void fill();

public slots:
    void currentChanged(const QString& desktopFile);

signals:
    void entrySelected(const QString&, const QString &, bool);

protected slots:
    void itemSelected(QListViewItem *);

protected:
    void fillBranch(const QString& relPath, AppTreeItem* parent);

    QStringList fileList(const QString& relativePath);
    QStringList dirList(const QString& relativePath);
};


#endif
