/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#ifndef RUNNER_H
#define RUNNER_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QStringList>

#include <KDE/KConfigGroup>
#include <KDE/KService>

#include <plasma/plasma_export.h>
#include <plasma/runnercontext.h>
#include <plasma/querymatch.h>
#include <plasma/version.h>

class KCompletion;

namespace Plasma
{

class Package;
class RunnerScript;
class QueryMatch;
class AbstractRunnerPrivate;

/**
 * An abstract base class for Plasma Runner plugins
 *
 * Be aware that runners have to be thread-safe. This is due to
 * the fact that each runner is executed in its own thread for
 * each new term. Thus, a runner may be executed more than once
 * at the same time.
 */
class PLASMA_EXPORT AbstractRunner : public QObject
{
    Q_OBJECT

    public:
        enum Speed { SlowSpeed,
                     NormalSpeed
                   };

        enum Priority { LowestPriority = 0,
                        LowPriority,
                        NormalPriority,
                        HighPriority,
                        HighestPriority
                      };

        typedef QList<AbstractRunner*> List;

        virtual ~AbstractRunner();

        /**
         * This is the main query method. It should trigger creation of
         * QueryMatch instances through RunnerContext::addInformationalMatch,
         * RunnerContext::addExactMatch, and RunnerContext::addPossibleMatch.
         *
         * If the runner can run precisely the requested term (RunnerContext::query()),
         * it should create an exact match (RunnerContext::addExactMatch).
         * The first runner that creates a QueryMatch will be the
         * default runner. Other runner's matches will be suggested in the
         * interface. Non-exact matches should be offered via RunnerContext::addPossibleMatch.
         *
         * The match will be activated if the user selects it.
         *
         * If this runner's exact match is selected, it will be passed into
         * the run method.
         * @see run
         *
         * Since each runner is executed in its own thread there is no need
         * to return from this method right away, nor to create all matches
         * here.
         */
        virtual void match(Plasma::RunnerContext &context);

        /**
         * Triggers a call to match.
         *
         * @arg globalContext the search context used in executing this match.
         */
        void performMatch(Plasma::RunnerContext &context);

        /**
         * If the runner has options that the user can interact with to modify
         * what happens when run or one of the actions created in fillMatches
         * is called, the runner should return true
         */
        bool hasRunOptions();

        /**
         * If hasRunOptions() returns true, this method may be called to get
         * a widget displaying the options the user can interact with to modify
         * the behaviour of what happens when a given match is selected.
         *
         * @param widget the parent of the options widgets.
         */
        virtual void createRunOptions(QWidget *widget);

        /**
         * Called whenever an exact or possible match associated with this
         * runner is triggered.
         */
        virtual void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);

        /**
         * The nominal speed of the runner.
         * @see setSpeed
         */
        Speed speed() const;

        /**
         * The priority of the runner.
         * @see setPriority
         */
        Priority priority() const;

        /**
         * Returns the OR'ed value of all the Information types (as defined in RunnerContext::Type)
         * this runner is not interested in. 
         * @return OR'ed value of black listed types
         */
        RunnerContext::Types ignoredTypes() const;

        /**
         * Sets the types this runner will ignore 
         * @param types OR'ed listed of ignored types
         */
        void setIgnoredTypes(RunnerContext::Types types);

        /**
          * Returns the user visible engine name for the Runner
          */
        QString name() const;

        /**
          * Returns an id string for the Runner
          */
        QString id() const;

        /**
          * Returns the description of this Runner
          */
        QString description() const;

        /**
         * Accessor for the associated Package object if any.
         *
         * Note that the returned pointer is only valid for the lifetime of
         * the runner.
         *
         * @return the Package object, or 0 if none
         **/
        const Package* package() const;

        /**
         * Signal runner to reload its configuration.
         */
        virtual void reloadConfiguration();

    protected:
        friend class RunnerManager;
        friend class RunnerManagerPrivate;

        /**
         * Constructs a Runner object. Since AbstractRunner has pure virtuals,
         * this constructor can not be called directly. Rather a subclass must
         * be created
         */
        explicit AbstractRunner(QObject* parent = 0, const QString& serviceId = QString());
        AbstractRunner(QObject* parent, const QVariantList& args);

        /**
         * Provides access to the runner's configuration object.
         */
        KConfigGroup config() const;

        /**
         * Sets whether or not the the runner has options for matches
         */
        void setHasRunOptions(bool hasRunOptions);

        /**
         * Sets the nominal speed of the runner. Only slow runners need
         * to call this within their constructor because the default
         * speed is NormalSpeed. Runners that use DBUS should call
         * this within their constructors.
         */
        void setSpeed(Speed newSpeed);

        /**
         * Sets the priority of the runner. Lower priority runners are executed
         * only after higher priority runners.
         */
        void setPriority(Priority newPriority);

        /**
         * A blocking method to do queries of installed Services which can provide
         * a measure of safety for runners running their own threads. This should
         * be used instead of calling KServiceTypeTrader::query(..) directly.
         *
         * @arg serviceType a service type like "Plasma/Applet" or "KFilePlugin"
         * @arg constraint a constraint to limit the the choices returned.
         * @see KServiceTypeTrader::query(const QString&, const QString&)
         *
         * @return a list of services that satisfy the query.
         */
        KService::List serviceQuery(const QString &serviceType,
                                    const QString &constraint = QString()) const;

        QMutex* bigLock() const;

    protected Q_SLOTS:
        void init();

    private:
        AbstractRunnerPrivate* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_RUNNER( libname, classname )     \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_runner_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)


#define K_EXPORT_RUNNER_CONFIG( name, classname )     \
K_PLUGIN_FACTORY(ConfigFactory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(ConfigFactory("kcm_krunner_" #name)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif
