/****************************************************************************
**
**
** KRDB - puts current KDE color scheme into preprocessor statements
** cats specially written application default files and uses xrdb -merge to
** write to RESOURCE_MANAGER. Thus it gives a  simple way to make non-KDE
** applications fit in with the desktop
**
** Copyright (C) 1998 by Mark Donohoe
** Copyright (C) 1999 by Dirk A. Mueller (reworked for KDE 2.0)
** Copyright (C) 2001 by Matthias Ettrich (add support for GTK applications )
** Copyright (C) 2001 by Waldo Bastian <bastian@kde.org>
** Copyright (C) 2002 by Karol Szwed <gallium@kde.org>
** This application is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include <config-workspace.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef Unsorted
#include <QBuffer>
#include <QDir>
#include <QtCore/QSettings>
#include <QtCore/QTextCodec>
#include <QToolTip>
//Added by qt3to4:
#include <QPixmap>
#include <QByteArray>
#include <QTextStream>
#include <QDateTime>
#include <QtDBus/QtDBus>
#include <ktoolinvocation.h>
#include <klauncher_iface.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <ksavefile.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <kstyle.h>

#include "krdb.h"
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <QX11Info>
#endif
inline const char * gtkEnvVar(int version)
{
    return 2==version ? "GTK2_RC_FILES" : "GTK_RC_FILES";
}

inline const char * sysGtkrc(int version)
{
    if(2==version)
    {
	if(access("/etc/opt/gnome/gtk-2.0", F_OK) == 0)
	    return "/etc/opt/gnome/gtk-2.0/gtkrc";
	else
	    return "/etc/gtk-2.0/gtkrc";
    }
    else
    {
	if(access("/etc/opt/gnome/gtk", F_OK) == 0)
	    return "/etc/opt/gnome/gtk/gtkrc";
	else
	    return "/etc/gtk/gtkrc";
    }
}

inline const char * userGtkrc(int version)
{
    return 2==version  ? "/.gtkrc-2.0" : "/.gtkrc";
}

// -----------------------------------------------------------------------------
static void applyGtkStyles(bool active, int version)
{
   QString gtkkde = KStandardDirs::locateLocal("config", 2==version?"gtkrc-2.0":"gtkrc");
   QByteArray gtkrc = getenv(gtkEnvVar(version));
   QStringList list = QFile::decodeName(gtkrc).split( ':');
   QString userHomeGtkrc = QDir::homePath()+userGtkrc(version);
   if (!list.contains(userHomeGtkrc))
      list.prepend(userHomeGtkrc);
   QLatin1String systemGtkrc = QLatin1String(sysGtkrc(version));
   if (!list.contains(systemGtkrc))
      list.prepend(systemGtkrc);
   list.removeAll(gtkkde);
   list.append(gtkkde);
   if (!active)
      ::unlink(QFile::encodeName(gtkkde));

   // Pass env. var to kdeinit.
   QString name = gtkEnvVar(version);
   QString value = QFile::encodeName(list.join(":"));
   KToolInvocation::klauncher()->setLaunchEnv(name, value);
}

// -----------------------------------------------------------------------------

static void applyQtColors( KConfigGroup kglobals, QSettings& settings, QPalette& newPal )
{
  QStringList actcg, inactcg, discg;
  /* export kde color settings */
  int i;
  for (i = 0; i < QPalette::NColorRoles; i++)
     actcg   << newPal.color(QPalette::Active,
                (QPalette::ColorRole) i).name();
  for (i = 0; i < QPalette::NColorRoles; i++)
     inactcg << newPal.color(QPalette::Inactive,
                (QPalette::ColorRole) i).name();
  for (i = 0; i < QPalette::NColorRoles; i++)
     discg   << newPal.color(QPalette::Disabled,
                (QPalette::ColorRole) i).name();

  settings.setValue("/qt/Palette/active", actcg);
  settings.setValue("/qt/Palette/inactive", inactcg);
  settings.setValue("/qt/Palette/disabled", discg);

  // export kwin's colors to qtrc for kstyle to use
  kglobals.changeGroup("WM");

  // active colors
  QColor clr = newPal.color( QPalette::Active, QPalette::Background );
  clr = kglobals.readEntry("activeBackground", clr);
  settings.setValue("/qt/KWinPalette/activeBackground", clr.name());
  if (QPixmap::defaultDepth() > 8)
    clr = clr.dark(110);
  clr = kglobals.readEntry("activeBlend", clr);
  settings.setValue("/qt/KWinPalette/activeBlend", clr.name());
  clr = newPal.color( QPalette::Active, QPalette::HighlightedText );
  clr = kglobals.readEntry("activeForeground", clr);
  settings.setValue("/qt/KWinPalette/activeForeground", clr.name());
  clr = newPal.color( QPalette::Active,QPalette::Background );
  clr = kglobals.readEntry("frame", clr);
  settings.setValue("/qt/KWinPalette/frame", clr.name());
  clr = kglobals.readEntry("activeTitleBtnBg", clr);
  settings.setValue("/qt/KWinPalette/activeTitleBtnBg", clr.name());

  // inactive colors
  clr = newPal.color(QPalette::Inactive, QPalette::Background);
  clr = kglobals.readEntry("inactiveBackground", clr);
  settings.setValue("/qt/KWinPalette/inactiveBackground", clr.name());
  if (QPixmap::defaultDepth() > 8)
    clr = clr.dark(110);
  clr = kglobals.readEntry("inactiveBlend", clr);
  settings.setValue("/qt/KWinPalette/inactiveBlend", clr.name());
  clr = newPal.color(QPalette::Inactive, QPalette::Background).dark();
  clr = kglobals.readEntry("inactiveForeground", clr);
  settings.setValue("/qt/KWinPalette/inactiveForeground", clr.name());
  clr = newPal.color(QPalette::Inactive, QPalette::Background);
  clr = kglobals.readEntry("inactiveFrame", clr);
  settings.setValue("/qt/KWinPalette/inactiveFrame", clr.name());
  clr = kglobals.readEntry("inactiveTitleBtnBg", clr);
  settings.setValue("/qt/KWinPalette/inactiveTitleBtnBg", clr.name());

  kglobals.changeGroup("KDE");
  settings.setValue("/qt/KDE/contrast", kglobals.readEntry("contrast", 7));
}

// -----------------------------------------------------------------------------

static void applyQtSettings( KConfigGroup kglobals, QSettings& settings )
{
    // export KDE's plugin library path to Trolltech.conf
    // This is only needed for Qt applications that run outside of a KDE session but still should
    // use a KDE style. In order to load the style (or any KDE code in that regard) Qt needs to
    // know the plugin path of KDE.

  QString qversion = qVersion();
  if ( qversion.count( '.' ) > 1 )
     qversion.truncate( qversion.lastIndexOf( '.' ) );
  if ( qversion.contains( '-' ) )
     qversion.truncate( qversion.lastIndexOf( '-' ) );

    // paths that KDE added
    QStringList kdeAdded = settings.value("/qt/KDE/kdeAddedLibraryPaths").toStringList();

    const QString &libPathKey = QString("/qt/%1/libraryPath").arg(qversion);
    // paths that Qt currently adds. Don't use toStringList! That's a different storage format
    QStringList libraryPath = settings.value(libPathKey, QString()).toString().split(QLatin1Char(':'), QString::SkipEmptyParts);

    // only keep entries that are not from KDE
    foreach (const QString &path, const_cast<const QStringList &>(kdeAdded)) {
        libraryPath.removeAll(path);
    }

    kdeAdded.clear();

    // paths that need to be in the list
    const QStringList &plugins = KGlobal::dirs()->resourceDirs("qtplugins");
    foreach (const QString &_path, plugins) {
        QString path = QDir(_path).canonicalPath();
        if (path.isEmpty() || kdeAdded.contains(path)) {
            continue;
        }
        kdeAdded.prepend(path);
        if (path.contains("/lib64/")) {
            path.replace("/lib64/", "/lib/");
            if (!kdeAdded.contains(path)) {
                kdeAdded.prepend(path);
            }
        }
    }
    foreach (const QString &path, const_cast<const QStringList &>(kdeAdded)) {
        libraryPath.append(path);
    }

    // Write the list out..
    settings.setValue("/qt/KDE/kdeAddedLibraryPaths", kdeAdded);
    settings.setValue(libPathKey, libraryPath.join(QLatin1String(":")));

  /* export widget style */
  kglobals.changeGroup("General");
  QString style = kglobals.readEntry("widgetStyle", KStyle::defaultStyle());
  if (!style.isEmpty())
    settings.setValue("/qt/style", style);

  /* export font settings */
  settings.setValue("/qt/font", KGlobalSettings::generalFont().toString());

  /* ##### looks like kcmfonts skips this, so we don't do this here */
/*bool usexft = kglobals.readEntry("AntiAliasing", false);
  kconfig.setGroup("General");
  settings.writeEntry("/qt/enableXft", usexft);
  settings.writeEntry("/qt/useXft", usexft); */

  /* export effects settings */
  kglobals.changeGroup("KDE");
  bool effectsEnabled = kglobals.readEntry("EffectsEnabled", false);
  bool fadeMenus = kglobals.readEntry("EffectFadeMenu", false);
  bool fadeTooltips = kglobals.readEntry("EffectFadeTooltip", false);
  bool animateCombobox = kglobals.readEntry("EffectAnimateCombo", false);

  QStringList guieffects;
  if (effectsEnabled) {
    guieffects << QString("general");
    if (fadeMenus)
      guieffects << QString("fademenu");
    if (animateCombobox)
      guieffects << QString("animatecombo");
    if (fadeTooltips)
      guieffects << QString("fadetooltip");
  }
  else
    guieffects << QString("none");

  settings.setValue("/qt/GUIEffects", guieffects);
}

// -----------------------------------------------------------------------------

static void addColorDef(QString& s, const char* n, const QColor& col)
{
  QString tmp;

  tmp.sprintf("#define %s #%02x%02x%02x\n",
              n, col.red(), col.green(), col.blue());

  s += tmp;
}


// -----------------------------------------------------------------------------

static void copyFile(QFile& tmp, QString const& filename, bool )
{
  QFile f( filename );
  if ( f.open(QIODevice::ReadOnly) ) {
      QByteArray buf( 8192, ' ' );
      while ( !f.atEnd() ) {
          int read = f.read( buf.data(), buf.size() );
          if ( read > 0 )
              tmp.write( buf.data(), read );
      }
  }
}


// -----------------------------------------------------------------------------

static QString item( int i ) {
    return QString::number( i / 255.0, 'f', 3 );
}

static QString color( const QColor& col )
{
    return QString( "{ %1, %2, %3 }" ).arg( item( col.red() ) ).arg( item( col.green() ) ).arg( item( col.blue() ) );
}

static void createGtkrc( bool exportColors, const QPalette& cg, int version )
{
    // lukas: why does it create in ~/.kde/share/config ???
    // pfeiffer: so that we don't overwrite the user's gtkrc.
    // it is found via the GTK_RC_FILES environment variable.
    KSaveFile saveFile( KStandardDirs::locateLocal( "config", 2==version?"gtkrc-2.0":"gtkrc" ) );
    if ( !saveFile.open() )
        return;

    QTextStream t ( &saveFile );
    t.setCodec( QTextCodec::codecForLocale () );

    t << i18n(
            "# created by KDE, %1\n"
            "#\n"
            "# If you do not want KDE to override your GTK settings, select\n"
            "# Appearance & Themes -> Colors in the Control Center and disable the checkbox\n"
            "# \"Apply colors to non-KDE applications\"\n"
            "#\n"
            "#\n", QDateTime::currentDateTime().toString());

    t << "style \"default\"" << endl;
    t << "{" << endl;
    if (exportColors)
    {
        t << "  bg[NORMAL] = " << color( cg.color( QPalette::Active, QPalette::Background ) ) << endl;
        t << "  bg[SELECTED] = " << color( cg.color(QPalette::Active, QPalette::Highlight) ) << endl;
        t << "  bg[INSENSITIVE] = " << color( cg.color( QPalette::Active, QPalette::Background ) ) << endl;
        t << "  bg[ACTIVE] = " << color( cg.color( QPalette::Active, QPalette::Mid ) ) << endl;
        t << "  bg[PRELIGHT] = " << color( cg.color( QPalette::Active, QPalette::Background ) ) << endl;
        t << endl;
        t << "  base[NORMAL] = " << color( cg.color( QPalette::Active, QPalette::Base ) ) << endl;
        t << "  base[SELECTED] = " << color( cg.color(QPalette::Active, QPalette::Highlight) ) << endl;
        t << "  base[INSENSITIVE] = " << color( cg.color( QPalette::Active, QPalette::Background ) ) << endl;
        t << "  base[ACTIVE] = " << color( cg.color(QPalette::Active, QPalette::Highlight) ) << endl;
        t << "  base[PRELIGHT] = " << color( cg.color(QPalette::Active, QPalette::Highlight) ) << endl;
        t << endl;
        t << "  text[NORMAL] = " << color( cg.color(QPalette::Active, QPalette::Text) ) << endl;
        t << "  text[SELECTED] = " << color( cg.color(QPalette::Active, QPalette::HighlightedText) ) << endl;
        t << "  text[INSENSITIVE] = " << color( cg.color( QPalette::Active, QPalette::Mid ) ) << endl;
        t << "  text[ACTIVE] = " << color( cg.color(QPalette::Active, QPalette::HighlightedText) ) << endl;
        t << "  text[PRELIGHT] = " << color( cg.color(QPalette::Active, QPalette::HighlightedText) ) << endl;
        t << endl;
        t << "  fg[NORMAL] = " << color ( cg.color( QPalette::Active, QPalette::Foreground ) ) << endl;
        t << "  fg[SELECTED] = " << color( cg.color(QPalette::Active, QPalette::HighlightedText) ) << endl;
        t << "  fg[INSENSITIVE] = " << color( cg.color( QPalette::Active, QPalette::Mid ) ) << endl;
        t << "  fg[ACTIVE] = " << color( cg.color( QPalette::Active, QPalette::Foreground ) ) << endl;
        t << "  fg[PRELIGHT] = " << color( cg.color( QPalette::Active, QPalette::Foreground ) ) << endl;
    }

    t << "}" << endl;
    t << endl;
    t << "class \"*\" style \"default\"" << endl;
    t << endl;
    if ( 2==version ) {  // we should maybe check for MacOS settings here
	t << "gtk-alternative-button-order = 1" << endl;
	t << endl;
    }

    if (exportColors)
    {
        // tooltips don't have the standard background color
        t << "style \"ToolTip\"" << endl;
        t << "{" << endl;
        QPalette group = QToolTip::palette();
        t << "  bg[NORMAL] = " << color( group.color( QPalette::Active, QPalette::Background ) ) << endl;
        t << "  base[NORMAL] = " << color( group.color( QPalette::Active, QPalette::Base ) ) << endl;
        t << "  text[NORMAL] = " << color( group.color( QPalette::Active, QPalette::Text ) ) << endl;
        t << "  fg[NORMAL] = " << color( group.color( QPalette::Active, QPalette::Foreground ) ) << endl;
        t << "}" << endl;
        t << endl;
        t << "widget \"gtk-tooltip\" style \"ToolTip\"" << endl;
        t << "widget \"gtk-tooltips\" style \"ToolTip\"" << endl;
        t << endl;


        // highlight the current (mouse-hovered) menu-item
        // not every button, checkbox, etc.
        t << "style \"MenuItem\"" << endl;
        t << "{" << endl;
        t << "  bg[PRELIGHT] = " << color( cg.color(QPalette::Highlight) ) << endl;
        t << "}" << endl;
        t << endl;
        t << "class \"*MenuItem\" style \"MenuItem\"" << endl;
        t << endl;
    }
}

// -----------------------------------------------------------------------------

void runRdb( uint flags )
{
  // Obtain the application palette that is about to be set.
  bool exportColors      = flags & KRdbExportColors;
  bool exportQtColors    = flags & KRdbExportQtColors;
  bool exportQtSettings  = flags & KRdbExportQtSettings;
  bool exportXftSettings = flags & KRdbExportXftSettings;

  KSharedConfigPtr kglobalcfg = KSharedConfig::openConfig( "kdeglobals" );
  KConfigGroup kglobals(kglobalcfg, "KDE");
  QPalette newPal = KGlobalSettings::createApplicationPalette(kglobalcfg);

  KTemporaryFile tmpFile;
  if (!tmpFile.open())
  {
    kDebug() << "Couldn't open temp file";
    exit(0);
  }

  // Export colors to non-(KDE/Qt) apps (e.g. Motif, GTK+ apps)
  if (exportColors)
  {
    KGlobal::dirs()->addResourceType("appdefaults", "data", "kdisplay/app-defaults/");
    KGlobal::locale()->insertCatalog("krdb");
    createGtkrc( true, newPal, 1 );
    createGtkrc( true, newPal, 2 );

    QString preproc;
    QColor backCol = newPal.color( QPalette::Active, QPalette::Background );
    addColorDef(preproc, "FOREGROUND"         , newPal.color( QPalette::Active, QPalette::Foreground ) );
    addColorDef(preproc, "BACKGROUND"         , backCol);
    addColorDef(preproc, "HIGHLIGHT"          , backCol.light(100+(2*KGlobalSettings::contrast()+4)*16/1));
    addColorDef(preproc, "LOWLIGHT"           , backCol.dark(100+(2*KGlobalSettings::contrast()+4)*10));
    addColorDef(preproc, "SELECT_BACKGROUND"  , newPal.color( QPalette::Active, QPalette::Highlight));
    addColorDef(preproc, "SELECT_FOREGROUND"  , newPal.color( QPalette::Active, QPalette::HighlightedText));
    addColorDef(preproc, "WINDOW_BACKGROUND"  , newPal.color( QPalette::Active, QPalette::Base ) );
    addColorDef(preproc, "WINDOW_FOREGROUND"  , newPal.color( QPalette::Active, QPalette::Foreground ) );
    addColorDef(preproc, "INACTIVE_BACKGROUND", KGlobalSettings::inactiveTitleColor());
    addColorDef(preproc, "INACTIVE_FOREGROUND", KGlobalSettings::inactiveTitleColor());
    addColorDef(preproc, "ACTIVE_BACKGROUND"  , KGlobalSettings::activeTitleColor());
    addColorDef(preproc, "ACTIVE_FOREGROUND"  , KGlobalSettings::activeTitleColor());
    //---------------------------------------------------------------

    tmpFile.write( preproc.toLatin1(), preproc.length() );

    QStringList list;

    QStringList adPaths = KGlobal::dirs()->findDirs("appdefaults", "");
    for (QStringList::ConstIterator it = adPaths.begin(); it != adPaths.end(); ++it) {
      QDir dSys( *it );

      if ( dSys.exists() ) {
        dSys.setFilter( QDir::Files );
        dSys.setSorting( QDir::Name );
        dSys.setNameFilters(QStringList("*.ad"));
        list += dSys.entryList();
      }
    }

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
      copyFile(tmpFile, KStandardDirs::locate("appdefaults", *it ), true);
  }

  // Merge ~/.Xresources or fallback to ~/.Xdefaults
  QString homeDir = QDir::homePath();
  QString xResources = homeDir + "/.Xresources";

  // very primitive support for ~/.Xresources by appending it
  if ( QFile::exists( xResources ) )
    copyFile(tmpFile, xResources, true);
  else
    copyFile(tmpFile, homeDir + "/.Xdefaults", true);

  // Export the Xcursor theme & size settings
  KConfigGroup mousecfg(KSharedConfig::openConfig( "kcminputrc" ), "Mouse" );
  QString theme = mousecfg.readEntry("cursorTheme", QString());
  QString size  = mousecfg.readEntry("cursorSize", QString());
  QString contents;

  if (!theme.isNull())
    contents = "Xcursor.theme: " + theme + '\n';

  if (!size.isNull())
    contents += "Xcursor.size: " + size + '\n';

  if (exportXftSettings)
  {
    kglobals.changeGroup("General");

    if (kglobals.hasKey("XftAntialias"))
    {
      contents += "Xft.antialias: ";
      if(kglobals.readEntry("XftAntialias", true))
        contents += "1\n";
      else
        contents += "0\n";
    }

    if (kglobals.hasKey("XftHintStyle"))
    {
      QString hintStyle = kglobals.readEntry("XftHintStyle", "hintmedium");
      contents += "Xft.hinting: ";
      if(hintStyle.isEmpty())
        contents += "-1\n";
      else
      {
        if(hintStyle!="hintnone")
          contents += "1\n";
        else
          contents += "0\n";
        contents += "Xft.hintstyle: " + hintStyle + '\n';
      }
    }

    if (kglobals.hasKey("XftSubPixel"))
    {
      QString subPixel = kglobals.readEntry("XftSubPixel");
      if(!subPixel.isEmpty())
        contents += "Xft.rgba: " + subPixel + '\n';
    }

    KConfig _cfgfonts( "kcmfonts" );
    KConfigGroup cfgfonts(&_cfgfonts, "General");

    if( cfgfonts.readEntry( "forceFontDPI", 0 ) != 0 )
      contents += "Xft.dpi: " + cfgfonts.readEntry( "forceFontDPI" ) + '\n';
    else
    {
      KProcess proc;
      proc << "xrdb" << "-quiet" << "-remove" << "-nocpp";
      proc.start();
      if (proc.waitForStarted())
      {
        proc.write( QByteArray( "Xft.dpi\n" ) );
        proc.closeWriteChannel();
        proc.waitForFinished();
      }
    }
  }

  if (contents.length() > 0)
    tmpFile.write( contents.toLatin1(), contents.length() );

  tmpFile.flush();

  KProcess proc;
#ifndef NDEBUG
  proc << "xrdb" << "-merge" << tmpFile.fileName();
#else
  proc << "xrdb" << "-quiet" << "-merge" << tmpFile.fileName();
#endif
  proc.execute();

  applyGtkStyles(exportColors, 1);
  applyGtkStyles(exportColors, 2);

  /* Qt exports */
  if ( exportQtColors || exportQtSettings )
  {
    QSettings* settings = new QSettings(QLatin1String("Trolltech"));

    if ( exportQtColors )
      applyQtColors( kglobals, *settings, newPal );    // For kcmcolors

    if ( exportQtSettings )
      applyQtSettings( kglobals, *settings );          // For kcmstyle

    delete settings;
    QApplication::flush();
#ifdef Q_WS_X11
    // We let KIPC take care of ourselves, as we are in a KDE app with
    // QApp::setDesktopSettingsAware(false);
    // Instead of calling QApp::x11_apply_settings() directly, we instead
    // modify the timestamp which propagates the settings changes onto
    // Qt-only apps without adversely affecting ourselves.

    // Cheat and use the current timestamp, since we just saved to qtrc.
    QDateTime settingsstamp = QDateTime::currentDateTime();

    static Atom qt_settings_timestamp = 0;
    if (!qt_settings_timestamp) {
	 QString atomname("_QT_SETTINGS_TIMESTAMP_");
	 atomname += XDisplayName( 0 ); // Use the $DISPLAY envvar.
	 qt_settings_timestamp = XInternAtom( QX11Info::display(), atomname.toLatin1(), False);
    }

    QBuffer stamp;
    QDataStream s(&stamp.buffer(), QIODevice::WriteOnly);
    s << settingsstamp;
    XChangeProperty( QX11Info::display(), QX11Info::appRootWindow(), qt_settings_timestamp,
		     qt_settings_timestamp, 8, PropModeReplace,
		     (unsigned char*) stamp.buffer().data(),
		     stamp.buffer().size() );
    QApplication::flush();
#endif
  }
}

