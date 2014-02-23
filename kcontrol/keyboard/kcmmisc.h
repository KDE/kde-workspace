/*
 * keyboard.h
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __KCMMISC_H__
#define __KCMMISC_H__

#include <QString>
#include <QWidget>
#include <QButtonGroup>

class Ui_KeyboardConfigWidget;

enum TriState {
	STATE_ON = 0,  STATE_OFF = 1, STATE_UNCHANGED = 2
};


class TriStateHelper {
public:
    static void setTriState(QButtonGroup* group, TriState state);
    static TriState getTriState(const QButtonGroup* group);

	static TriState getTriState(int state) { return static_cast<TriState>(state); }
	static int getInt(TriState state) { return static_cast<int>(state); }
	static const char* getString(TriState state) {
		return state == STATE_ON ? "0" : state == STATE_OFF ? "1" : "2";
	}
};

class KCMiscKeyboardWidget : public QWidget
{
  Q_OBJECT
public:
  KCMiscKeyboardWidget(QWidget *parent);
  ~KCMiscKeyboardWidget();

  void save();
  void load();
  void defaults();

  QString quickHelp() const;

private Q_SLOTS:
  void changed();

  void delaySliderChanged (int value);
  void delaySpinboxChanged (int value);
  void rateSliderChanged (int value);
  void rateSpinboxChanged (double value);
  void keyboardRepeatStateChanged(int selection);

Q_SIGNALS:
	void changed(bool state);

private:
  void setClickVolume( int );
  void setRepeat( TriState flag, int delay, double rate);
  void setRepeatRate( int );

  int getClick();
  int getRepeatRate();

  int sliderMax;
  int clickVolume;
  enum TriState keyboardRepeat;
  enum TriState numlockState;

  Ui_KeyboardConfigWidget& ui;
};

#endif

