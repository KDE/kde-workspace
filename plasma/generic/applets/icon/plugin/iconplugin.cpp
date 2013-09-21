#include "iconplugin.h"
#include "icon_p.h"
#include <QtQml>

void IconPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.icon"));
    qmlRegisterType<IconPrivate>(uri, 1, 0,"Logic");
}

#include "iconplugin.moc"