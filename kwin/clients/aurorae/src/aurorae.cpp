/********************************************************************
Copyright (C) 2009, 2010, 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "aurorae.h"
#include "auroraetheme.h"
#include "config-kwin.h"

#include <QApplication>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeItem>
#include <QGraphicsView>
#include <QPaintEngine>

#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <Plasma/FrameSvg>

namespace Aurorae
{

AuroraeFactory::AuroraeFactory()
        : QObject()
        , KDecorationFactoryUnstable()
        , m_theme(new AuroraeTheme(this))
        , m_engine(new QDeclarativeEngine(this))
        , m_component(new QDeclarativeComponent(m_engine, this))
        , m_engineType(AuroraeEngine)
{
    init();
}

void AuroraeFactory::init()
{
    qRegisterMetaType<uint>("Qt::MouseButtons");

    KConfig conf("auroraerc");
    KConfigGroup group(&conf, "Engine");
    if (!group.hasKey("EngineType") && !group.hasKey("ThemeName")) {
        // neither engine type and theme name are configured, use the only available theme
        initQML(group);
    } else if (group.hasKey("EngineType")) {
        const QString engineType = group.readEntry("EngineType", "aurorae").toLower();
        if (engineType == "qml") {
            initQML(group);
        } else {
            // fallback to classic Aurorae Themes
            initAurorae(conf, group);
        }
    } else {
        // fallback to classic Aurorae Themes
        initAurorae(conf, group);
    }
}

void AuroraeFactory::initAurorae(KConfig &conf, KConfigGroup &group)
{
    m_engineType = AuroraeEngine;
    const QString themeName = group.readEntry("ThemeName");
    if (themeName.isEmpty()) {
        // no theme configured, fall back to Plastik QML theme
        initQML(group);
        return;
    }
    KConfig config("aurorae/themes/" + themeName + '/' + themeName + "rc", KConfig::FullConfig, "data");
    KConfigGroup themeGroup(&conf, themeName);
    m_theme->loadTheme(themeName, config);
    m_theme->setBorderSize((KDecorationDefines::BorderSize)themeGroup.readEntry<int>("BorderSize", KDecorationDefines::BorderNormal));
    m_theme->setButtonSize((KDecorationDefines::BorderSize)themeGroup.readEntry<int>("ButtonSize", KDecorationDefines::BorderNormal));
    m_theme->setTabDragMimeType(tabDragMimeType());
    // setup the QML engine
    /* use logic from KDeclarative::setupBindings():
    "addImportPath adds the path at the beginning, so to honour user's
     paths we need to traverse the list in reverse order" */
    QStringListIterator paths(KGlobal::dirs()->findDirs("module", "imports"));
    paths.toBack();
    while (paths.hasPrevious()) {
        m_engine->addImportPath(paths.previous());
    }
    m_component->loadUrl(QUrl(KStandardDirs::locate("data", "kwin/aurorae/aurorae.qml")));
    m_engine->rootContext()->setContextProperty("auroraeTheme", m_theme);
    m_themeName = themeName;
}

void AuroraeFactory::initQML(const KConfigGroup &group)
{
    // try finding the QML package
    const QString themeName = group.readEntry("ThemeName", "kwin4_decoration_qml_plastik");
    kDebug(1212) << "Trying to load QML Decoration " << themeName;
    const QString internalname = themeName.toLower();

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(internalname);
    KService::List offers = KServiceTypeTrader::self()->query("KWin/Decoration", constraint);
    if (offers.isEmpty()) {
        kError(1212) << "Couldn't find QML Decoration " << themeName << endl;
        // TODO: what to do in error case?
        return;
    }
    KService::Ptr service = offers.first();
    KPluginInfo plugininfo(service);
    const QString pluginName = service->property("X-KDE-PluginInfo-Name").toString();
    const QString scriptName = service->property("X-Plasma-MainScript").toString();
    const QString file = KStandardDirs::locate("data", QLatin1String(KWIN_NAME) + "/decorations/" + pluginName + "/contents/" + scriptName);
    if (file.isNull()) {
        kDebug(1212) << "Could not find script file for " << pluginName;
        // TODO: what to do in error case?
        return;
    }
    m_engineType = QMLEngine;
    // setup the QML engine
    /* use logic from KDeclarative::setupBindings():
    "addImportPath adds the path at the beginning, so to honour user's
     paths we need to traverse the list in reverse order" */
    QStringListIterator paths(KGlobal::dirs()->findDirs("module", "imports"));
    paths.toBack();
    while (paths.hasPrevious()) {
        m_engine->addImportPath(paths.previous());
    }
    m_component->loadUrl(QUrl::fromLocalFile(file));
    m_themeName = themeName;
}

AuroraeFactory::~AuroraeFactory()
{
    s_instance = NULL;
}

AuroraeFactory *AuroraeFactory::instance()
{
    if (!s_instance) {
        s_instance = new AuroraeFactory;
    }

    return s_instance;
}

bool AuroraeFactory::reset(unsigned long changed)
{
    if (changed & SettingButtons) {
        emit buttonsChanged();
    }
    if (changed & SettingFont){
        emit titleFontChanged();
    }
    if (changed == SettingCompositing) {
        return false;
    }
    const KConfig conf("auroraerc");
    const KConfigGroup group(&conf, "Engine");
    const QString themeName = group.readEntry("ThemeName", "example-deco");
    const KConfig config("aurorae/themes/" + themeName + '/' + themeName + "rc", KConfig::FullConfig, "data");
    const KConfigGroup themeGroup(&conf, themeName);
    if (themeName != m_themeName) {
        m_engine->clearComponentCache();
        init();
        // recreate all decorations
        return true;
    }
    if (m_engineType == AuroraeEngine) {
        m_theme->setBorderSize((KDecorationDefines::BorderSize)themeGroup.readEntry<int>("BorderSize", KDecorationDefines::BorderNormal));
        m_theme->setButtonSize((KDecorationDefines::BorderSize)themeGroup.readEntry<int>("ButtonSize", KDecorationDefines::BorderNormal));
    }
    emit configChanged();
    return changed & (SettingDecoration | SettingButtons | SettingBorder); // need hard reset
}

bool AuroraeFactory::supports(Ability ability) const
{
    switch (ability) {
    case AbilityAnnounceButtons:
    case AbilityUsesAlphaChannel:
    case AbilityAnnounceAlphaChannel:
    case AbilityButtonMenu:
    case AbilityButtonSpacer:
    case AbilityExtendIntoClientArea:
    case AbilityButtonMinimize:
    case AbilityButtonMaximize:
    case AbilityButtonClose:
    case AbilityButtonAboveOthers:
    case AbilityButtonBelowOthers:
    case AbilityButtonShade:
    case AbilityButtonOnAllDesktops:
    case AbilityButtonHelp:
    case AbilityButtonApplicationMenu:
    case AbilityProvidesShadow:
        return true; // TODO: correct value from theme
    case AbilityTabbing:
        return false;
    case AbilityUsesBlurBehind:
        return true;
    default:
        return false;
    }
}

KDecoration *AuroraeFactory::createDecoration(KDecorationBridge *bridge)
{
    AuroraeClient *client = new AuroraeClient(bridge, this);
    return client;
}

QList< KDecorationDefines::BorderSize > AuroraeFactory::borderSizes() const
{
    return QList< BorderSize >() << BorderTiny << BorderNormal <<
        BorderLarge << BorderVeryLarge <<  BorderHuge <<
        BorderVeryHuge << BorderOversized;
}

QDeclarativeItem *AuroraeFactory::createQmlDecoration(Aurorae::AuroraeClient *client)
{
    QDeclarativeContext *context = new QDeclarativeContext(m_engine->rootContext(), this);
    context->setContextProperty("decoration", client);
    return qobject_cast< QDeclarativeItem* >(m_component->create(context));
}

AuroraeFactory *AuroraeFactory::s_instance = NULL;

/*******************************************************
* Client
*******************************************************/
AuroraeClient::AuroraeClient(KDecorationBridge *bridge, KDecorationFactory *factory)
    : KDecorationUnstable(bridge, factory)
    , m_view(NULL)
    , m_scene(new QGraphicsScene(this))
    , m_item(AuroraeFactory::instance()->createQmlDecoration(this))
{
    connect(this, SIGNAL(keepAboveChanged(bool)), SIGNAL(keepAboveChangedWrapper()));
    connect(this, SIGNAL(keepBelowChanged(bool)), SIGNAL(keepBelowChangedWrapper()));
    connect(AuroraeFactory::instance(), SIGNAL(buttonsChanged()), SIGNAL(buttonsChanged()));
    connect(AuroraeFactory::instance(), SIGNAL(configChanged()), SIGNAL(configChanged()));
    connect(AuroraeFactory::instance(), SIGNAL(titleFontChanged()), SIGNAL(fontChanged()));
    connect(m_item, SIGNAL(alphaChanged()), SLOT(slotAlphaChanged()));
    connect(this, SIGNAL(appMenuAvailable()), SIGNAL(appMenuAvailableChanged()));
    connect(this, SIGNAL(appMenuUnavailable()), SIGNAL(appMenuAvailableChanged()));
}

AuroraeClient::~AuroraeClient()
{
    if (m_item) {
        m_item->setParent(NULL);
        m_item->deleteLater();
    }
    m_scene->setParent(NULL);
    m_scene->deleteLater();
    m_view->setParent(NULL);
    m_view->deleteLater();
}

void AuroraeClient::init()
{
    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    // HACK: we need to add the GraphicsView as a child widget to a normal widget
    // the GraphicsView eats the mouse release event and by that kwin core starts to move
    // the decoration each time the decoration is clicked
    // therefore we use two widgets and inject an own mouse release event to the parent widget
    // when the graphics view eats a mouse event
    createMainWidget();
    widget()->setAttribute(Qt::WA_TranslucentBackground);
    widget()->setAttribute(Qt::WA_NoSystemBackground);
    widget()->installEventFilter(this);
    m_view = new QGraphicsView(m_scene, widget());
    m_view->setAttribute(Qt::WA_TranslucentBackground);
    m_view->setWindowFlags(Qt::X11BypassWindowManagerHint);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    QPalette pal = m_view->palette();
    pal.setColor(m_view->backgroundRole(), Qt::transparent);
    m_view->setPalette(pal);
    QPalette pal2 = widget()->palette();
    pal2.setColor(widget()->backgroundRole(), Qt::transparent);
    widget()->setPalette(pal2);
    if (m_item)
        m_scene->addItem(m_item);
    slotAlphaChanged();

    AuroraeFactory::instance()->theme()->setCompositingActive(compositingActive());
}

bool AuroraeClient::eventFilter(QObject *object, QEvent *event)
{
    // we need to filter the wheel events on the decoration
    // QML does not yet provide a way to accept wheel events, this will change with Qt 5
    // TODO: remove in KDE5
    // see BUG: 304248
    if (object != widget() || event->type() != QEvent::Wheel) {
        return KDecorationUnstable::eventFilter(object, event);
    }
    QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
    if (mousePosition(wheel->pos()) == PositionCenter) {
        titlebarMouseWheelOperation(wheel->delta());
        return true;
    }
    return false;
}

void AuroraeClient::activeChange()
{
    emit activeChanged();
}

void AuroraeClient::captionChange()
{
    emit captionChanged();
}

void AuroraeClient::iconChange()
{
    emit iconChanged();
}

void AuroraeClient::desktopChange()
{
    emit desktopChanged();
}

void AuroraeClient::maximizeChange()
{
    if (!options()->moveResizeMaximizedWindows()) {
        emit maximizeChanged();
    }
}

void AuroraeClient::resize(const QSize &s)
{
    if (m_item) {
        m_item->setWidth(s.width());
        m_item->setHeight(s.height());
    }
    m_scene->setSceneRect(QRectF(QPoint(0, 0), s));
    m_view->resize(s);
    widget()->resize(s);
}

void AuroraeClient::shadeChange()
{
    emit shadeChanged();
}

void AuroraeClient::borders(int &left, int &right, int &top, int &bottom) const
{
    if (!m_item) {
        left = right = top = bottom = 0;
        return;
    }
    const bool maximized = maximizeMode() == MaximizeFull && !options()->moveResizeMaximizedWindows();
    QObject *borders = NULL;
    if (maximized) {
        borders = m_item->findChild<QObject*>("maximizedBorders");
    } else {
        borders = m_item->findChild<QObject*>("borders");
    }
    sizesFromBorders(borders, left, right, top, bottom);
}

void AuroraeClient::padding(int &left, int &right, int &top, int &bottom) const
{
    if (!m_item) {
        left = right = top = bottom = 0;
        return;
    }
    if (maximizeMode() == MaximizeFull && !options()->moveResizeMaximizedWindows()) {
        left = right = top = bottom = 0;
        return;
    }
    sizesFromBorders(m_item->findChild<QObject*>("padding"), left, right, top, bottom);
}

void AuroraeClient::sizesFromBorders(const QObject *borders, int &left, int &right, int &top, int &bottom) const
{
    if (!borders) {
        return;
    }
    left = borders->property("left").toInt();
    right = borders->property("right").toInt();
    top = borders->property("top").toInt();
    bottom = borders->property("bottom").toInt();
}

QSize AuroraeClient::minimumSize() const
{
    return widget()->minimumSize();
}

KDecorationDefines::Position AuroraeClient::mousePosition(const QPoint &point) const
{
    // based on the code from deKorator
    int pos = PositionCenter;
    if (isShade() || isMaximized()) {
        return Position(pos);
    }

    int borderLeft, borderTop, borderRight, borderBottom;
    borders(borderLeft, borderRight, borderTop, borderBottom);
    int paddingLeft, paddingTop, paddingRight, paddingBottom;
    padding(paddingLeft, paddingRight, paddingTop, paddingBottom);
    int titleEdgeLeft, titleEdgeRight, titleEdgeTop, titleEdgeBottom;
    AuroraeFactory::instance()->theme()->titleEdges(titleEdgeLeft, titleEdgeTop, titleEdgeRight, titleEdgeBottom, false);
    switch (AuroraeFactory::instance()->theme()->decorationPosition()) {
    case DecorationTop:
        borderTop = titleEdgeTop;
        break;
    case DecorationLeft:
        borderLeft = titleEdgeLeft;
        break;
    case DecorationRight:
        borderRight = titleEdgeRight;
        break;
    case DecorationBottom:
        borderBottom = titleEdgeBottom;
        break;
    default:
        break; // nothing
    }
    if (point.x() >= (m_view->width() -  borderRight - paddingRight)) {
        pos |= PositionRight;
    } else if (point.x() <= borderLeft + paddingLeft) {
        pos |= PositionLeft;
    }

    if (point.y() >= m_view->height() - borderBottom - paddingBottom) {
        pos |= PositionBottom;
    } else if (point.y() <= borderTop + paddingTop ) {
        pos |= PositionTop;
    }

    return Position(pos);
}

void AuroraeClient::reset(long unsigned int changed)
{
    KDecoration::reset(changed);
}

void AuroraeClient::menuClicked()
{
    showWindowMenu(QCursor::pos());
}

void AuroraeClient::appMenuClicked()
{
    showApplicationMenu(QCursor::pos());
}

void AuroraeClient::toggleShade()
{
    setShade(!isShade());
}

void AuroraeClient::toggleKeepAbove()
{
    setKeepAbove(!keepAbove());
}

void AuroraeClient::toggleKeepBelow()
{
    setKeepBelow(!keepBelow());
}

bool AuroraeClient::isMaximized() const
{
    return maximizeMode()==KDecorationDefines::MaximizeFull && !options()->moveResizeMaximizedWindows();
}

void AuroraeClient::titlePressed(int button, int buttons)
{
    titlePressed(static_cast<Qt::MouseButton>(button), static_cast<Qt::MouseButtons>(buttons));
}

void AuroraeClient::titleReleased(int button, int buttons)
{
    titleReleased(static_cast<Qt::MouseButton>(button), static_cast<Qt::MouseButtons>(buttons));
}

void AuroraeClient::titleMouseMoved(int button, int buttons)
{
    titleMouseMoved(static_cast<Qt::MouseButton>(button), static_cast<Qt::MouseButtons>(buttons));
}

void AuroraeClient::titlePressed(Qt::MouseButton button, Qt::MouseButtons buttons)
{
    const QPoint cursor = QCursor::pos();
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress, widget()->mapFromGlobal(cursor),
                                         cursor, button, buttons, Qt::NoModifier);
    processMousePressEvent(event);
    delete event;
    event = 0;
}

void AuroraeClient::titleReleased(Qt::MouseButton button, Qt::MouseButtons buttons)
{
    const QPoint cursor = QCursor::pos();
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, widget()->mapFromGlobal(cursor),
                                         cursor, button, buttons, Qt::NoModifier);
    QApplication::sendEvent(widget(), event);
    delete event;
    event = 0;
}

void AuroraeClient::titleMouseMoved(Qt::MouseButton button, Qt::MouseButtons buttons)
{
    const QPoint cursor = QCursor::pos();
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, widget()->mapFromGlobal(cursor),
                                         cursor, button, buttons, Qt::NoModifier);
    QApplication::sendEvent(widget(), event);
    delete event;
    event = 0;
}

void AuroraeClient::themeChanged()
{
    m_scene->clear();
    m_item = AuroraeFactory::instance()->createQmlDecoration(this);
    if (!m_item) {
        return;
    }

    m_item->setWidth(m_scene->sceneRect().width());
    m_item->setHeight(m_scene->sceneRect().height());
    m_scene->addItem(m_item);
    connect(m_item, SIGNAL(alphaChanged()), SLOT(slotAlphaChanged()));
    slotAlphaChanged();
}

int AuroraeClient::doubleClickInterval() const
{
    return QApplication::doubleClickInterval();
}

void AuroraeClient::closeWindow()
{
    QMetaObject::invokeMethod(qobject_cast< KDecorationUnstable* >(this), "doCloseWindow", Qt::QueuedConnection);
}

void AuroraeClient::doCloseWindow()
{
    KDecorationUnstable::closeWindow();
}

void AuroraeClient::maximize(int button)
{
    // a maximized window does not need to have a window decoration
    // in that case we need to delay handling by one cycle
    // BUG: 304870
    QMetaObject::invokeMethod(qobject_cast< KDecorationUnstable* >(this),
                              "doMaximzie",
                              Qt::QueuedConnection,
                              Q_ARG(int, button));
}

void AuroraeClient::doMaximzie(int button)
{
    KDecorationUnstable::maximize(static_cast<Qt::MouseButton>(button));
}

void AuroraeClient::titlebarDblClickOperation()
{
    // the double click operation can result in a window being maximized
    // see maximize
    QMetaObject::invokeMethod(qobject_cast< KDecorationUnstable* >(this), "doTitlebarDblClickOperation", Qt::QueuedConnection);
}

void AuroraeClient::doTitlebarDblClickOperation()
{
    KDecorationUnstable::titlebarDblClickOperation();
}

QVariant AuroraeClient::readConfig(const QString &key, const QVariant &defaultValue)
{
    KSharedConfigPtr config = KSharedConfig::openConfig("auroraerc");
    return config->group(AuroraeFactory::instance()->currentThemeName()).readEntry(key, defaultValue);
}

void AuroraeClient::slotAlphaChanged()
{
    if (!m_item) {
        setAlphaEnabled(false);
        return;
    }
    QVariant alphaProperty = m_item->property("alpha");
    if (alphaProperty.isValid() && alphaProperty.canConvert<bool>()) {
        setAlphaEnabled(alphaProperty.toBool());
    } else {
        // by default all Aurorae themes use the alpha channel
        setAlphaEnabled(true);
    }
}

QRegion AuroraeClient::region(KDecorationDefines::Region r)
{
    if (r != ExtendedBorderRegion) {
        return QRegion();
    }
    if (!m_item) {
        return QRegion();
    }
    if (isMaximized()) {
        // empty region for maximized windows
        return QRegion();
    }
    int left, right, top, bottom;
    left = right = top = bottom = 0;
    sizesFromBorders(m_item->findChild<QObject*>("extendedBorders"), left, right, top, bottom);
    if (top == 0 && right == 0 && bottom == 0 && left == 0) {
        // no extended borders
        return QRegion();
    }

    int paddingLeft, paddingRight, paddingTop, paddingBottom;
    paddingLeft = paddingRight = paddingTop = paddingBottom = 0;
    padding(paddingLeft, paddingRight, paddingTop, paddingBottom);
    QRect rect = widget()->rect().adjusted(paddingLeft, paddingTop, -paddingRight, -paddingBottom);
    rect.translate(-paddingLeft, -paddingTop);

    return QRegion(rect.adjusted(-left, -top, right, bottom)).subtract(rect);
}

bool AuroraeClient::animationsSupported() const
{
    if (!compositingActive()) {
        return false;
    }
    QPixmap pix(1,1);
    QPainter p(&pix);
    const bool raster = p.paintEngine()->type() == QPaintEngine::Raster;
    p.end();
    return raster;
}

} // namespace Aurorae

extern "C"
{
    KDE_EXPORT KDecorationFactory *create_factory() {
        return Aurorae::AuroraeFactory::instance();
    }
    KWIN_EXPORT int decoration_version() {
        return KWIN_DECORATION_API_VERSION;
    }
}


#include "aurorae.moc"
