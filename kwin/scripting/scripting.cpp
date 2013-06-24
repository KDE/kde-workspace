/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010 Rohan Prabhu <rohan@rohanprabhu.com>
Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

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

#include "scripting.h"
// own
#include "meta.h"
#include "scriptingutils.h"
#include "workspace_wrapper.h"
#include "scripting_model.h"
#include "../client.h"
#include "../thumbnailitem.h"
#include "../options.h"
#include "../workspace.h"
// KDE
#include <kstandarddirs.h>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KPluginInfo>
#include <KDE/KServiceTypeTrader>
#include <kdeclarative.h>
// Qt
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QFutureWatcher>
#include <QSettings>
#include <QtConcurrentRun>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/qdeclarative.h>
#include <QMenu>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

QScriptValue kwinScriptPrint(QScriptContext *context, QScriptEngine *engine)
{
    KWin::AbstractScript *script = qobject_cast<KWin::Script*>(context->callee().data().toQObject());
    if (!script) {
        return engine->undefinedValue();
    }
    QString result;
    QTextStream stream(&result);
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0) {
            stream << " ";
        }
        QScriptValue argument = context->argument(i);
        if (KWin::Client *client = qscriptvalue_cast<KWin::Client*>(argument)) {
            client->print<QTextStream>(stream);
        } else {
            stream << argument.toString();
        }
    }
    script->printMessage(result);

    return engine->undefinedValue();
}

QScriptValue kwinScriptReadConfig(QScriptContext *context, QScriptEngine *engine)
{
    KWin::AbstractScript *script = qobject_cast<KWin::AbstractScript*>(context->callee().data().toQObject());
    if (!script) {
        return engine->undefinedValue();
    }
    if (context->argumentCount() < 1 || context->argumentCount() > 2) {
        kDebug(1212) << "Incorrect number of arguments";
        return engine->undefinedValue();
    }
    const QString key = context->argument(0).toString();
    QVariant defaultValue;
    if (context->argumentCount() == 2) {
        defaultValue = context->argument(1).toVariant();
    }
    return engine->newVariant(script->config().readEntry(key, defaultValue));
}

QScriptValue kwinScriptGlobalShortcut(QScriptContext *context, QScriptEngine *engine)
{
    return KWin::globalShortcut<KWin::AbstractScript*>(context, engine);
}

QScriptValue kwinAssertTrue(QScriptContext *context, QScriptEngine *engine)
{
    return KWin::scriptingAssert<bool>(context, engine, 1, 2, true);
}

QScriptValue kwinAssertFalse(QScriptContext *context, QScriptEngine *engine)
{
    return KWin::scriptingAssert<bool>(context, engine, 1, 2, false);
}

QScriptValue kwinAssertEquals(QScriptContext *context, QScriptEngine *engine)
{
    return KWin::scriptingAssert<QVariant>(context, engine, 2, 3);
}

QScriptValue kwinAssertNull(QScriptContext *context, QScriptEngine *engine)
{
    if (!KWin::validateParameters(context, 1, 2)) {
        return engine->undefinedValue();
    }
    if (!context->argument(0).isNull()) {
        if (context->argumentCount() == 2) {
            context->throwError(QScriptContext::UnknownError, context->argument(1).toString());
        } else {
            context->throwError(QScriptContext::UnknownError,
                                i18nc("Assertion failed in KWin script with given value",
                                      "Assertion failed: %1 is not null", context->argument(0).toString()));
        }
        return engine->undefinedValue();
    }
    return true;
}

QScriptValue kwinAssertNotNull(QScriptContext *context, QScriptEngine *engine)
{
    if (!KWin::validateParameters(context, 1, 2)) {
        return engine->undefinedValue();
    }
    if (context->argument(0).isNull()) {
        if (context->argumentCount() == 2) {
            context->throwError(QScriptContext::UnknownError, context->argument(1).toString());
        } else {
            context->throwError(QScriptContext::UnknownError,
                                i18nc("Assertion failed in KWin script",
                                      "Assertion failed: argument is null"));
        }
        return engine->undefinedValue();
    }
    return true;
}

QScriptValue kwinRegisterScreenEdge(QScriptContext *context, QScriptEngine *engine)
{
    return KWin::registerScreenEdge<KWin::AbstractScript*>(context, engine);
}

QScriptValue kwinRegisterUserActionsMenu(QScriptContext *context, QScriptEngine *engine)
{
    return KWin::registerUserActionsMenu<KWin::AbstractScript*>(context, engine);
}

QScriptValue kwinCallDBus(QScriptContext *context, QScriptEngine *engine)
{
    KWin::AbstractScript *script = qobject_cast<KWin::AbstractScript*>(context->callee().data().toQObject());
    if (!script) {
        context->throwError(QScriptContext::UnknownError, "Internal Error: script not registered");
        return engine->undefinedValue();
    }
    if (context->argumentCount() < 4) {
        context->throwError(QScriptContext::SyntaxError,
                            i18nc("Error in KWin Script",
                                  "Invalid number of arguments. At least service, path, interface and method need to be provided"));
        return engine->undefinedValue();
    }
    if (!KWin::validateArgumentType<QString, QString, QString, QString>(context)) {
        context->throwError(QScriptContext::SyntaxError,
                            i18nc("Error in KWin Script",
                                  "Invalid type. Service, path, interface and method need to be string values"));
        return engine->undefinedValue();
    }
    const QString service = context->argument(0).toString();
    const QString path = context->argument(1).toString();
    const QString interface = context->argument(2).toString();
    const QString method = context->argument(3).toString();
    int argumentsCount = context->argumentCount();
    if (context->argument(argumentsCount-1).isFunction()) {
        --argumentsCount;
    }
    QDBusMessage msg = QDBusMessage::createMethodCall(service, path, interface, method);
    QVariantList arguments;
    for (int i=4; i<argumentsCount; ++i) {
        if (context->argument(i).isArray()) {
            QStringList stringArray = engine->fromScriptValue<QStringList>(context->argument(i));
            arguments << qVariantFromValue(stringArray);
        } else {
            arguments << context->argument(i).toVariant();
        }
    }
    if (!arguments.isEmpty()) {
        msg.setArguments(arguments);
    }
    if (argumentsCount == context->argumentCount()) {
        // no callback, just fire and forget
        QDBusConnection::sessionBus().asyncCall(msg);
    } else {
        // with a callback
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(msg), script);
        watcher->setProperty("callback", script->registerCallback(context->argument(context->argumentCount()-1)));
        QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), script, SLOT(slotPendingDBusCall(QDBusPendingCallWatcher*)));
    }
    return engine->undefinedValue();
}

KWin::AbstractScript::AbstractScript(int id, QString scriptName, QString pluginName, QObject *parent)
    : QObject(parent)
    , m_scriptId(id)
    , m_pluginName(pluginName)
    , m_running(false)
    , m_workspace(new WorkspaceWrapper(this))
{
    m_scriptFile.setFileName(scriptName);
    if (m_pluginName.isNull()) {
        m_pluginName = scriptName;
    }
}

KWin::AbstractScript::~AbstractScript()
{
}

KConfigGroup KWin::AbstractScript::config() const
{
    return KGlobal::config()->group("Script-" + m_pluginName);
}

void KWin::AbstractScript::stop()
{
    deleteLater();
}

void KWin::AbstractScript::printMessage(const QString &message)
{
    kDebug(1212) << scriptFile().fileName() << ":" << message;
    emit print(message);
}

void KWin::AbstractScript::registerShortcut(QAction *a, QScriptValue callback)
{
    m_shortcutCallbacks.insert(a, callback);
    connect(a, SIGNAL(triggered(bool)), SLOT(globalShortcutTriggered()));
}

void KWin::AbstractScript::globalShortcutTriggered()
{
    callGlobalShortcutCallback<KWin::AbstractScript*>(this, sender());
}

bool KWin::AbstractScript::borderActivated(KWin::ElectricBorder edge)
{
    screenEdgeActivated(this, edge);
    return true;
}

void KWin::AbstractScript::installScriptFunctions(QScriptEngine* engine)
{
    // add our print
    QScriptValue printFunc = engine->newFunction(kwinScriptPrint);
    printFunc.setData(engine->newQObject(this));
    engine->globalObject().setProperty("print", printFunc);
    // add read config
    QScriptValue configFunc = engine->newFunction(kwinScriptReadConfig);
    configFunc.setData(engine->newQObject(this));
    engine->globalObject().setProperty("readConfig", configFunc);
    QScriptValue dbusCallFunc = engine->newFunction(kwinCallDBus);
    dbusCallFunc.setData(engine->newQObject(this));
    engine->globalObject().setProperty("callDBus", dbusCallFunc);
    // add global Shortcut
    registerGlobalShortcutFunction(this, engine, kwinScriptGlobalShortcut);
    // add screen edge
    registerScreenEdgeFunction(this, engine, kwinRegisterScreenEdge);
    // add user actions menu register function
    regesterUserActionsMenuFunction(this, engine, kwinRegisterUserActionsMenu);
    // add assertions
    QScriptValue assertTrueFunc = engine->newFunction(kwinAssertTrue);
    engine->globalObject().setProperty("assertTrue", assertTrueFunc);
    engine->globalObject().setProperty("assert", assertTrueFunc);
    QScriptValue assertFalseFunc = engine->newFunction(kwinAssertFalse);
    engine->globalObject().setProperty("assertFalse", assertFalseFunc);
    QScriptValue assertEqualsFunc = engine->newFunction(kwinAssertEquals);
    engine->globalObject().setProperty("assertEquals", assertEqualsFunc);
    QScriptValue assertNullFunc = engine->newFunction(kwinAssertNull);
    engine->globalObject().setProperty("assertNull", assertNullFunc);
    engine->globalObject().setProperty("assertEquals", assertEqualsFunc);
    QScriptValue assertNotNullFunc = engine->newFunction(kwinAssertNotNull);
    engine->globalObject().setProperty("assertNotNull", assertNotNullFunc);
    // global properties
    engine->globalObject().setProperty("KWin", engine->newQMetaObject(&WorkspaceWrapper::staticMetaObject));
    QScriptValue workspace = engine->newQObject(AbstractScript::workspace(), QScriptEngine::QtOwnership,
                                                QScriptEngine::ExcludeSuperClassContents | QScriptEngine::ExcludeDeleteLater);
    engine->globalObject().setProperty("workspace", workspace, QScriptValue::Undeletable);
    // install meta functions
    KWin::MetaScripting::registration(engine);
}

int KWin::AbstractScript::registerCallback(QScriptValue value)
{
    int id = m_callbacks.size();
    m_callbacks.insert(id, value);
    return id;
}

void KWin::AbstractScript::slotPendingDBusCall(QDBusPendingCallWatcher* watcher)
{
    if (watcher->isError()) {
        kDebug(1212) << "Received D-Bus message is error";
        watcher->deleteLater();
        return;
    }
    const int id = watcher->property("callback").toInt();
    QDBusMessage reply = watcher->reply();
    QScriptValue callback (m_callbacks.value(id));
    QScriptValueList arguments;
    foreach (const QVariant &argument, reply.arguments()) {
        arguments << callback.engine()->newVariant(argument);
    }
    callback.call(QScriptValue(), arguments);
    m_callbacks.remove(id);
    watcher->deleteLater();
}

void KWin::AbstractScript::registerUseractionsMenuCallback(QScriptValue callback)
{
    m_userActionsMenuCallbacks.append(callback);
}

QList< QAction * > KWin::AbstractScript::actionsForUserActionMenu(KWin::Client *c, QMenu *parent)
{
    QList<QAction*> returnActions;
    for (QList<QScriptValue>::const_iterator it = m_userActionsMenuCallbacks.constBegin(); it != m_userActionsMenuCallbacks.constEnd(); ++it) {
        QScriptValue callback(*it);
        QScriptValueList arguments;
        arguments << callback.engine()->newQObject(c);
        QScriptValue actions = callback.call(QScriptValue(), arguments);
        if (!actions.isValid() || actions.isUndefined() || actions.isNull()) {
            // script does not want to handle this Client
            continue;
        }
        if (actions.isObject()) {
            QAction *a = scriptValueToAction(actions, parent);
            if (a) {
                returnActions << a;
            }
        }
    }

    return returnActions;
}

QAction *KWin::AbstractScript::scriptValueToAction(QScriptValue &value, QMenu *parent)
{
    QScriptValue titleValue = value.property("text");
    QScriptValue checkableValue = value.property("checkable");
    QScriptValue checkedValue = value.property("checked");
    QScriptValue itemsValue = value.property("items");
    QScriptValue triggeredValue = value.property("triggered");

    if (!titleValue.isValid()) {
        // title not specified - does not make any sense to include
        return NULL;
    }
    const QString title = titleValue.toString();
    const bool checkable = checkableValue.isValid() && checkableValue.toBool();
    const bool checked = checkable && checkedValue.isValid() && checkedValue.toBool();
    // either a menu or a menu item
    if (itemsValue.isValid()) {
        if (!itemsValue.isArray()) {
            // not an array, so cannot be a menu
            return NULL;
        }
        QScriptValue lengthValue = itemsValue.property("length");
        if (!lengthValue.isValid() || !lengthValue.isNumber() || lengthValue.toInteger() == 0) {
            // length property missing
            return NULL;
        }
        return createMenu(title, itemsValue, parent);
    } else if (triggeredValue.isValid()) {
        // normal item
        return createAction(title, checkable, checked, triggeredValue, parent);
    }
    return NULL;
}

QAction *KWin::AbstractScript::createAction(const QString &title, bool checkable, bool checked, QScriptValue &callback, QMenu *parent)
{
    QAction *action = new QAction(title, parent);
    action->setCheckable(checkable);
    action->setChecked(checked);
    // TODO: rename m_shortcutCallbacks
    m_shortcutCallbacks.insert(action, callback);
    connect(action, SIGNAL(triggered(bool)), SLOT(globalShortcutTriggered()));
    connect(action, SIGNAL(destroyed(QObject*)), SLOT(actionDestroyed(QObject*)));
    return action;
}

QAction *KWin::AbstractScript::createMenu(const QString &title, QScriptValue &items, QMenu *parent)
{
    QMenu *menu = new QMenu(title, parent);
    const int length = static_cast<int>(items.property("length").toInteger());
    for (int i=0; i<length; ++i) {
        QScriptValue value = items.property(QString::number(i));
        if (!value.isValid()) {
            continue;
        }
        if (value.isObject()) {
            QAction *a = scriptValueToAction(value, menu);
            if (a) {
                menu->addAction(a);
            }
        }
    }
    return menu->menuAction();
}

void KWin::AbstractScript::actionDestroyed(QObject *object)
{
    // TODO: Qt 5 - change to lambda function
    m_shortcutCallbacks.remove(static_cast<QAction*>(object));
}

KWin::Script::Script(int id, QString scriptName, QString pluginName, QObject* parent)
    : AbstractScript(id, scriptName, pluginName, parent)
    , m_engine(new QScriptEngine(this))
    , m_starting(false)
    , m_agent(new ScriptUnloaderAgent(this))
{
    QDBusConnection::sessionBus().registerObject('/' + QString::number(scriptId()), this, QDBusConnection::ExportScriptableContents | QDBusConnection::ExportScriptableInvokables);
}

KWin::Script::~Script()
{
    QDBusConnection::sessionBus().unregisterObject('/' + QString::number(scriptId()));
}

void KWin::Script::run()
{
    if (running() || m_starting) {
        return;
    }
    m_starting = true;
    QFutureWatcher<QByteArray> *watcher = new QFutureWatcher<QByteArray>(this);
    connect(watcher, SIGNAL(finished()), SLOT(slotScriptLoadedFromFile()));
    watcher->setFuture(QtConcurrent::run(this, &KWin::Script::loadScriptFromFile));
}

QByteArray KWin::Script::loadScriptFromFile()
{
    if (!scriptFile().open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    QByteArray result(scriptFile().readAll());
    scriptFile().close();
    return result;
}

void KWin::Script::slotScriptLoadedFromFile()
{
    QFutureWatcher<QByteArray> *watcher = dynamic_cast< QFutureWatcher< QByteArray>* >(sender());
    if (!watcher) {
        // not invoked from a QFutureWatcher
        return;
    }
    if (watcher->result().isNull()) {
        // do not load empty script
        deleteLater();
        watcher->deleteLater();
        return;
    }
    QScriptValue optionsValue = m_engine->newQObject(options, QScriptEngine::QtOwnership,
                            QScriptEngine::ExcludeSuperClassContents | QScriptEngine::ExcludeDeleteLater);
    m_engine->globalObject().setProperty("options", optionsValue, QScriptValue::Undeletable);
    m_engine->globalObject().setProperty("QTimer", constructTimerClass(m_engine));
    QObject::connect(m_engine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(sigException(QScriptValue)));
    KWin::MetaScripting::supplyConfig(m_engine);
    installScriptFunctions(m_engine);

    QScriptValue ret = m_engine->evaluate(watcher->result());

    if (ret.isError()) {
        sigException(ret);
        deleteLater();
    }

    watcher->deleteLater();
    setRunning(true);
    m_starting = false;
}

void KWin::Script::sigException(const QScriptValue& exception)
{
    QScriptValue ret = exception;
    if (ret.isError()) {
        kDebug(1212) << "defaultscript encountered an error at [Line " << m_engine->uncaughtExceptionLineNumber() << "]";
        kDebug(1212) << "Message: " << ret.toString();
        kDebug(1212) << "-----------------";

        QScriptValueIterator iter(ret);
        while (iter.hasNext()) {
            iter.next();
            qDebug() << " " << iter.name() << ": " << iter.value().toString();
        }
    }
    emit printError(exception.toString());
    stop();
}

KWin::ScriptUnloaderAgent::ScriptUnloaderAgent(KWin::Script *script)
    : QScriptEngineAgent(script->engine())
    , m_script(script)
{
    script->engine()->setAgent(this);
}

void KWin::ScriptUnloaderAgent::scriptUnload(qint64 id)
{
    Q_UNUSED(id)
    m_script->stop();
}

KWin::DeclarativeScript::DeclarativeScript(int id, QString scriptName, QString pluginName, QObject* parent)
    : AbstractScript(id, scriptName, pluginName, parent)
    , m_engine(new QDeclarativeEngine(this))
    , m_component(new QDeclarativeComponent(m_engine, this))
    , m_scene(new QGraphicsScene(this))
{
}

KWin::DeclarativeScript::~DeclarativeScript()
{
}

void KWin::DeclarativeScript::run()
{
    if (running()) {
        return;
    }
    // add read config
    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_engine);
    kdeclarative.initialize();
    kdeclarative.setupBindings();
    installScriptFunctions(kdeclarative.scriptEngine());
    qmlRegisterType<DesktopThumbnailItem>("org.kde.kwin", 0, 1, "DesktopThumbnailItem");
    qmlRegisterType<WindowThumbnailItem>("org.kde.kwin", 0, 1, "ThumbnailItem");
    qmlRegisterType<KWin::ScriptingClientModel::ClientModel>();
    qmlRegisterType<KWin::ScriptingClientModel::SimpleClientModel>("org.kde.kwin", 0, 1, "ClientModel");
    qmlRegisterType<KWin::ScriptingClientModel::ClientModelByScreen>("org.kde.kwin", 0, 1, "ClientModelByScreen");
    qmlRegisterType<KWin::ScriptingClientModel::ClientModelByScreenAndDesktop>("org.kde.kwin", 0, 1, "ClientModelByScreenAndDesktop");
    qmlRegisterType<KWin::ScriptingClientModel::ClientFilterModel>("org.kde.kwin", 0, 1, "ClientFilterModel");
    qmlRegisterType<KWin::Client>();

    m_engine->rootContext()->setContextProperty("options", options);

    m_component->loadUrl(QUrl::fromLocalFile(scriptFile().fileName()));
    if (m_component->isLoading()) {
        connect(m_component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), SLOT(createComponent()));
    } else {
        createComponent();
    }
}

void KWin::DeclarativeScript::createComponent()
{
    if (m_component->isError()) {
        kDebug(1212) << "Component failed to load: " << m_component->errors();
    } else {
        m_scene->addItem(qobject_cast<QDeclarativeItem*>(m_component->create()));
    }
    setRunning(true);
}

KWin::Scripting *KWin::Scripting::s_self = NULL;

KWin::Scripting *KWin::Scripting::create(QObject *parent)
{
    Q_ASSERT(!s_self);
    s_self = new Scripting(parent);
    return s_self;
}

KWin::Scripting::Scripting(QObject *parent)
    : QObject(parent)
    , m_scriptsLock(new QMutex(QMutex::Recursive))
{
    QDBusConnection::sessionBus().registerObject("/Scripting", this, QDBusConnection::ExportScriptableContents | QDBusConnection::ExportScriptableInvokables);
    QDBusConnection::sessionBus().registerService("org.kde.kwin.Scripting");
    connect(Workspace::self(), SIGNAL(configChanged()), SLOT(start()));
    connect(Workspace::self(), SIGNAL(workspaceInitialized()), SLOT(start()));
}

void KWin::Scripting::start()
{
#if 0
    // TODO make this threaded again once KConfigGroup is sufficiently thread safe, bug #305361 and friends
    // perform querying for the services in a thread
    QFutureWatcher<LoadScriptList> *watcher = new QFutureWatcher<LoadScriptList>(this);
    connect(watcher, SIGNAL(finished()), this, SLOT(slotScriptsQueried()));
    watcher->setFuture(QtConcurrent::run(this, &KWin::Scripting::queryScriptsToLoad, pluginStates, offers));
#else
    LoadScriptList scriptsToLoad = queryScriptsToLoad();
    for (LoadScriptList::const_iterator it = scriptsToLoad.constBegin();
            it != scriptsToLoad.constEnd();
            ++it) {
        if (it->first) {
            loadScript(it->second.first, it->second.second);
        } else {
            loadDeclarativeScript(it->second.first, it->second.second);
        }
    }

    runScripts();
#endif
}

LoadScriptList KWin::Scripting::queryScriptsToLoad()
{
    KSharedConfig::Ptr _config = KGlobal::config();
    static bool s_started = false;
    if (s_started) {
        _config->reparseConfiguration();
    } else {
        s_started = true;
    }
    QMap<QString,QString> pluginStates = KConfigGroup(_config, "Plugins").entryMap();
    KService::List offers = KServiceTypeTrader::self()->query("KWin/Script");

    LoadScriptList scriptsToLoad;

    foreach (const KService::Ptr & service, offers) {
        KPluginInfo plugininfo(service);
        const QString value = pluginStates.value(plugininfo.pluginName() + QString::fromLatin1("Enabled"), QString());
        plugininfo.setPluginEnabled(value.isNull() ? plugininfo.isPluginEnabledByDefault() : QVariant(value).toBool());
        const bool javaScript = service->property("X-Plasma-API").toString() == "javascript";
        const bool declarativeScript = service->property("X-Plasma-API").toString() == "declarativescript";
        if (!javaScript && !declarativeScript) {
            continue;
        }

        if (!plugininfo.isPluginEnabled()) {
            if (isScriptLoaded(plugininfo.pluginName())) {
                // unload the script
                unloadScript(plugininfo.pluginName());
            }
            continue;
        }
        const QString pluginName = service->property("X-KDE-PluginInfo-Name").toString();
        const QString scriptName = service->property("X-Plasma-MainScript").toString();
        const QString file = KStandardDirs::locate("data", QLatin1String(KWIN_NAME) + "/scripts/" + pluginName + "/contents/" + scriptName);
        if (file.isNull()) {
            kDebug(1212) << "Could not find script file for " << pluginName;
            continue;
        }
        scriptsToLoad << qMakePair(javaScript, qMakePair(file, pluginName));
    }
    return scriptsToLoad;
}

void KWin::Scripting::slotScriptsQueried()
{
    QFutureWatcher<LoadScriptList> *watcher = dynamic_cast< QFutureWatcher<LoadScriptList>* >(sender());
    if (!watcher) {
        // slot invoked not from a FutureWatcher
        return;
    }

    LoadScriptList scriptsToLoad = watcher->result();
    for (LoadScriptList::const_iterator it = scriptsToLoad.constBegin();
            it != scriptsToLoad.constEnd();
            ++it) {
        if (it->first) {
            loadScript(it->second.first, it->second.second);
        } else {
            loadDeclarativeScript(it->second.first, it->second.second);
        }
    }

    runScripts();
    watcher->deleteLater();
}

bool KWin::Scripting::isScriptLoaded(const QString &pluginName) const
{
    QMutexLocker locker(m_scriptsLock.data());
    foreach (AbstractScript *script, scripts) {
        if (script->pluginName() == pluginName) {
            return true;
        }
    }
    return false;
}

bool KWin::Scripting::unloadScript(const QString &pluginName)
{
    QMutexLocker locker(m_scriptsLock.data());
    foreach (AbstractScript *script, scripts) {
        if (script->pluginName() == pluginName) {
            script->deleteLater();
            return true;
        }
    }
    return false;
}

void KWin::Scripting::runScripts()
{
    QMutexLocker locker(m_scriptsLock.data());
    for (int i = 0; i < scripts.size(); i++) {
        scripts.at(i)->run();
    }
}

void KWin::Scripting::scriptDestroyed(QObject *object)
{
    QMutexLocker locker(m_scriptsLock.data());
    scripts.removeAll(static_cast<KWin::Script*>(object));
}

int KWin::Scripting::loadScript(const QString &filePath, const QString& pluginName)
{
    QMutexLocker locker(m_scriptsLock.data());
    if (isScriptLoaded(pluginName)) {
        return -1;
    }
    const int id = scripts.size();
    KWin::Script *script = new KWin::Script(id, filePath, pluginName, this);
    connect(script, SIGNAL(destroyed(QObject*)), SLOT(scriptDestroyed(QObject*)));
    scripts.append(script);
    return id;
}

int KWin::Scripting::loadDeclarativeScript(const QString& filePath, const QString& pluginName)
{
    QMutexLocker locker(m_scriptsLock.data());
    if (isScriptLoaded(pluginName)) {
        return -1;
    }
    const int id = scripts.size();
    KWin::DeclarativeScript *script = new KWin::DeclarativeScript(id, filePath, pluginName, this);
    connect(script, SIGNAL(destroyed(QObject*)), SLOT(scriptDestroyed(QObject*)));
    scripts.append(script);
    return id;
}

KWin::Scripting::~Scripting()
{
    QDBusConnection::sessionBus().unregisterObject("/Scripting");
    QDBusConnection::sessionBus().unregisterService("org.kde.kwin.Scripting");
    s_self = NULL;
}

QList< QAction * > KWin::Scripting::actionsForUserActionMenu(KWin::Client *c, QMenu *parent)
{
    QList<QAction*> actions;
    foreach (AbstractScript *script, scripts) {
        actions << script->actionsForUserActionMenu(c, parent);
    }
    return actions;
}

#include "scripting.moc"
