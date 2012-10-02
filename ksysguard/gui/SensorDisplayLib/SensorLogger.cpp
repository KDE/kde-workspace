/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2001 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QTreeView>
#include <QtGui/QHBoxLayout>
#include <QtXml/QDomNodeList>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knotification.h>
#include <ksgrd/SensorManager.h>
#include <kdebug.h>
#include "StyleEngine.h"

#include "SensorLoggerDlg.h"
#include "SensorLoggerSettings.h"
#include "SensorLogger.h"

#define NONE -1

LogSensorView::LogSensorView( QWidget *parent )
 : QTreeView( parent )
{
}

void LogSensorView::contextMenuEvent( QContextMenuEvent *event )
{
  const QModelIndex index = indexAt( event->pos() );

  emit contextMenuRequest( index, viewport()->mapToGlobal( event->pos() ) );
}

class LogSensorModel : public QAbstractTableModel
{
  public:
    LogSensorModel( QObject *parent = 0 )
      : QAbstractTableModel( parent )
    {
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
      Q_UNUSED( parent );

      return 5;
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
      Q_UNUSED( parent );

      return mSensors.count();
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
      if ( !index.isValid() )
        return QVariant();

      if ( index.row() >= mSensors.count() || index.row() < 0 )
        return QVariant();

      LogSensor *sensor = mSensors[ index.row() ];

      if ( role == Qt::DisplayRole ) {
        switch ( index.column() ) {
          case 1:
            return sensor->timerInterval();
            break;
          case 2:
            return sensor->sensorName();
            break;
          case 3:
            return sensor->hostName();
            break;
          case 4:
            return sensor->fileName();
            break;
        }
      } else if ( role == Qt::DecorationRole ) {
        static QPixmap runningPixmap = KIconLoader::global()->loadIcon( "running", KIconLoader::Small, KIconLoader::SizeSmall );
        static QPixmap waitingPixmap = KIconLoader::global()->loadIcon( "waiting", KIconLoader::Small, KIconLoader::SizeSmall );

        if ( index.column() == 0 ) {
          if ( sensor->isLogging() )
            return runningPixmap;
          else
            return waitingPixmap;
        }
      } else if ( role == Qt::ForegroundRole ) {
        if ( sensor->limitReached() )
          return mAlarmColor;
        else
          return mForegroundColor;
      } else if ( role == Qt::BackgroundRole ) {
          return mBackgroundColor;
      }

      return QVariant();
    }

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
    {
      if ( orientation == Qt::Vertical )
        return QVariant();

      if ( role == Qt::DisplayRole ) {
        switch ( section ) {
          case 0:
            return i18nc("@title:column", "Logging");
            break;
          case 1:
            return i18nc("@title:column", "Timer Interval");
            break;
          case 2:
            return i18nc("@title:column", "Sensor Name");
            break;
          case 3:
            return i18nc("@title:column", "Host Name");
            break;
          case 4:
            return i18nc("@title:column", "Log File");
            break;
          default:
            return QVariant();
        }
      }

      return QVariant();
    }

    void addSensor( LogSensor *sensor )
    {
      mSensors.append( sensor );

      connect( sensor, SIGNAL(changed()), this, SIGNAL(layoutChanged()) );

      emit layoutChanged();
    }

    void removeSensor( LogSensor *sensor )
    {
      delete mSensors.takeAt( mSensors.indexOf( sensor ) );

      emit layoutChanged();
    }

    LogSensor* sensor( const QModelIndex &index ) const
    {
      if ( !index.isValid() || index.row() >= mSensors.count() || index.row() < 0 )
        return 0;

      return mSensors[ index.row() ];
    }

    void clear()
    {
      qDeleteAll( mSensors );
      mSensors.clear();
    }

    const QList<LogSensor*> sensors() const
    {
      return mSensors;
    }

    void setForegroundColor( const QColor &color ) { mForegroundColor = color; }
    QColor foregroundColor() const { return mForegroundColor; }

    void setBackgroundColor( const QColor &color ) { mBackgroundColor = color; }
    QColor backgroundColor() const { return mBackgroundColor; }

    void setAlarmColor( const QColor &color ) { mAlarmColor = color; }
    QColor alarmColor() const { return mAlarmColor; }

  private:

    QColor mForegroundColor;
    QColor mBackgroundColor;
    QColor mAlarmColor;

    QList<LogSensor*> mSensors;
};

LogSensor::LogSensor( QObject *parent )
  : QObject( parent ),
    mTimerID( NONE ),
    mLowerLimitActive( false ),
    mUpperLimitActive( 0 ),
    mLowerLimit( 0 ),
    mUpperLimit( 0 ),
    mLimitReached( false )
{
}

LogSensor::~LogSensor()
{
}

void LogSensor::setHostName( const QString& name )
{
  mHostName = name;
}

QString LogSensor::hostName() const
{
  return mHostName;
}

void LogSensor::setSensorName( const QString& name )
{
  mSensorName = name;
}

QString LogSensor::sensorName() const
{
  return mSensorName;
}

void LogSensor::setFileName( const QString& name )
{
  mFileName = name;
}

QString LogSensor::fileName() const
{
  return mFileName;
}

void LogSensor::setUpperLimitActive( bool value )
{
  mUpperLimitActive = value;
}

bool LogSensor::upperLimitActive() const
{
  return mUpperLimitActive;
}

void LogSensor::setLowerLimitActive( bool value )
{
  mLowerLimitActive = value;
}

bool LogSensor::lowerLimitActive() const
{
  return mLowerLimitActive;
}

void LogSensor::setUpperLimit( double value )
{
  mUpperLimit = value;
}

double LogSensor::upperLimit() const
{
  return mUpperLimit;
}

void LogSensor::setLowerLimit( double value )
{
  mLowerLimit = value;
}

double LogSensor::lowerLimit() const
{
  return mLowerLimit;
}

void LogSensor::setTimerInterval( int interval )
{
  mTimerInterval = interval;

  if ( mTimerID != NONE ) {
    timerOff();
    timerOn();
  }
}

int LogSensor::timerInterval() const
{
  return mTimerInterval;
}

bool LogSensor::isLogging() const
{
  return mTimerID != NONE;
}

bool LogSensor::limitReached() const
{
  return mLimitReached;
}

void LogSensor::timerOff()
{
  if ( mTimerID > 0 )
    killTimer( mTimerID );
  mTimerID = NONE;
}

void LogSensor::timerOn()
{
  mTimerID = startTimer( mTimerInterval * 1000 );
}

void LogSensor::startLogging()
{
  timerOn();
}

void LogSensor::stopLogging()
{
  timerOff();
}

void LogSensor::timerEvent ( QTimerEvent * event )
{
  Q_UNUSED(event);
  KSGRD::SensorMgr->sendRequest( mHostName, mSensorName, static_cast<KSGRD::SensorClient*>(this), 42 );
}

void LogSensor::answerReceived( int id, const QList<QByteArray>& answer ) //virtual
{
  QFile mLogFile( mFileName );

  if ( !mLogFile.open( QIODevice::ReadWrite | QIODevice::Append ) ) {
    stopLogging();
    return;
  }

  switch ( id ) {
    case 42: {
      QTextStream stream( &mLogFile );
      double value = 0;
      if ( !answer.isEmpty() )
        value = answer[ 0 ].toDouble();

      if ( mLowerLimitActive && value < mLowerLimit ) {
        timerOff();
        mLimitReached = true;

        // send notification
        KNotification::event( "sensor_alarm", QString( "sensor '%1' at '%2' reached lower limit" )
                            .arg( mSensorName ).arg( mHostName), QPixmap(), 0 );

        timerOn();
      } else if ( mUpperLimitActive && value > mUpperLimit ) {
        timerOff();
        mLimitReached = true;

        // send notification
        KNotification::event( "sensor_alarm", QString( "sensor '%1' at '%2' reached upper limit" )
                            .arg( mSensorName).arg( mHostName), QPixmap(), 0 );

        timerOn();
      } else {
        mLimitReached = false;
      }

      const QDate date = QDateTime::currentDateTime().date();
      const QTime time = QDateTime::currentDateTime().time();

      stream << QString( "%1 %2 %3 %4 %5: %6\n" ).arg( date.shortMonthName( date.month() ) )
                                                 .arg( date.day() ).arg( time.toString() )
                                                 .arg( mHostName).arg( mSensorName ).arg( value );
    }
  }

  emit changed();

  mLogFile.close();
}

SensorLogger::SensorLogger( QWidget *parent, const QString& title, SharedSettings *workSheetSettings )
  : KSGRD::SensorDisplay( parent, title, workSheetSettings )
{
  mModel = new LogSensorModel( this );
  mModel->setForegroundColor( KSGRD::Style->firstForegroundColor() );
  mModel->setBackgroundColor( KSGRD::Style->backgroundColor() );
  mModel->setAlarmColor( KSGRD::Style->alarmColor() );
  
  QLayout *layout = new QHBoxLayout(this);
  mView = new LogSensorView( this );
  layout->addWidget(mView);
  setLayout(layout);

  mView->header()->setStretchLastSection( true );
  mView->setRootIsDecorated( false );
  mView->setItemsExpandable( false );
  mView->setModel( mModel );
  setPlotterWidget( mView );

  connect( mView, SIGNAL(contextMenuRequest(QModelIndex,QPoint)),
           this, SLOT(contextMenuRequest(QModelIndex,QPoint)) );

  QPalette palette = mView->palette();
  palette.setColor( QPalette::Base, KSGRD::Style->backgroundColor() );
  mView->setPalette( palette );

  setTitle( i18n( "Sensor Logger" ) );
  setMinimumSize( 50, 25 );
}

SensorLogger::~SensorLogger(void)
{
}

bool SensorLogger::addSensor( const QString& hostName, const QString& sensorName, const QString& sensorType, const QString& )
{
  if ( sensorType != "integer" && sensorType != "float" )
    return false;

  SensorLoggerDlg dlg( this );

  if ( dlg.exec() ) {
    if ( !dlg.fileName().isEmpty() ) {
      LogSensor *sensor = new LogSensor( mModel );

      sensor->setHostName( hostName );
      sensor->setSensorName( sensorName );
      sensor->setFileName( dlg.fileName() );
      sensor->setTimerInterval( dlg.timerInterval() );
      sensor->setLowerLimitActive( dlg.lowerLimitActive() );
      sensor->setUpperLimitActive( dlg.upperLimitActive() );
      sensor->setLowerLimit( dlg.lowerLimit() );
      sensor->setUpperLimit( dlg.upperLimit() );

      mModel->addSensor( sensor );
    }
  } else {
    return false;  //User cancelled dialog, so don't add sensor logger
  }

  return true;
}

bool SensorLogger::editSensor( LogSensor* sensor )
{
  SensorLoggerDlg dlg( this );

  dlg.setFileName( sensor->fileName() );
  dlg.setTimerInterval( sensor->timerInterval() );
  dlg.setLowerLimitActive( sensor->lowerLimitActive() );
  dlg.setLowerLimit( sensor->lowerLimit() );
  dlg.setUpperLimitActive( sensor->upperLimitActive() );
  dlg.setUpperLimit( sensor->upperLimit() );

  if ( dlg.exec() ) {
    if ( !dlg.fileName().isEmpty() ) {
      sensor->setFileName( dlg.fileName() );
      sensor->setTimerInterval( dlg.timerInterval() );
      sensor->setLowerLimitActive( dlg.lowerLimitActive() );
      sensor->setUpperLimitActive( dlg.upperLimitActive() );
      sensor->setLowerLimit( dlg.lowerLimit() );
      sensor->setUpperLimit( dlg.upperLimit() );
    }
  }

  return true;
}

void SensorLogger::configureSettings()
{
  SensorLoggerSettings dlg( this );

  dlg.setTitle( title() );
  dlg.setForegroundColor( mModel->foregroundColor() );
  dlg.setBackgroundColor( mModel->backgroundColor() );
  dlg.setAlarmColor( mModel->alarmColor() );

  if ( dlg.exec() ) {
    setTitle( dlg.title() );

    mModel->setForegroundColor( dlg.foregroundColor() );
    mModel->setBackgroundColor( dlg.backgroundColor() );
    mModel->setAlarmColor( dlg.alarmColor() );

    QPalette palette = mView->palette();
    palette.setColor( QPalette::Base, dlg.backgroundColor() );
    mView->setPalette( palette );
  }
}

void SensorLogger::applyStyle()
{
  mModel->setForegroundColor( KSGRD::Style->firstForegroundColor() );
  mModel->setBackgroundColor( KSGRD::Style->backgroundColor() );
  mModel->setAlarmColor( KSGRD::Style->alarmColor() );

  QPalette palette = mView->palette();
  palette.setColor( QPalette::Base, KSGRD::Style->backgroundColor() );
  mView->setPalette( palette );
}

bool SensorLogger::restoreSettings( QDomElement& element )
{
  mModel->setForegroundColor( restoreColor( element, "textColor", Qt::green) );
  mModel->setBackgroundColor( restoreColor( element, "backgroundColor", Qt::black ) );
  mModel->setAlarmColor( restoreColor( element, "alarmColor", Qt::red ) );

  mModel->clear();

  QDomNodeList dnList = element.elementsByTagName( "logsensors" );
  for ( int i = 0; i < dnList.count(); i++ ) {
    QDomElement element = dnList.item( i ).toElement();
    LogSensor* sensor = new LogSensor( mModel );

    sensor->setHostName( element.attribute("hostName") );
    sensor->setSensorName( element.attribute("sensorName") );
    sensor->setFileName( element.attribute("fileName") );
    sensor->setTimerInterval( element.attribute("timerInterval").toInt() );
    sensor->setLowerLimitActive( element.attribute("lowerLimitActive").toInt() );
    sensor->setLowerLimit( element.attribute("lowerLimit").toDouble() );
    sensor->setUpperLimitActive( element.attribute("upperLimitActive").toInt() );
    sensor->setUpperLimit( element.attribute("upperLimit").toDouble() );

    mModel->addSensor( sensor );
  }

  SensorDisplay::restoreSettings( element );

  QPalette palette = mView->palette();
  palette.setColor( QPalette::Base, mModel->backgroundColor() );
  mView->setPalette( palette );

  return true;
}

bool SensorLogger::saveSettings( QDomDocument& doc, QDomElement& element )
{
  saveColor( element, "textColor", mModel->foregroundColor() );
  saveColor( element, "backgroundColor", mModel->backgroundColor() );
  saveColor( element, "alarmColor", mModel->alarmColor() );

  const QList<LogSensor*> sensors = mModel->sensors();
  for ( int i = 0; i < sensors.count(); ++i ) {
    LogSensor *sensor = sensors[ i ];
    QDomElement log = doc.createElement( "logsensors" );
    log.setAttribute("sensorName", sensor->sensorName());
    log.setAttribute("hostName", sensor->hostName());
    log.setAttribute("fileName", sensor->fileName());
    log.setAttribute("timerInterval", sensor->timerInterval());
    log.setAttribute("lowerLimitActive", QString("%1").arg(sensor->lowerLimitActive()));
    log.setAttribute("lowerLimit", QString("%1").arg(sensor->lowerLimit()));
    log.setAttribute("upperLimitActive", QString("%1").arg(sensor->upperLimitActive()));
    log.setAttribute("upperLimit", QString("%1").arg(sensor->upperLimit()));

    element.appendChild( log );
  }

  SensorDisplay::saveSettings( doc, element );

  return true;
}

void SensorLogger::answerReceived( int, const QList<QByteArray>& ) //virtual
{
 // we do not use this, since all answers are received by the LogSensors
}

void SensorLogger::contextMenuRequest( const QModelIndex &index, const QPoint &point )
{
  LogSensor *sensor = mModel->sensor( index );

  QMenu pm;

  QAction *action = 0;
  if (hasSettingsDialog()) {
    action = pm.addAction(i18n("&Properties"));
    action->setData( 1 );
  }
  if(!mSharedSettings->locked) {  

    action = pm.addAction(i18n("&Remove Display"));
    action->setData( 2 );

    pm.addSeparator();

    action = pm.addAction(i18n("&Remove Sensor"));
    action->setData( 3 );
    if ( !sensor )
      action->setEnabled( false );

    action = pm.addAction(i18n("&Edit Sensor..."));
    action->setData( 4 );
    if ( !sensor )
      action->setEnabled( false );
  }

  if ( sensor ) {
    if ( sensor->isLogging() ) {
      action = pm.addAction(i18n("St&op Logging"));
      action->setData( 6 );
    } else {
      action = pm.addAction(i18n("S&tart Logging"));
      action->setData( 5 );
    }
  }

  action = pm.exec( point );
  if ( !action )
    return;

  switch (action->data().toInt())
  {
    case 1:
      configureSettings();
      break;
    case 2: {
      KSGRD::SensorDisplay::DeleteEvent *ev = new KSGRD::SensorDisplay::DeleteEvent( this );
      kapp->postEvent(parent(), ev);
      break;
      }
    case 3:
      if ( sensor )
        mModel->removeSensor( sensor );
      break;
    case 4:
      if ( sensor )
        editSensor( sensor );
      break;
    case 5:
      if ( sensor )
        sensor->startLogging();
      break;
    case 6:
      if ( sensor )
        sensor->stopLogging();
      break;
  }
}

#include "SensorLogger.moc"
