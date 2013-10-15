/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef WINDOWSELECTOR_H
#define WINDOWSELECTOR_H

#include <QWidget>
#include <QAbstractNativeEventFilter>

namespace KHotKeys
{

class WindowSelector : public QWidget, public QAbstractNativeEventFilter
    {
    Q_OBJECT

    public:
        WindowSelector( QObject* receiver, const char* slot );
        virtual ~WindowSelector();
        void select();

        virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    Q_SIGNALS:
        void selected_signal( WId w );
    private:
        WId findRealWindow( WId w, int depth = 0 );
    };

} // namespace KHotKeys

#endif
