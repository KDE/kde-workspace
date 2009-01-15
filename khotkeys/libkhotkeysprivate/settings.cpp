/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _SETTINGS_CPP_

#include "settings.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include "triggers.h"
#include "conditions.h"
#include "action_data.h"

namespace KHotKeys
{

// Settings

Settings::Settings()
    : m_actions( NULL ),
      gestures_disabled(false),
      gesture_mouse_button(0),
      gesture_timeout(0),
      gestures_exclude(NULL),
      daemon_disabled(false)
    {
    }


Settings::~Settings()
    {
    delete m_actions; m_actions = 0;
    }


Action_data_group *Settings::actions()
    {
    return m_actions;
    }


bool Settings::areGesturesDisabled() const
    {
    return gestures_disabled;
    }


void Settings::disableDaemon()
    {
    daemon_disabled = true;
    }


void Settings::disableGestures()
    {
    gestures_disabled = true;
    }


void Settings::enableDaemon()
    {
    daemon_disabled = false;
    }


void Settings::enableGestures()
    {
    gestures_disabled = false;
    }


int Settings::gestureMouseButton() const
    {
    return gesture_mouse_button;
    }


Windowdef_list *Settings::gesturesExclude()
    {
    return gestures_exclude;
    }


const Windowdef_list *Settings::gesturesExclude() const
    {
    return gestures_exclude;
    }


int Settings::gestureTimeOut() const
    {
    return gesture_timeout;
    }


bool Settings::isDaemonDisabled() const
    {
    return daemon_disabled;
    }


void Settings::setActions( Action_data_group *actions )
    {
    delete m_actions;
    m_actions = actions;
    }


void Settings::setGesturesExclude( Windowdef_list *gestures )
    {
    delete gestures_exclude;
    gestures_exclude = gestures;
    }


void Settings::setGestureMouseButton( int mouse_button )
    {
    gesture_mouse_button = mouse_button;
    }


void Settings::setGestureTimeOut(int timeout)
    {
    gesture_timeout = timeout;
    }


void Settings::setVoiceShortcut( const KShortcut &shortcut )
    {
    voice_shortcut = shortcut;
    }


Action_data_group *Settings::takeActions()
    {
    Action_data_group *res = m_actions;
    m_actions = 0;
    return res;
    }


KShortcut Settings::voiceShortcut() const
    {
    return voice_shortcut;
    }


bool Settings::read_settings( bool include_disabled_P )
    {
    delete m_actions; m_actions = 0;

    KConfig cfg( KHOTKEYS_CONFIG_FILE );
    return read_settings( cfg, include_disabled_P, ImportNone );
    }

bool Settings::import( KConfig& cfg_P, bool ask_P )
    {
    return read_settings( cfg_P, true, ask_P ? ImportAsk : ImportSilent );
    }

bool Settings::read_settings( KConfig& cfg_P, bool include_disabled_P, ImportType import_P )
    {
    if( m_actions == NULL )
        m_actions = new Action_data_group( NULL, "should never see", "should never see",
            NULL, Action_data_group::SYSTEM_ROOT, true );
    if( cfg_P.groupList().count() == 0 ) // empty
        return false;
    KConfigGroup mainGroup( &cfg_P, "Main" ); // main group
    if( import_P == ImportNone ) // reading main cfg file
        already_imported = mainGroup.readEntry( "AlreadyImported",QStringList() );
    else
        {
        QString import_id = mainGroup.readEntry( "ImportId" );
        if( !import_id.isEmpty())
            {
            if( already_imported.contains( import_id ))
                {
                if( import_P == ImportSilent
                    || KMessageBox::warningContinueCancel( NULL,
                        i18n( "This \"actions\" file has already been imported before. "
                              "Are you sure you want to import it again?" )) != KMessageBox::Continue )
                    return true; // import "successful"
                }
            else
                already_imported.append( import_id );
            }
        else
            {
            if( import_P != ImportSilent
                && KMessageBox::warningContinueCancel( NULL,
                    i18n( "This \"actions\" file has no ImportId field and therefore it cannot be determined "
                          "whether or not it has been imported already. Are you sure you want to import it?" ))
                    == KMessageBox::Cancel )
                return true;
            }
        }
    int version = mainGroup.readEntry( "Version", -1234576 );
    switch( version )
        {
        case 1:
            read_settings_v1( cfg_P );
          break;
        case 2:
            read_settings_v2( cfg_P, include_disabled_P );
          break;
        default:
            kWarning( 1217 ) << "Unknown cfg. file version\n";
          return false;
        case -1234576: // no config file
            if( import_P ) // if importing, this is an error
                return false;
          break;
        }
    if( import_P != ImportNone )
        return true; // don't read global settings
    daemon_disabled = mainGroup.readEntry( "Disabled", false);
    KConfigGroup gesturesConfig( &cfg_P, "Gestures" );
    gestures_disabled = gesturesConfig.readEntry( "Disabled", true);
    gesture_mouse_button = gesturesConfig.readEntry( "MouseButton", 2 );
    gesture_mouse_button = qBound( 2, gesture_mouse_button, 9 );
    gesture_timeout = gesturesConfig.readEntry( "Timeout", 300 );
    KConfigGroup gesturesExcludeConfig( &cfg_P, "GesturesExclude" );
    delete gestures_exclude;
    gestures_exclude = new Windowdef_list( gesturesExcludeConfig );
    KConfigGroup voiceConfig( &cfg_P, "Voice" );
    voice_shortcut=KShortcut( voiceConfig.readEntry("Shortcut" , "")  );
    return true;
    }

void Settings::write_settings( Action_data_group *action_list )
    {
    KConfig cfg( KHOTKEYS_CONFIG_FILE );

    if (action_list==0)
        {
        action_list = m_actions;
        }

// CHECKME    smazat stare sekce ?
    QStringList groups = cfg.groupList();
    for( QStringList::ConstIterator it = groups.begin();
         it != groups.end();
         ++it )
        cfg.deleteGroup( *it );
    KConfigGroup mainGroup( &cfg, "Main" ); // main group
    mainGroup.writeEntry( "Version", 2 ); // now it's version 2 cfg. file
    mainGroup.writeEntry( "AlreadyImported", already_imported );
    KConfigGroup dataGroup( &cfg,  "Data" );
    int cnt = write_actions_recursively_v2( dataGroup, action_list, true );
    mainGroup.writeEntry( "Autostart", cnt != 0 && !daemon_disabled );
    mainGroup.writeEntry( "Disabled", daemon_disabled );
    KConfigGroup gesturesConfig( &cfg, "Gestures" );
    gesturesConfig.writeEntry( "Disabled", gestures_disabled );
    gesturesConfig.writeEntry( "MouseButton", gesture_mouse_button );
    gesturesConfig.writeEntry( "Timeout", gesture_timeout );
    if( gestures_exclude != NULL )
        {
        KConfigGroup gesturesExcludeConfig( &cfg, "GesturesExclude" );
        gestures_exclude->cfg_write( gesturesExcludeConfig );
        }
    else
        cfg.deleteGroup( "GesturesExclude" );
    KConfigGroup voiceConfig( &cfg, "Voice" );
    voiceConfig.writeEntry("Shortcut" , voice_shortcut.toString() );

    }


// return value means the number of enabled actions written in the cfg file
// i.e. 'Autostart' for value > 0 should be on
int Settings::write_actions_recursively_v2( KConfigGroup& cfg_P, Action_data_group* parent_P, bool enabled_P )
    {
    int enabled_cnt = 0;
    QString save_cfg_group = cfg_P.name();
    int cnt = 0;
    if( parent_P )
        {
        for( Action_data_group::ConstIterator it = parent_P->first_child();
            it != parent_P->after_last_child();
             ++it )
            {
            ++cnt;
            if( enabled_P && (*it)->enabled( true ))
                ++enabled_cnt;
            KConfigGroup itConfig( cfg_P.config(), save_cfg_group + '_' + QString::number( cnt ));
            ( *it )->cfg_write( itConfig );
            Action_data_group* grp = dynamic_cast< Action_data_group* >( *it );
            if( grp != NULL )
                enabled_cnt += write_actions_recursively_v2( itConfig, grp, enabled_P && (*it)->enabled( true ));
            }
        }
    cfg_P.writeEntry( "DataCount", cnt );
    return enabled_cnt;
    }

void Settings::read_settings_v2( KConfig& cfg_P, bool include_disabled_P  )
    {
    KConfigGroup dataGroup( &cfg_P, "Data" );
    read_actions_recursively_v2( dataGroup, m_actions, include_disabled_P );
    }

void Settings::read_actions_recursively_v2( KConfigGroup& cfg_P, Action_data_group* parent_P,
    bool include_disabled_P )
    {
    QString save_cfg_group = cfg_P.name();
    int cnt = cfg_P.readEntry( "DataCount",0 );
    for( int i = 1;
         i <= cnt;
         ++i )
        {
        KConfigGroup itConfig( cfg_P.config(), save_cfg_group + '_' + QString::number( i ));
        if( include_disabled_P || Action_data_base::cfg_is_enabled( itConfig ))
            {
            Action_data_base* new_action = Action_data_base::create_cfg_read( itConfig, parent_P );
            Action_data_group* grp = dynamic_cast< Action_data_group* >( new_action );
            if( grp != NULL )
                read_actions_recursively_v2( itConfig, grp, include_disabled_P );
            }
        }
    }

// backward compatibility
void Settings::read_settings_v1( KConfig& cfg_P )
    {
    KConfigGroup mainGroup( &cfg_P, "Main" );
    int sections = mainGroup.readEntry( "Num_Sections", 0 );
    Action_data_group* menuentries = NULL;
    for( Action_data_group::ConstIterator it = m_actions->first_child();
         it != m_actions->after_last_child();
         ++it )
        {
        Action_data_group* tmp = dynamic_cast< Action_data_group* >( *it );
        if( tmp == NULL )
            continue;
        if( tmp->system_group() == Action_data_group::SYSTEM_MENUENTRIES )
            {
            menuentries = tmp;
            break;
            }
        }
    for( int sect = 1;
         sect <= sections;
         ++sect )
        {
        QString group = QString( "Section%1" ).arg( sect );
        if( !cfg_P.hasGroup( group ))
            continue;
        KConfigGroup sectionConfig( &cfg_P, group );
        QString name = sectionConfig.readEntry( "Name" );
        if( name.isNull() )
            continue;
        QString shortcut = sectionConfig.readEntry( "Shortcut" );
        if( shortcut.isNull() )
            continue;
        QString run = sectionConfig.readEntry( "Run" );
        if( run.isNull() )
            continue;
        bool menuentry = sectionConfig.readEntry( "MenuEntry", false);
        // CHECKME tohle pridavani az pak je trosku HACK
        if( menuentry )
            {
            if( menuentries == NULL )
                {
                menuentries = new Action_data_group( m_actions,
                    i18n( MENU_EDITOR_ENTRIES_GROUP_NAME ),
                    i18n( "These entries were created using Menu Editor." ), NULL,
                    Action_data_group::SYSTEM_MENUENTRIES, true );
                menuentries->set_conditions( new Condition_list( "", menuentries ));
                }
            ( void ) new Menuentry_shortcut_action_data( menuentries, name, "",
                KShortcut( shortcut ), run );
            }
        else
            {
            ( void ) new Command_url_shortcut_action_data( m_actions, name, "",
                KShortcut( shortcut ), run );
            }
        }
    }

} // namespace KHotKeys
