/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller
 *   kollix@aon.at
 *   This file is part of the KDE Control Center Module for Joysticks
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

#include "caldialog.h"
#include "joydevice.h"

#include <QLabel>
#include <QTimer>
#include <QApplication>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kvbox.h>
//--------------------------------------------------------------

CalDialog::CalDialog(QWidget *parent, JoyDevice *joy)
  : KDialog( parent ),
    joydev(joy)
{
  setObjectName( "calibrateDialog" );
  setModal( true );
  setCaption( i18n("Calibration") );
  setButtons( Cancel | User1 );
  setDefaultButton( User1 );
  setButtonGuiItem( User1, KGuiItem( i18n("Next") ) );

  KVBox *main = new KVBox( this );
  setMainWidget( main );

  text = new QLabel(main);
  text->setMinimumHeight(200);
  valueLbl = new QLabel(main);
  connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
}

//--------------------------------------------------------------

void CalDialog::calibrate()
{
  text->setText(i18n("Please wait a moment to calculate the precision"));
  setResult(-1);
  show();

  // calibrate precision (which min,max delivers the joystick in its center position)
  // get values through the normal idle procedure
  QTimer ti;
  ti.setSingleShot(true); // single shot
  ti.start(2000);         // in 2 seconds

  // normally I'd like to hide the 'Next' button in this step,
  // but it does not work - which means: in the steps after the first,
  // the 'Next' button does not have the focus (to be the default button)

  do
  {
    qApp->processEvents(QEventLoop::AllEvents, 2000);
  }
  while ( ti.isActive() && (result() != QDialog::Rejected) );

  if ( result() == QDialog::Rejected ) return;  // user cancelled the dialog

  joydev->calcPrecision();

  int i, lastVal;
  int min[2], center[2], max[2];
  QString hint;

  for (i = 0; i < joydev->numAxes(); i++)
  {
    if ( i == 0 )
      hint = i18n("(usually X)");
    else if ( i == 1 )
      hint = i18n("(usually Y)");
    else
      hint = "";

    // minimum position
    text->setText(i18n("<qt>Calibration is about to check the value range your device delivers.<br /><br />"
                       "Please move <b>axis %1 %2</b> on your device to the <b>minimum</b> position.<br /><br />"
                       "Press any button on the device or click on the 'Next' button "
                       "to continue with the next step.</qt>", i+1, hint));
    waitButton(i, true, lastVal);

    if ( result() == QDialog::Rejected ) return;  // user cancelled the dialog

    joydev->resetMinMax(i, lastVal);
    if ( result() != -2 ) waitButton(i, false, lastVal);

    if ( result() == QDialog::Rejected ) return;  // user cancelled the dialog

    min[0] = joydev->axisMin(i);
    min[1] = joydev->axisMax(i);

    // center position
    text->setText(i18n("<qt>Calibration is about to check the value range your device delivers.<br /><br />"
                       "Please move <b>axis %1 %2</b> on your device to the <b>center</b> position.<br /><br />"
                       "Press any button on the device or click on the 'Next' button "
                       "to continue with the next step.</qt>", i+1, hint));
    waitButton(i, true, lastVal);

    if ( result() == QDialog::Rejected ) return;  // user cancelled the dialog

    joydev->resetMinMax(i, lastVal);
    if ( result() != -2 ) waitButton(i, false, lastVal);

    if ( result() == QDialog::Rejected ) return;  // user canceled the dialog

    center[0] = joydev->axisMin(i);
    center[1] = joydev->axisMax(i);

    // maximum position
    text->setText(i18n("<qt>Calibration is about to check the value range your device delivers.<br /><br />"
                       "Please move <b>axis %1 %2</b> on your device to the <b>maximum</b> position.<br /><br />"
                       "Press any button on the device or click on the 'Next' button "
                       "to continue with the next step.</qt>", i+1, hint));
    waitButton(i, true, lastVal);

    if ( result() == QDialog::Rejected ) return;  // user cancelled the dialog

    joydev->resetMinMax(i, lastVal);
    if ( result() != -2 ) waitButton(i, false, lastVal);

    if ( result() == QDialog::Rejected ) return;  // user canceled the dialog

    max[0] = joydev->axisMin(i);
    max[1] = joydev->axisMax(i);

    joydev->calcCorrection(i, min, center, max);
  }

  JoyDevice::ErrorCode ret = joydev->applyCalibration();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
    reject();
  }

  KMessageBox::information(this, i18n("You have successfully calibrated your device"), i18n("Calibration Success"));
  accept();
}

//--------------------------------------------------------------

void CalDialog::waitButton(int axis, bool press, int &lastVal)
{
  JoyDevice::EventType type;
  int number, value;
  bool button = false;
  lastVal = 0;

  setResult(-1);
  // loop until the user presses a button on the device or on the dialog
  do
  {
    qApp->processEvents(QEventLoop::AllEvents, 100);

    if ( joydev->getEvent(type, number, value) )
    {
      button = ( (type == JoyDevice::BUTTON) && (press ? (value == 1) : (value == 0)) );

      if ( (type == JoyDevice::AXIS) && (number == axis) )
        valueLbl->setText(i18n("Value Axis %1: %2", axis+1, lastVal = value));
    }
  }
  while ( !button && (result() == -1) );
}

//--------------------------------------------------------------
// Next button

void CalDialog::slotUser1()
{
  setResult(-2);
}

//--------------------------------------------------------------

#include "caldialog.moc"

//--------------------------------------------------------------
