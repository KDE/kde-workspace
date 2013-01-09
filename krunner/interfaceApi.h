/*
 *   Copyright 2013 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef INTERFACEAPI_H
#define INTERFACEAPI_H

#include <QObject>
#include <QStringList>

namespace Plasma
{
    class Package;
} // namespace Plasma

class InterfaceApi : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList runners READ runners WRITE setRunners NOTIFY runnersChanged)
    Q_PROPERTY(QString singleRunnerId READ singleRunnerId WRITE setSingleRunnerId NOTIFY singleRunnerIdChanged)

public:
    InterfaceApi(QObject *parent = 0);

    Q_INVOKABLE void toggleConfig();

    void setPackage(Plasma::Package *package);
    Plasma::Package *package() const;

    QStringList runners() const;
    void setRunners(const QStringList &runners);

    QString singleRunnerId() const;
    void setSingleRunnerId(const QString &id);

    Q_INVOKABLE QString filePath(const QString &fileType, const QString &filename) const;
    Q_INVOKABLE QString filePath(const QString &fileType) const;

    void signalClearHistory();
    void query(const QString &query);

Q_SIGNALS:
    void runnersChanged();
    void singleRunnerIdChanged();
    void startQuery(const QString &term);
    void clearHistory();

private:
    QStringList m_runners;
    QString m_singleRunnerId;
    Plasma::Package *m_package;
};

/*
   FIXMES
   no searching unless KAuthorized::authorize(QLatin1String("run_command"))
   preloading of config?
        m_runnerManager->reloadConfiguration(); // pre-load the runners

   setup/tear down of search when the UI is shown or hidden

    // We delay the call to matchSessionComplete until next event cycle
    // This is necessary since we might hide the dialog right before running
    // a match, and the runner might still need to be prepped to
    // succesfully run a match
    QTimer::singleShot(0, m_runnerManager, SLOT(matchSessionComplete()));
*/

#endif

