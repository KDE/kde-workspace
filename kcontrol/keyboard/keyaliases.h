#ifndef ALIASES_H
#define ALIASES_H
#include<QMap>

class Aliases
{
public:
    Aliases();
    QMap<QString,QString>qwerty;
    QMap<QString,QString>azerty;
    QMap<QString,QString>qwertz;
	QString findaliasdir();
    QString getAlias(QString type,QString name);
};

#endif // ALIASES_H
