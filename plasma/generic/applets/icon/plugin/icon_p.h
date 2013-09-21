#ifndef ICONPRIVATE_H
#define ICONPRIVATE_H

#include <QObject>

class IconPrivate : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE QString getName(const QUrl& url) const;
    Q_INVOKABLE QString getIcon(const QUrl& url) const;

};

#endif