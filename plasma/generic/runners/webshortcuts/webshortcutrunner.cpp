/*
 *   Copyright (C) 2007 Teemu Rytilahti <tpr@iki.fi>
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

#include "webshortcutrunner.h"

#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KSycoca>
#include <KToolInvocation>
#include <KUrl>
#include <KUriFilter>

#include <QtDBus/QtDBus>

WebshortcutRunner::WebshortcutRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args),
      m_match(this), m_filterBeforeRun(false)
{
    Q_UNUSED(args);
    setObjectName( QLatin1String("Web Shortcut" ));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File | Plasma::RunnerContext::Executable);

    m_icon = KIcon("internet-web-browser");

    m_match.setType(Plasma::QueryMatch::ExactMatch);
    m_match.setRelevance(0.9);

    // Listen for KUriFilter plugin config changes and update state...
    QDBusConnection sessionDbus = QDBusConnection::sessionBus();
    sessionDbus.connect(QString(), "/", "org.kde.KUriFilterPlugin",
                        "configure", this, SLOT(readFiltersConfig()));

    connect(this, SIGNAL(teardown()), this, SLOT(resetState()));
    readFiltersConfig();
}

WebshortcutRunner::~WebshortcutRunner()
{
}

void WebshortcutRunner::readFiltersConfig()
{
    // Make sure that the searchEngines cache, etc. is refreshed when the config file is changed.
    loadSyntaxes();
}

void WebshortcutRunner::loadSyntaxes()
{
    KUriFilterData filterData (QLatin1String(":q"));
    filterData.setSearchFilteringOptions(KUriFilterData::RetrieveAvailableSearchProvidersOnly);
    if (KUriFilter::self()->filterSearchUri(filterData, KUriFilter::NormalTextFilter)) {
        m_delimiter = filterData.searchTermSeparator();
    }

    //kDebug() << "keyword delimiter:" << m_delimiter;
    //kDebug() << "search providers:" << filterData.preferredSearchProviders();

    QList<Plasma::RunnerSyntax> syns;
    Q_FOREACH (const QString &provider, filterData.preferredSearchProviders()) {
        //kDebug() << "checking out" << provider;
        Plasma::RunnerSyntax s(filterData.queryForPreferredSearchProvider(provider), /*":q:",*/
                              i18n("Opens \"%1\" in a web browser with the query :q:.", provider));
        syns << s;
    }

    setSyntaxes(syns);
}

void WebshortcutRunner::resetState()
{
    kDebug();
    m_lastFailedKey.clear();
    m_lastProvider.clear();
    m_lastKey.clear();
}

void WebshortcutRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();

    if (term.length() < 3 || !term.contains(m_delimiter))
        return;

    // kDebug() << "checking term" << term;

    const int delimIndex = term.indexOf(m_delimiter);
    if (delimIndex == term.length() - 1)
        return;

    const QString key = term.left(delimIndex);

    if (key == m_lastFailedKey) {
        return;    // we already know it's going to suck ;)
    }

    if (!context.isValid()) {
        kDebug() << "invalid context";
        return;
    }

    // Do a fake user feedback text update if the keyword has not changed.
    // There is no point filtering the request on every key stroke.
    // filtering
    if (m_lastKey == key) {
        m_filterBeforeRun = true;
        m_match.setText(i18n("Search %1 for %2", m_lastProvider, term.mid(delimIndex + 1)));
        context.addMatch(term, m_match);
        return;
    }

    KUriFilterData filterData(term);
    if (!KUriFilter::self()->filterSearchUri(filterData, KUriFilter::WebShortcutFilter)) {
        m_lastFailedKey = key;
        return;
    }

    m_lastFailedKey.clear();
    m_lastKey = key;
    m_lastProvider = filterData.searchProvider();

    m_match.setData(filterData.uri().url());
    m_match.setId("WebShortcut:" + key);

    m_match.setIcon(KIcon(filterData.iconName()));
    m_match.setText(i18n("Search %1 for %2", m_lastProvider, filterData.searchTerm()));
    context.addMatch(term, m_match);
}

void WebshortcutRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    QString location;

    //kDebug() << "filter before run?" << m_filterBeforeRun;
    if (m_filterBeforeRun) {
        m_filterBeforeRun = false;
        //kDebug() << "look up webshortcut:" << context.query();
        KUriFilterData filterData (context.query());
        if (KUriFilter::self()->filterSearchUri(filterData, KUriFilter::WebShortcutFilter))
            location = filterData.uri().url();
    } else {
        location = match.data().toString();
    }

    //kDebug() << location;
    if (!location.isEmpty()) {
        KToolInvocation::invokeBrowser(location);
    }
}

#include "webshortcutrunner.moc"
