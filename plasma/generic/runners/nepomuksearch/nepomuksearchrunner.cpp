/* This file is part of the Nepomuk Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-13 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "nepomuksearchrunner.h"

#include <QMenu>
#include <QMimeData>
#include <QFile>

#include <KIcon>
#include <KRun>
#include <KDebug>
#include <KUrl>

#include <Nepomuk2/File>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Variant>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Query/ResultIterator>
#include <Nepomuk2/Query/QueryParser>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/ComparisonTerm>

#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Vocabulary/NFO>
#include <Soprano/Vocabulary/NAO>

#include <KFileItemActions>
#include <KFileItemList>
#include <KFileItemListProperties>
#include <KIO/NetAccess>

#include <KMimeType>
#include <KMimeTypeTrader>
#include <KService>

using namespace Nepomuk2::Vocabulary;
using namespace Soprano::Vocabulary;

namespace {
    /**
     * Milliseconds to wait before issuing the next query.
     * This timeout is intended to prevent us from starting
     * a query after each key press by the user. So it should
     * roughly equal the time between key presses of an average user.
     */
    const int s_userActionTimeout = 400;
    const int s_maxResults = 10;
}


Nepomuk2::SearchRunner::SearchRunner( QObject* parent, const QVariantList& args )
    : Plasma::AbstractRunner( parent, args )
{
}


Nepomuk2::SearchRunner::SearchRunner( QObject* parent, const QString& serviceId )
    : Plasma::AbstractRunner( parent, serviceId )
{
}


void Nepomuk2::SearchRunner::init()
{
    // Just constructing the instance initializes the ResourceManager
    Nepomuk2::ResourceManager::instance();

    // we are pretty slow at times and use DBus calls
    setSpeed(SlowSpeed);

    // we are way less important than others, mostly because we are slow
    setPriority(LowPriority);

    m_actions = new KFileItemActions(this);
    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds files, documents and other content that matches :q: using the desktop search system.")));
}


Nepomuk2::SearchRunner::~SearchRunner()
{
    qDeleteAll(m_konqActions);
}


void Nepomuk2::SearchRunner::match( Plasma::RunnerContext& context )
{
    kDebug() << &context << context.query();

    if( ResourceManager::instance()->initialized() ) {
        // This method needs to be thread-safe since KRunner does simply start new threads whenever
        // the query term changes.
        m_mutex.lock();

        // we do not want to restart a query on each key-press. That would result
        // in way too many queries for the rather sluggy Nepomuk query service
        // Thus, we use a little timeout to make sure we do not query too often

        m_waiter.wait(&m_mutex, s_userActionTimeout);
        m_mutex.unlock();

        if (!context.isValid()) {
            kDebug() << "deprecated search:" << context.query();
            // we are no longer the latest call
            return;
        }

        // no queries on very short strings
        // We use a default length of 4 characters cause virtuoso has a min length requirement of 4
        // leading characters. This way we avoid using the expensive regex search
        if( context.query().length() >= 4 ) {
            Query::Query query = Query::QueryParser::parseQuery(context.query());
            query.setLimit(s_maxResults);

            Query::ResultIterator it( query );
            while( context.isValid() && it.next() ) {
                Plasma::QueryMatch match = convertToQueryMatch( it.result() );
                if( match.isValid() )
                    context.addMatch(context.query(), match);
            }
        }
    }
}

namespace {
    // Copied from kde-runtime/nepomuk/kioslaves/kio_nepomuk.cpp
    /**
     * This function constructs the url the nepomuk kioslave would redirect to.
     * We use this instead of just supplying the nepomuk kioslave with the url because
     * applications like Dolphin display the url on the title bar even though the
     * 'display name' would be set cause that would involve stating the file.
     *
     * Therefore, in order to avoid showing an ugly 'nepomuk:/res/uuid' to the user we
     * redirect the url ourselves so we have a pretty representation.
     */
    KUrl redirectionUrl(const Nepomuk2::Resource& res) {
        using namespace Nepomuk2;

        // list tags by listing everything tagged with that tag
        if (res.hasType(NAO::Tag())) {
            Query::ComparisonTerm term(NAO::hasTag(), Query::ResourceTerm( res ), Query::ComparisonTerm::Equal);
            KUrl url = Query::Query(term).toSearchUrl(i18n( "Things tagged '%1'", res.genericLabel()));
            url.addQueryItem(QLatin1String("resource"), KUrl(res.uri()).url());
            return url;
        }

        // list everything else besides files by querying things related to the resource in some way
        // this works for music albums or artists but it would also work for tags
        else if (!res.hasType(NFO::FileDataObject())) {
            Query::ComparisonTerm term(QUrl(), Query::ResourceTerm(res), Query::ComparisonTerm::Equal);
            KUrl url = Query::Query(term).toSearchUrl(res.genericLabel());
            url.addQueryItem(QLatin1String("resource"), KUrl(res.uri()).url());
            return url;
        }

        // no forwarding done
        return KUrl();
    }
}

void Nepomuk2::SearchRunner::run( const Plasma::RunnerContext&, const Plasma::QueryMatch& match )
{
    // If no action was selected, the interface doesn't support multiple
    // actions so we simply open the file
    if (QAction *a = match.selectedAction()) {
        if (a != action("open")) {
            match.selectedAction()->trigger();
            return;
        }
    }

    Nepomuk2::Resource res = match.data().value<Nepomuk2::Resource>();
    KUrl url = res.uri();
    KUrl nieUrl = res.property( NIE::url() ).toUrl();
    if( !nieUrl.isEmpty() )
        url = nieUrl;

    // Redirect it in order to avoid showing the user an ugly URL
    if(url.scheme() == QLatin1String("nepomuk")) {
        KUrl newUrl = redirectionUrl(res);
        if(newUrl.isValid())
            url = newUrl;
    }

    KService::Ptr preferredServicePtr;
    if (res.hasProperty(Nepomuk2::Vocabulary::NIE::mimeType()) &&
         KUrl(res.property(Nepomuk2::Vocabulary::NIE::url()).toUrl()).isLocalFile()) {
        preferredServicePtr = KMimeTypeTrader::self()->preferredService(res.property(Nepomuk2::Vocabulary::NIE::mimeType()).toString());
    }

    if (preferredServicePtr.isNull() || !KRun::run(*preferredServicePtr.constData(), KUrl::List(url), 0)) {
        (void)new KRun(url, 0);
    }
}

QList<QAction*> Nepomuk2::SearchRunner::actionsFromMenu(QMenu *menu, const QString &prefix, QObject *parent)
{
    Q_ASSERT(menu);

    QList<QAction*> ret;
    foreach (QAction *action, menu->actions()) {
        if (QMenu *submenu = action->menu()) {
            //Flatten hierarchy and prefix submenu text to all actions in submenu
            ret << actionsFromMenu(submenu, action->text(), parent);
        } else if (!action->isSeparator() && action->isEnabled()) {
            QString text = action->text();
            if (action->isCheckable()) {
                if (action->isChecked()) {
                    text = QString("(%1) %2").arg(QChar(0x2613)).arg(text);
                } else {
                    text = QString("( ) %1").arg(text);
                }
            }

            if (!prefix.isEmpty()) {
                text = QString("%1: %2").arg(prefix).arg(text);
            }
            text = text.replace(QRegExp("&([\\S])"), "\\1");

            QAction *a = new QAction(action->icon(), text, parent);

            QObject::connect(a, SIGNAL(triggered(bool)), action, SIGNAL(triggered(bool)));
            ret << a;
        }
    }
    return ret;
}


QList<QAction*> Nepomuk2::SearchRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    //Unlike other runners, the actions generated here are likely to see
    //little reuse. Hence, we will clear the actions then generate new
    //ones per iteration to avoid excessive memory consumption.
    qDeleteAll(m_konqActions);
    m_konqActions.clear();

    QList<QAction*> ret;
    if (!action("open")) {
         addAction("open", KIcon("document-open"), i18n("Open"));
    }
    ret << action("open");

    Nepomuk2::Resource res = match.data().value<Nepomuk2::Resource>();

    KUrl url(res.uri());
    KIO::UDSEntry entry;
    if (!KIO::NetAccess::stat(url.path(), entry, 0)) {
        return QList<QAction*>();
    }

    KFileItemList list;
    list << KFileItem(entry, url);

    KFileItemListProperties prop;
    prop.setItems(list);

    QMenu dummy;
    m_actions->setItemListProperties(prop);
    m_actions->addOpenWithActionsTo(&dummy, QString());
    //Add user defined actions
    m_actions->addServiceActionsTo(&dummy);

    m_konqActions = actionsFromMenu(&dummy);

    ret << m_konqActions;

    return ret;
}

QMimeData * Nepomuk2::SearchRunner::mimeDataForMatch(const Plasma::QueryMatch *match)
{
    Nepomuk2::Resource res = match->data().value<Nepomuk2::Resource>();

    QUrl url = res.property(NIE::url()).toUrl();

    if (!url.isValid()) {
        return 0;
    }

    QMimeData *result = new QMimeData();
    QList<QUrl> urls;
    urls << url;
    kDebug() << urls;
    result->setUrls(urls);
    return result;
}

namespace {
    qreal normalizeScore(double score) {
        // no search result is ever a perfect match, NEVER. And mostly, when typing a URL
        // the users wants to open that url instead of using the search result. Thus, all
        // search results need to have a lower score than URLs which can drop to 0.5
        // And in the end, for 10 results, the score is not that important at the moment.
        // This can be improved in the future.
        // We go the easy way here and simply cut the score at 0.4
        return qMin(0.4, score);
    }
}

// FIXME: Avoid using the Resource class. Request the extra properties from the query
Plasma::QueryMatch Nepomuk2::SearchRunner::convertToQueryMatch(const Nepomuk2::Query::Result& result)
{
    Plasma::QueryMatch match( this );
    match.setType(Plasma::QueryMatch::PossibleMatch);
    match.setRelevance(normalizeScore(result.score()));

    Nepomuk2::Resource res = result.resource();

    QString type;
    QString iconName;

    KMimeType::Ptr mimetype;
    if (res.hasProperty(NIE::mimeType())) {
        mimetype = KMimeType::mimeType(res.property(NIE::mimeType()).toString());
    }

    const QUrl fileUrl = res.toFile().url();
    if (!mimetype && res.isFile() && fileUrl.isLocalFile() ) {
        mimetype = KMimeType::findByUrl(fileUrl);
    }

    if (mimetype) {
        type = mimetype->comment();
        iconName = mimetype->iconName();
    }

    if(type.isEmpty()) {
        type = Nepomuk2::Types::Class(res.type()).label();

        // The Query engine should typically never return properties, classes or graphs.
        // But this doesn't seem to be the case cause of certain bugs in virtuoso
        // Earlier versions of Nepomuk avoided this by complex queries which resulted in many
        // "Virtuoso is crazy" reports.
        // For 4.10, we just try to cover it up and not show the results.
        // See nepomuk-core/libnepomukcore/query/query.cpp for more details
        //
        if(type.contains(QLatin1String("property"), Qt::CaseInsensitive) ||
           type.contains(QLatin1String("class"), Qt::CaseInsensitive) ||
           type.contains(QLatin1String("graph"), Qt::CaseInsensitive)) {
                return Plasma::QueryMatch( 0 );
        }
        iconName = res.genericIcon();
    }

    // HACK: Do not show non-existing files
    if( fileUrl.isLocalFile() && !QFile::exists( fileUrl.toLocalFile() ) )
        return Plasma::QueryMatch( 0 );

    // HACK: Do not show resources which do not have a label
    QString label = res.genericLabel();
    if( label.startsWith(QLatin1String("nepomuk:/res")) )
        return Plasma::QueryMatch( 0 );

    match.setText(res.genericLabel());
    match.setSubtext(type);
    match.setIcon(KIcon(iconName.isEmpty() ? QString::fromLatin1("nepomuk") : iconName));

    match.setData(qVariantFromValue(res));
    match.setId(KUrl(res.uri()).url());

    return match;
}


K_EXPORT_PLASMA_RUNNER(nepomuksearchrunner, Nepomuk2::SearchRunner)

#include "nepomuksearchrunner.moc"
