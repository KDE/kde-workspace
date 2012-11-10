/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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


#ifndef KCM_KEYBOARD_WIDGET_H_
#define KCM_KEYBOARD_WIDGET_H_

#include "ui_kcm_keyboard.h"

#include <kcomponentdata.h>
#include <QtGui/QTabWidget>

class QWidget;
class KeyboardConfig;
class Rules;
class Flags;
class KComponentData;
class QString;
class QPushButton;
class LayoutsTableModel;
class KCMiscKeyboardWidget;
class KeyboardLayoutActionCollection;

class KCMKeyboardWidget: public QTabWidget
{
	Q_OBJECT

public:
	KCMKeyboardWidget(Rules* rules, KeyboardConfig* keyboardConfig,
			const KComponentData componentData, const QVariantList &args, QWidget* parent=0);
	virtual ~KCMKeyboardWidget();

	void updateUI();
	void save();

	//temp hack
	KCMiscKeyboardWidget* getKcmMiscWidget() const { return kcmMiscWidget; }

Q_SIGNALS:
	void changed(bool state);

private Q_SLOTS:
	void addLayout();
	void removeLayout();
	void layoutSelectionChanged();
	void uiChanged();
    void scrollToGroupShortcut();
    void scrollTo3rdLevelShortcut();
    void clearGroupShortcuts();
    void clear3rdLevelShortcuts();
    void updateXkbShortcutsButtons();
    void moveUp();
    void moveDown();
    void configureLayoutsChanged();
    void configureXkbOptionsChanged();
    void previewLayout();

private:
    Rules *rules;
    Flags *flags;
    Ui::TabWidget *uiWidget;
    KeyboardConfig *keyboardConfig;
	const KComponentData componentData;
	KeyboardLayoutActionCollection* actionCollection;
	LayoutsTableModel* layoutsTableModel;
	KCMiscKeyboardWidget* kcmMiscWidget;
	bool uiUpdating;

	void initializeLayoutsUI();
	void initializeXkbOptionsUI();
    void initializeKeyboardModelUI();
    void updateHardwareUI();
    void updateLayoutsUI();
    void updateShortcutsUI();
    void updateXkbOptionsUI();
    void updateSwitcingPolicyUI();
    void updateXkbShortcutButton(const QString& groupName, QPushButton* button);
    void clearXkbGroup(const QString& groupName);
    void moveSelectedLayouts(int shift);
    void populateWithCurrentLayouts();
    void populateWithCurrentXkbOptions();
    void updateLoopCount();
    void handleParameters(const QVariantList &args);
};


#endif /* KCM_KEYBOARD_WIDGET_H_ */
