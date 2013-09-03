/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#ifndef SWITCHWINDOW_HEADER
#define SWITCHWINDOW_HEADER

#include <QMenu>

#include <KServiceGroup>

#include <plasma/containmentactions.h>

class QAction;
class QMenu;

class AppLauncher : public Plasma::ContainmentActions
{
    Q_OBJECT
    public:
        AppLauncher(QObject* parent, const QVariantList& args);
        ~AppLauncher();

        void init(const KConfigGroup &config);

        QList<QAction*> contextualActions();

    protected:
        void makeMenu(QMenu *menu, const KServiceGroup::Ptr group);

    private:
        KServiceGroup::Ptr m_group;
        QList<QAction *> m_actions;
};


#endif
