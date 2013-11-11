/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef SCRIPTENGINE
#define SCRIPTENGINE

#include <QScriptEngine>
#include <QScriptValue>

#include "../plasmagenericshell_export.h"

namespace Plasma
{
    class Applet;
    class Containment;
    class Corona;
} // namespace Plasma

namespace WorkspaceScripting
{

class Containment;

class PLASMAGENERICSHELL_EXPORT ScriptEngine : public QScriptEngine
{
    Q_OBJECT

public:
    ScriptEngine(Plasma::Corona *corona, QObject *parent = 0);
    ~ScriptEngine();

    static QStringList pendingUpdateScripts();
    static QStringList defaultLayoutScripts();

    Plasma::Corona *corona() const;
    bool evaluateScript(const QString &script, const QString &path = QString());
    QScriptValue wrap(Plasma::Applet *w);
    virtual QScriptValue wrap(Plasma::Containment *c);
    QScriptValue wrap(Containment *c);
    virtual int defaultPanelScreen() const;

    static bool isPanel(const Plasma::Containment *c);
    static ScriptEngine *envFor(QScriptEngine *engine);

Q_SIGNALS:
    void print(const QString &string);
    void printError(const QString &string);
    void createPendingPanelViews();

private:
    void setupEngine();
    static QString onlyExec(const QString &commandLine);

    // containment accessors
    static QStringList availableContainments(const QString &type);
    static QScriptValue newActivity(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newPanel(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue activities(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue activityById(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue activityForScreen(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue panelById(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue panels(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue fileExists(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue loadTemplate(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue applicationExists(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue defaultApplication(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue applicationPath(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue userDataPath(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue knownWallpaperPlugins(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue configFile(QScriptContext *context, QScriptEngine *engine);

    // helpers
    static QScriptValue createContainment(const QString &type, const QString &defautPlugin,
                                          QScriptContext *context, QScriptEngine *engine);

private Q_SLOTS:
    void exception(const QScriptValue &value);

private:
    Plasma::Corona *m_corona;
    QScriptValue m_scriptSelf;
};

static const int PLASMA_DESKTOP_SCRIPTING_VERSION = 7;
}

#endif

