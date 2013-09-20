/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef ICON_APPLET_H
#define ICON_APPLET_H

#include <KMimeType>
#include <QUrl>
#include <KDirWatch>
#include <KService>

#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QGraphicsLinearLayout>

#include <Plasma/Applet>
//#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

class KPropertiesDialog;

class IconApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        IconApplet(QObject *parent, const QVariantList &args);
        ~IconApplet();

        void init();
        void setUrl(const QUrl& url, bool fromConfigDialog = false);

    public Q_SLOTS:
        void openUrl();
        void updateDesktopFile();
        void configChanged();

    protected:
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void showConfigurationInterface();
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

    private Q_SLOTS:
        void acceptedPropertiesDialog();
        void cancelledPropertiesDialog();
        void delayedDestroy();
        void checkExistenceOfUrl();
        void checkService(const QStringList &service);
        void iconSizeChanged(int group);

    private:
        //dropUrls from DolphinDropController
        void dropUrls(const QList<QUrl>& urls,
                      const QUrl& destination,
                      Qt::KeyboardModifiers modifier);

        QString m_text;
        QString m_genericName;
        QWeakPointer<KPropertiesDialog> m_dialog;
        QUrl m_url;
        QUrl m_configTarget;
        KDirWatch *m_watcher;
        QSize m_lastFreeSize;
        KService::Ptr m_service;
        bool m_hasDesktopFile;
	QGraphicsLinearLayout *m_plasmoidLayout;
	Plasma::DeclarativeWidget *m_declarativeWidget;
};

K_EXPORT_PLASMA_APPLET(icon, IconApplet)

#endif
