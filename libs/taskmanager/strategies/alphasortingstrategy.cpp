/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "alphasortingstrategy.h"

#include <QMap>
#include <QString>
#include <QtAlgorithms>
#include <QList>

#include <KDebug>

#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"


namespace TaskManager
{

AlphaSortingStrategy::AlphaSortingStrategy(QObject *parent)
    : AbstractSortingStrategy(parent)
{
    setType(GroupManager::AlphaSorting);
}

void AlphaSortingStrategy::sortItems(ItemList &items)
{
    GroupManager *gm = qobject_cast<GroupManager *>(parent());
    bool         separateLaunchers = !gm || gm->separateLaunchers();

    //kDebug();
    QMap<QString, AbstractGroupableItem*> map;
    QMap<QString, AbstractGroupableItem*> launcherMap;

    foreach (AbstractGroupableItem * groupable, items) {
        if (!groupable) {
            continue;
        }
        
        if (groupable->itemType() == GroupItemType) {
            map.insertMulti(groupable->name().toLower(), groupable);
            continue;
        } else if (groupable->itemType() == LauncherItemType) {
            if (separateLaunchers) {
                launcherMap.insertMulti(groupable->name().toLower(), groupable);
            } else {
                map.insertMulti(groupable->name().toLower(), groupable);
            }
            continue;
        }


        TaskItem *item = qobject_cast<TaskItem*>(groupable);
        if (!item)  {
            kDebug() << "Wrong object type";
            continue;
        }

        if (!item->task()) {
            kDebug() << "Null Pointer";
            continue;
        }

        //sort by programname not windowname
        //kDebug() << "inserting multi item" <<  item->task()->classClass();
        map.insertMulti(item->taskName().toLower(), groupable);
    }

    items.clear();
    if (separateLaunchers) {
        items << launcherMap.values();
    }
    items << map.values();
}

} //namespace

#include "alphasortingstrategy.moc"

