/* This file is part of the Nepomuk Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SEARCH_RUNNER_H_
#define _NEPOMUK_SEARCH_RUNNER_H_

#include <Plasma/AbstractRunner>

#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QList>

#include "result.h"


namespace Nepomuk {

    
    class SearchRunner : public Plasma::AbstractRunner
    {
        Q_OBJECT

    public:
        SearchRunner( QObject* parent, const QVariantList& args );
        SearchRunner( QObject* parent, const QString& serviceId = QString() );
        ~SearchRunner();

        void match( Plasma::RunnerContext& context );
        void run( const Plasma::RunnerContext& context, const Plasma::QueryMatch& action );

    private:
        void init();

        QMutex m_mutex;
        QWaitCondition m_waiter;

        int m_matchCnt;
    };
}

#endif
