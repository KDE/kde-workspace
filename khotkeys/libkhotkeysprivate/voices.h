/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005 Olivier Goffart  <ogoffart @ kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef VOICES_H_
#define VOICES_H_

#include <QWidget>
#include <kshortcut.h>

class Sound;
class QTimer;
class KAction;

namespace KHotKeys
{

class Voice;
class SoundRecorder;

class Voice_trigger;
class VoiceSignature;


class Q_DECL_EXPORT Voice  : public QObject
    {
    Q_OBJECT
    public:
        Voice( bool enabled_P, QObject* parent_P );
        virtual ~Voice();
        void enable( bool enable_P );

		void register_handler( Voice_trigger* );
		void unregister_handler( Voice_trigger* );
//		bool x11Event( XEvent* e );
		
		void set_shortcut( const KShortcut &k);
		
		/**
		 * return QString() is a new signature is far enough from others signature
		 * otherwise, return the stringn which match.
		 */
		QString isNewSoundFarEnough(const VoiceSignature& s, const QString& currentTrigger);
		
		bool doesVoiceCodeExists(const QString &s);

    public Q_SLOTS:
         void record_start();
         void record_stop();

    private Q_SLOTS:
		void slot_sound_recorded( const Sound & );
		void slot_key_pressed();
		void slot_timeout();

    Q_SIGNALS:
        void handle_voice( const QString &voice );
    private:

        bool _enabled;
        bool _recording;

		QList<Voice_trigger *> _references;
		SoundRecorder *_recorder;
		
		KShortcut _shortcut;
		KAction *_kga;
		
		QTimer *_timer;
    };

	
Q_DECL_EXPORT extern Voice* voice_handler;

} // namespace KHotKeys

#endif
