#include "accessiblefactory.h"

#include <plasma/applet.h>

#include "panelview.h"
#include "desktopview.h"
#include "plasmaapp.h"


QAccessibleInterface* Plasma::accessibleInterfaceFactory(const QString &key, QObject *object)
{
    Q_UNUSED(key)

    if (Plasma::Applet *applet = qobject_cast<Plasma::Applet*>(object))
        return new AccessiblePlasmaApplet(applet);
    if (PanelView *view = qobject_cast<PanelView*>(object))
        return new AccessiblePlasmaPanelView(view);
    if (Plasma::View *view = qobject_cast<Plasma::View*>(object))
        return new AccessiblePlasmaView(view);
    if (PlasmaApp *app = qobject_cast<PlasmaApp*>(object))
        return new AccessiblePlasmaApp(app);
    return 0;
}
