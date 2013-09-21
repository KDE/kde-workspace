#ifndef ICONPLUGIN_H
#define ICONPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class IconPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

    public:
        virtual void registerTypes(const char *uri);
};

#endif //ICONPLUGIN_H