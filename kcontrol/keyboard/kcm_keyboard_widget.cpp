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

#include "kcm_keyboard_widget.h"

#include <kactioncollection.h>
#include <kaction.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>

#include <QtGui/QMessageBox>
#include <QtGui/QWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPixmap>
#include <QtGui/QVBoxLayout>
#include <QtGui/QX11Info>

#include "keyboard_config.h"
#include "preview/keyboardpainter.h"
#include "xkb_rules.h"
#include "flags.h"
#include "x11_helper.h"
#include "kcm_view_models.h"
#include "kcm_add_layout_dialog.h"
#include "bindings.h"

#include "kcmmisc.h"

#include "ui_kcm_add_layout_dialog.h"


static const QString GROUP_SWITCH_GROUP_NAME("grp");
static const QString LV3_SWITCH_GROUP_NAME("lv3");
//static const QString RESET_XKB_OPTIONS("-option");

static const int TAB_HARDWARE = 0;
static const int TAB_LAYOUTS = 1;
static const int TAB_ADVANCED = 2;

static const int MIN_LOOPING_COUNT = 2;


KCMKeyboardWidget::KCMKeyboardWidget(Rules* rules_, KeyboardConfig* keyboardConfig_,
		const KComponentData componentData_, const QVariantList &args, QWidget* /*parent*/):
	rules(rules_),
	componentData(componentData_),
	actionCollection(NULL),
	uiUpdating(false)
{
	flags = new Flags();
	keyboardConfig = keyboardConfig_;

	uiWidget = new Ui::TabWidget;
	uiWidget->setupUi(this);

	kcmMiscWidget = new KCMiscKeyboardWidget(uiWidget->lowerHardwareWidget);
	uiWidget->lowerHardwareWidget->layout()->addWidget( kcmMiscWidget );
	connect(kcmMiscWidget, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));

    if( rules != NULL ) {
        initializeKeyboardModelUI();
    	initializeXkbOptionsUI();
    	initializeLayoutsUI();
    }
    else {
		uiWidget->tabLayouts->setEnabled(false);
		uiWidget->tabAdvanced->setEnabled(false);
		uiWidget->keyboardModelComboBox->setEnabled(false);
    }

    handleParameters(args);
}

KCMKeyboardWidget::~KCMKeyboardWidget()
{
	delete flags;
}

void KCMKeyboardWidget::handleParameters(const QVariantList &args)
{
    // TODO: improve parameter handling
	setCurrentIndex(TAB_HARDWARE);
    foreach(const QVariant& arg, args) {
  	  if( arg.type() == QVariant::String ) {
  		  QString str = arg.toString();
  		  if( str == "--tab=layouts" ) {
  			  setCurrentIndex(TAB_LAYOUTS);
  		  }
  		  else if( str == "--tab=advanced" ) {
  	  		  setCurrentIndex(TAB_ADVANCED);
  	  	  }
  	  }
    }
}

void KCMKeyboardWidget::save()
{
	if( rules == NULL )
		return;

	if( actionCollection != NULL ) {
		actionCollection->resetLayoutShortcuts();
		actionCollection->clear();
		delete actionCollection;
	}
	actionCollection = new KeyboardLayoutActionCollection(this, true);
	actionCollection->setToggleShortcut(uiWidget->kdeKeySequence->keySequence());
	actionCollection->setLayoutShortcuts(keyboardConfig->layouts, rules);

	//TODO: skip if no change in shortcuts?
    KGlobalSettings::emitChange(KGlobalSettings::SettingsChanged, KGlobalSettings::SETTINGS_SHORTCUTS);
}

void KCMKeyboardWidget::updateUI()
{
	if( rules == NULL )
		return;

	uiWidget->layoutsTableView->setModel(uiWidget->layoutsTableView->model());
	layoutsTableModel->refresh();
	uiWidget->layoutsTableView->resizeRowsToContents();

	uiUpdating = true;
	updateHardwareUI();
	updateXkbOptionsUI();
	updateSwitcingPolicyUI();
    updateLayoutsUI();
    updateShortcutsUI();
    uiUpdating = false;
}

void KCMKeyboardWidget::uiChanged()
{
	if( rules == NULL )
		return;

	((LayoutsTableModel*)uiWidget->layoutsTableView->model())->refresh();
// this collapses the tree so use more fine-grained updates
//	((LayoutsTableModel*)uiWidget->xkbOptionsTreeView->model())->refresh();

	if( uiUpdating )
		return;

	keyboardConfig->showIndicator = uiWidget->showIndicatorChk->isChecked();
	keyboardConfig->showSingle = uiWidget->showSingleChk->isChecked();

	keyboardConfig->configureLayouts = uiWidget->layoutsGroupBox->isChecked();
	keyboardConfig->keyboardModel = uiWidget->keyboardModelComboBox->itemData(uiWidget->keyboardModelComboBox->currentIndex()).toString();

	if( uiWidget->showFlagRadioBtn->isChecked() ) {
		keyboardConfig->indicatorType =  KeyboardConfig::SHOW_FLAG;
	}
	else
	if( uiWidget->showLabelRadioBtn->isChecked() ) {
		keyboardConfig->indicatorType =  KeyboardConfig::SHOW_LABEL;
	}
	else {
//	if( uiWidget->showFlagRadioBtn->isChecked() ) {
		keyboardConfig->indicatorType =  KeyboardConfig::SHOW_LABEL_ON_FLAG;
	}

	keyboardConfig->resetOldXkbOptions = uiWidget->configureKeyboardOptionsChk->isChecked();

	if( uiWidget->switchByDesktopRadioBtn->isChecked() ) {
		keyboardConfig->switchingPolicy = KeyboardConfig::SWITCH_POLICY_DESKTOP;
	}
	else
	if( uiWidget->switchByApplicationRadioBtn->isChecked() ) {
		keyboardConfig->switchingPolicy = KeyboardConfig::SWITCH_POLICY_APPLICATION;
	}
	else
	if( uiWidget->switchByWindowRadioBtn->isChecked() ) {
		keyboardConfig->switchingPolicy = KeyboardConfig::SWITCH_POLICY_WINDOW;
	}
	else {
		keyboardConfig->switchingPolicy = KeyboardConfig::SWITCH_POLICY_GLOBAL;
	}

	updateXkbShortcutsButtons();

	updateLoopCount();
	int loop = uiWidget->layoutLoopCountSpinBox->text().isEmpty()
			? KeyboardConfig::NO_LOOPING
			: uiWidget->layoutLoopCountSpinBox->value();
	keyboardConfig->layoutLoopCount = loop;

	layoutsTableModel->refresh();

	emit changed(true);
}

void KCMKeyboardWidget::initializeKeyboardModelUI()
{
    foreach(ModelInfo* modelInfo, rules->modelInfos) {
    	QString vendor = modelInfo->vendor;
    	if( vendor.isEmpty() ) {
    		vendor = i18nc("unknown keyboard model vendor", "Unknown");
    	}
		uiWidget->keyboardModelComboBox->addItem(i18nc("vendor | keyboard model", "%1 | %2", vendor, modelInfo->description), modelInfo->name);
	}
    uiWidget->keyboardModelComboBox->model()->sort(0);
	connect(uiWidget->keyboardModelComboBox, SIGNAL(activated(int)), this, SLOT(uiChanged()));
}

void KCMKeyboardWidget::addLayout()
{
	if( keyboardConfig->layouts.count() >= X11Helper::ARTIFICIAL_GROUP_LIMIT_COUNT ) { // artificial limit now
		QMessageBox msgBox;
		msgBox.setText(i18np("Only up to %1 keyboard layout is supported", "Only up to %1 keyboard layouts are supported", X11Helper::ARTIFICIAL_GROUP_LIMIT_COUNT));
		// more information https://bugs.freedesktop.org/show_bug.cgi?id=19501
		msgBox.exec();
		return;
	}

    AddLayoutDialog dialog(rules, keyboardConfig->isFlagShown() ? flags : NULL, keyboardConfig->isLabelShown(), this);
    dialog.setModal(true);
    if( dialog.exec() == QDialog::Accepted ) {
    	keyboardConfig->layouts.append( dialog.getSelectedLayoutUnit() );
    	layoutsTableModel->refresh();
    	uiWidget->layoutsTableView->resizeRowsToContents();
    	uiChanged();
    }

    updateLoopCount();
}

static
inline int min(int x, int y) { return x < y ? x : y; }

void KCMKeyboardWidget::updateLoopCount()
{
	int maxLoop = min(X11Helper::MAX_GROUP_COUNT, keyboardConfig->layouts.count() - 1);
	uiWidget->layoutLoopCountSpinBox->setMaximum(maxLoop);

	bool layoutsConfigured = uiWidget->layoutsGroupBox->isChecked();

	if( maxLoop < MIN_LOOPING_COUNT ) {
		uiWidget->layoutLoopingCheckBox->setEnabled(false);
		uiWidget->layoutLoopingCheckBox->setChecked(false);
	}
	else if( maxLoop >= X11Helper::MAX_GROUP_COUNT ) {
		uiWidget->layoutLoopingCheckBox->setEnabled(false);
		uiWidget->layoutLoopingCheckBox->setChecked(true);
	}
	else {
		uiWidget->layoutLoopingCheckBox->setEnabled(layoutsConfigured);
	}

	uiWidget->layoutLoopingGroupBox->setEnabled(
			layoutsConfigured && uiWidget->layoutLoopingCheckBox->isChecked());

	if( uiWidget->layoutLoopingCheckBox->isChecked() ) {
		if( uiWidget->layoutLoopCountSpinBox->text().isEmpty() ) {
			uiWidget->layoutLoopCountSpinBox->setValue(maxLoop);
//			keyboardConfig->layoutLoopCount = maxLoop;
		}
	}
	else {
		uiWidget->layoutLoopCountSpinBox->clear();
//		keyboardConfig->layoutLoopCount = KeyboardConfig::NO_LOOPING;
	}
}

void KCMKeyboardWidget::initializeLayoutsUI()
{
	layoutsTableModel = new LayoutsTableModel(rules, flags, keyboardConfig, uiWidget->layoutsTableView);
	uiWidget->layoutsTableView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
	uiWidget->layoutsTableView->setModel(layoutsTableModel);
	uiWidget->layoutsTableView->setIconSize( flags->getTransparentPixmap().size() );

	//TODO: do we need to delete this delegate or parent will take care of it?
	VariantComboDelegate* variantDelegate = new VariantComboDelegate(keyboardConfig, rules, uiWidget->layoutsTableView);
	uiWidget->layoutsTableView->setItemDelegateForColumn(LayoutsTableModel::VARIANT_COLUMN, variantDelegate);

	LabelEditDelegate* labelDelegate = new LabelEditDelegate(keyboardConfig, uiWidget->layoutsTableView);
	uiWidget->layoutsTableView->setItemDelegateForColumn(LayoutsTableModel::DISPLAY_NAME_COLUMN, labelDelegate);

	KKeySequenceWidgetDelegate* shortcutDelegate = new KKeySequenceWidgetDelegate(keyboardConfig, uiWidget->layoutsTableView);
	uiWidget->layoutsTableView->setItemDelegateForColumn(LayoutsTableModel::SHORTCUT_COLUMN, shortcutDelegate);

	//TODO: is it ok to hardcode sizes? any better approach?
	uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::MAP_COLUMN, 70);
	uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::LAYOUT_COLUMN, 200);
	uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::VARIANT_COLUMN, 200);
	uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::DISPLAY_NAME_COLUMN, 50);
	uiWidget->layoutsTableView->setColumnWidth(LayoutsTableModel::SHORTCUT_COLUMN, 130);

	connect(layoutsTableModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(uiChanged()));

	uiWidget->layoutLoopCountSpinBox->setMinimum(MIN_LOOPING_COUNT);

#ifdef DRAG_ENABLED
	uiWidget->layoutsTableView->setDragEnabled(true);
	uiWidget->layoutsTableView->setAcceptDrops(true);
#endif

	uiWidget->moveUpBtn->setIcon(KIcon("arrow-up"));
    uiWidget->moveDownBtn->setIcon(KIcon("arrow-down"));
	uiWidget->addLayoutBtn->setIcon(KIcon("list-add"));
    uiWidget->removeLayoutBtn->setIcon(KIcon("list-remove"));

    KIcon clearIcon = qApp->isLeftToRight() ? KIcon("edit-clear-locationbar-rtl") : KIcon("edit-clear-locationbar-ltr");
	uiWidget->xkbGrpClearBtn->setIcon(clearIcon);
	uiWidget->xkb3rdLevelClearBtn->setIcon(clearIcon);

	KIcon configIcon = KIcon("configure");
	uiWidget->xkbGrpShortcutBtn->setIcon(configIcon);
	uiWidget->xkb3rdLevelShortcutBtn->setIcon(configIcon);

    uiWidget->kdeKeySequence->setModifierlessAllowed(false);

	connect(uiWidget->addLayoutBtn, SIGNAL(clicked(bool)), this, SLOT(addLayout()));
	connect(uiWidget->removeLayoutBtn, SIGNAL(clicked(bool)), this, SLOT(removeLayout()));
//	connect(uiWidget->layoutsTable, SIGNAL(itemSelectionChanged()), this, SLOT(layoutSelectionChanged()));
	connect(uiWidget->layoutsTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(layoutSelectionChanged()));

//	connect(uiWidget->moveUpBtn, SIGNAL(triggered(QAction*)), this, SLOT(moveUp()));
//	connect(uiWidget->moveDownBtn, SIGNAL(triggered(QAction*)), this, SLOT(moveDown()));
	connect(uiWidget->moveUpBtn, SIGNAL(clicked(bool)), this, SLOT(moveUp()));
	connect(uiWidget->moveDownBtn, SIGNAL(clicked(bool)), this, SLOT(moveDown()));

    connect(uiWidget->previewbutton,SIGNAL(clicked(bool)),this,SLOT(previewLayout()));

	connect(uiWidget->xkbGrpClearBtn, SIGNAL(clicked(bool)), this, SLOT(clearGroupShortcuts()));
	connect(uiWidget->xkb3rdLevelClearBtn, SIGNAL(clicked(bool)), this, SLOT(clear3rdLevelShortcuts()));

//	connect(uiWidget->xkbGrpClearBtn, SIGNAL(triggered(QAction*)), this, SLOT(uiChanged()));
//	connect(uiWidget->xkb3rdLevelClearBtn, SIGNAL(triggered(QAction*)), this, SLOT(uiChanged()));
	connect(uiWidget->kdeKeySequence, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(uiChanged()));
	connect(uiWidget->switchingPolicyButtonGroup, SIGNAL(clicked(int)), this, SLOT(uiChanged()));

	connect(uiWidget->xkbGrpShortcutBtn, SIGNAL(clicked(bool)), this, SLOT(scrollToGroupShortcut()));
	connect(uiWidget->xkb3rdLevelShortcutBtn, SIGNAL(clicked(bool)), this, SLOT(scrollTo3rdLevelShortcut()));

	//	connect(uiWidget->configureLayoutsChk, SIGNAL(toggled(bool)), uiWidget->layoutsGroupBox, SLOT(setEnabled(bool)));
	connect(uiWidget->layoutsGroupBox, SIGNAL(toggled(bool)), this, SLOT(configureLayoutsChanged()));

	connect(uiWidget->showIndicatorChk, SIGNAL(clicked(bool)), this, SLOT(uiChanged()));
	connect(uiWidget->showIndicatorChk, SIGNAL(toggled(bool)), uiWidget->showSingleChk, SLOT(setEnabled(bool)));
	connect(uiWidget->showFlagRadioBtn, SIGNAL(clicked(bool)), this, SLOT(uiChanged()));
	connect(uiWidget->showLabelRadioBtn, SIGNAL(clicked(bool)), this, SLOT(uiChanged()));
	connect(uiWidget->showLabelOnFlagRadioBtn, SIGNAL(clicked(bool)), this, SLOT(uiChanged()));
	connect(uiWidget->showSingleChk, SIGNAL(toggled(bool)), this, SLOT(uiChanged()));

	connect(uiWidget->layoutLoopingCheckBox, SIGNAL(clicked(bool)), this, SLOT(uiChanged()));
	connect(uiWidget->layoutLoopCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(uiChanged()));
}

void KCMKeyboardWidget::previewLayout(){
    QMessageBox q;
    QModelIndex index = uiWidget->layoutsTableView->currentIndex() ;
    QModelIndex idcountry = index.sibling(index.row(),0) ;
    QString country=uiWidget->layoutsTableView->model()->data(idcountry).toString();
    QModelIndex idvariant = index.sibling(index.row(),2) ;
    QString variant=uiWidget->layoutsTableView->model()->data(idvariant).toString();
    if(index.row()==-1 || index.column()==-1){
        q.setText(i18n("No layout selected "));
        q.exec();
    }
    else{
        KeyboardPainter* layoutPreview = new KeyboardPainter();
        const LayoutInfo* layoutInfo = rules->getLayoutInfo(country);
        foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
            if(variant==variantInfo->description){
                variant=variantInfo->name;
                break;
            }
        }
        layoutPreview->generateKeyboardLayout(country,variant);
        layoutPreview->exec();
        layoutPreview->setModal(true);
    }
}

void KCMKeyboardWidget::configureLayoutsChanged()
{
	if( uiWidget->layoutsGroupBox->isChecked()	&& keyboardConfig->layouts.isEmpty() ) {
		populateWithCurrentLayouts();
	}
	uiChanged();
}

static QPair<int, int> getSelectedRowRange(const QModelIndexList& selected)
{
	if( selected.isEmpty() ) {
		return QPair<int, int>(-1, -1);
	}

	QList<int> rows;
	foreach(const QModelIndex& index, selected) {
		rows << index.row();
	}
	qSort(rows);
	return QPair<int, int>(rows[0], rows[rows.size()-1]);
}

void KCMKeyboardWidget::layoutSelectionChanged()
{
	QModelIndexList selected = uiWidget->layoutsTableView->selectionModel()->selectedIndexes();
	uiWidget->removeLayoutBtn->setEnabled( ! selected.isEmpty() );
	QPair<int, int> rowsRange( getSelectedRowRange(selected) );
	uiWidget->moveUpBtn->setEnabled( ! selected.isEmpty() && rowsRange.first > 0);
    uiWidget->previewbutton->setEnabled(! selected.isEmpty());
	uiWidget->moveDownBtn->setEnabled( ! selected.isEmpty() && rowsRange.second < keyboardConfig->layouts.size()-1 );
}

void KCMKeyboardWidget::removeLayout()
{
	if( ! uiWidget->layoutsTableView->selectionModel()->hasSelection() )
		return;

	QModelIndexList selected = uiWidget->layoutsTableView->selectionModel()->selectedIndexes();
	QPair<int, int> rowsRange( getSelectedRowRange(selected) );
	foreach(const QModelIndex& idx, selected) {
		if( idx.column() == 0 ) {
			keyboardConfig->layouts.removeAt(rowsRange.first);
		}
	}
	layoutsTableModel->refresh();
	uiChanged();

	if( keyboardConfig->layouts.size() > 0 ) {
		int rowToSelect = rowsRange.first;
		if( rowToSelect >= keyboardConfig->layouts.size() ) {
			rowToSelect--;
		}

        QModelIndex topLeft = layoutsTableModel->index(rowToSelect, 0, QModelIndex());
        QModelIndex bottomRight = layoutsTableModel->index(rowToSelect, layoutsTableModel->columnCount(topLeft)-1, QModelIndex());
        QItemSelection selection(topLeft, bottomRight);
        uiWidget->layoutsTableView->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
        uiWidget->layoutsTableView->setFocus();
	}

	layoutSelectionChanged();

	updateLoopCount();
}

void KCMKeyboardWidget::moveUp()
{
	moveSelectedLayouts(-1);
}

void KCMKeyboardWidget::moveDown()
{
	moveSelectedLayouts(1);
}

void KCMKeyboardWidget::moveSelectedLayouts(int shift)
{
    QItemSelectionModel* selectionModel = uiWidget->layoutsTableView->selectionModel();
    if( selectionModel == NULL || !selectionModel->hasSelection() )
        return;

    QModelIndexList selected = selectionModel->selectedRows();
    if( selected.count() < 1 )
        return;

    int newFirstRow = selected[0].row() + shift;
    int newLastRow = selected[ selected.size()-1 ].row() + shift;

    if( newFirstRow >= 0 && newLastRow <= keyboardConfig->layouts.size() - 1 ) {
        QList<int> selectionRows;
    	foreach(const QModelIndex& index, selected) {
    		int newRowIndex = index.row() + shift;
    		keyboardConfig->layouts.move(index.row(), newRowIndex);
            selectionRows << newRowIndex;
    	}
    	uiChanged();

    	QItemSelection selection;
    	foreach(int row, selectionRows) {
            QModelIndex topLeft = layoutsTableModel->index(row, 0, QModelIndex());
            QModelIndex bottomRight = layoutsTableModel->index(row, layoutsTableModel->columnCount(topLeft)-1, QModelIndex());
            selection << QItemSelectionRange(topLeft, bottomRight);
    	}
        uiWidget->layoutsTableView->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent);
        uiWidget->layoutsTableView->setFocus();
    }
}

void KCMKeyboardWidget::scrollToGroupShortcut()
{
    this->setCurrentIndex(TAB_ADVANCED);
    if( ! uiWidget->configureKeyboardOptionsChk->isChecked() ) {
    	uiWidget->configureKeyboardOptionsChk->setChecked(true);
    }
	((XkbOptionsTreeModel*)uiWidget->xkbOptionsTreeView->model())->gotoGroup(GROUP_SWITCH_GROUP_NAME, uiWidget->xkbOptionsTreeView);
}

void KCMKeyboardWidget::scrollTo3rdLevelShortcut()
{
	this->setCurrentIndex(TAB_ADVANCED);
    if( ! uiWidget->configureKeyboardOptionsChk->isChecked() ) {
    	uiWidget->configureKeyboardOptionsChk->setChecked(true);
    }
	((XkbOptionsTreeModel*)uiWidget->xkbOptionsTreeView->model())->gotoGroup(LV3_SWITCH_GROUP_NAME, uiWidget->xkbOptionsTreeView);
}

void KCMKeyboardWidget::clearGroupShortcuts()
{
	clearXkbGroup(GROUP_SWITCH_GROUP_NAME);
}

void KCMKeyboardWidget::clear3rdLevelShortcuts()
{
	clearXkbGroup(LV3_SWITCH_GROUP_NAME);
}

void KCMKeyboardWidget::clearXkbGroup(const QString& groupName)
{
	for(int ii=keyboardConfig->xkbOptions.count()-1; ii>=0; ii--) {
		if( keyboardConfig->xkbOptions[ii].startsWith(groupName + Rules::XKB_OPTION_GROUP_SEPARATOR) ) {
			keyboardConfig->xkbOptions.removeAt(ii);
		}
	}
	((XkbOptionsTreeModel*)uiWidget->xkbOptionsTreeView->model())->reset();
	uiWidget->xkbOptionsTreeView->update();
	updateXkbShortcutsButtons();
    emit changed(true);
}

static
bool xkbOptionGroupLessThan(const OptionGroupInfo* og1, const OptionGroupInfo* og2)
{
     return og1->description.toLower() < og2->description.toLower();
}
static
bool xkbOptionLessThan(const OptionInfo* o1, const OptionInfo* o2)
{
     return o1->description.toLower() < o2->description.toLower();
}

void KCMKeyboardWidget::initializeXkbOptionsUI()
{
	qSort(rules->optionGroupInfos.begin(), rules->optionGroupInfos.end(), xkbOptionGroupLessThan);
	foreach(OptionGroupInfo* optionGroupInfo, rules->optionGroupInfos) {
		qSort(optionGroupInfo->optionInfos.begin(), optionGroupInfo->optionInfos.end(), xkbOptionLessThan);
	}

	XkbOptionsTreeModel* model = new XkbOptionsTreeModel(rules, keyboardConfig, uiWidget->xkbOptionsTreeView);
	uiWidget->xkbOptionsTreeView->setModel(model);
	connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(uiChanged()));

	connect(uiWidget->configureKeyboardOptionsChk, SIGNAL(toggled(bool)), this, SLOT(configureXkbOptionsChanged()));
	//	connect(uiWidget->configureKeyboardOptionsChk, SIGNAL(toggled(bool)), this, SLOT(uiChanged()));
	connect(uiWidget->configureKeyboardOptionsChk, SIGNAL(toggled(bool)), uiWidget->xkbOptionsTreeView, SLOT(setEnabled(bool)));
}

void KCMKeyboardWidget::configureXkbOptionsChanged()
{
	if( uiWidget->configureKeyboardOptionsChk->isChecked() && keyboardConfig->xkbOptions.isEmpty() ) {
		populateWithCurrentXkbOptions();
	}
	((LayoutsTableModel*)uiWidget->xkbOptionsTreeView->model())->refresh();
	uiChanged();
}

void KCMKeyboardWidget::updateSwitcingPolicyUI()
{
    switch (keyboardConfig->switchingPolicy){
        case KeyboardConfig::SWITCH_POLICY_DESKTOP:
            uiWidget->switchByDesktopRadioBtn->setChecked(true);
            break;
        case KeyboardConfig::SWITCH_POLICY_APPLICATION:
            uiWidget->switchByApplicationRadioBtn->setChecked(true);
            break;
        case KeyboardConfig::SWITCH_POLICY_WINDOW:
            uiWidget->switchByWindowRadioBtn->setChecked(true);
            break;
        default:
        case KeyboardConfig::SWITCH_POLICY_GLOBAL:
            uiWidget->switchByGlobalRadioBtn->setChecked(true);
    }
}

void KCMKeyboardWidget::updateXkbShortcutButton(const QString& groupName, QPushButton* button)
{
	QStringList grpOptions;
	if( keyboardConfig->resetOldXkbOptions ) {
		QRegExp regexp = QRegExp("^" + groupName + Rules::XKB_OPTION_GROUP_SEPARATOR);
		grpOptions = keyboardConfig->xkbOptions.filter(regexp);
	}
	switch( grpOptions.size() ) {
	case 0:
		button->setText(i18nc("no shortcuts defined", "None"));
	break;
	case 1: {
		const QString& option = grpOptions.first();
		const OptionGroupInfo* optionGroupInfo = rules->getOptionGroupInfo(groupName);
		const OptionInfo* optionInfo = optionGroupInfo->getOptionInfo(option);
		if( optionInfo == NULL || optionInfo->description == NULL ) {
			kError() << "Could not find option info for " << option;
			button->setText(grpOptions.first());
		}
		else {
			button->setText(optionInfo->description);
		}
	}
	break;
	default:
		button->setText(i18np("%1 shortcut", "%1 shortcuts", grpOptions.size()));
	}
}

void KCMKeyboardWidget::updateXkbShortcutsButtons()
{
	updateXkbShortcutButton(GROUP_SWITCH_GROUP_NAME, uiWidget->xkbGrpShortcutBtn);
	updateXkbShortcutButton(LV3_SWITCH_GROUP_NAME, uiWidget->xkb3rdLevelShortcutBtn);
}

void KCMKeyboardWidget::updateShortcutsUI()
{
	updateXkbShortcutsButtons();

	delete actionCollection;
	actionCollection = new KeyboardLayoutActionCollection(this, true);
	KAction* toggleAction = actionCollection->getToggeAction();
    uiWidget->kdeKeySequence->setKeySequence(toggleAction->globalShortcut().primary());
    actionCollection->loadLayoutShortcuts(keyboardConfig->layouts, rules);
	layoutsTableModel->refresh();
}

void KCMKeyboardWidget::updateXkbOptionsUI()
{
    uiWidget->configureKeyboardOptionsChk->setChecked(keyboardConfig->resetOldXkbOptions);
}

void KCMKeyboardWidget::updateLayoutsUI() {
	uiWidget->layoutsGroupBox->setChecked(keyboardConfig->configureLayouts);
	uiWidget->showIndicatorChk->setChecked(keyboardConfig->showIndicator);
	uiWidget->showSingleChk->setChecked(keyboardConfig->showSingle);
	uiWidget->showFlagRadioBtn->setChecked(keyboardConfig->indicatorType == KeyboardConfig::SHOW_FLAG);
	uiWidget->showLabelRadioBtn->setChecked(keyboardConfig->indicatorType == KeyboardConfig::SHOW_LABEL);
	uiWidget->showLabelOnFlagRadioBtn->setChecked(keyboardConfig->indicatorType == KeyboardConfig::SHOW_LABEL_ON_FLAG);

	bool loopingOn = keyboardConfig->configureLayouts && keyboardConfig->layoutLoopCount
			!= KeyboardConfig::NO_LOOPING;
	uiWidget->layoutLoopingCheckBox->setChecked(loopingOn);
	uiWidget->layoutLoopingGroupBox->setEnabled(loopingOn);
	if( loopingOn ) {
		uiWidget->layoutLoopCountSpinBox->setValue(keyboardConfig->layoutLoopCount);
	}
	else {
		uiWidget->layoutLoopCountSpinBox->clear();
	}
}

void KCMKeyboardWidget::updateHardwareUI()
{
	int idx = uiWidget->keyboardModelComboBox->findData(keyboardConfig->keyboardModel);
	if( idx != -1 ) {
		uiWidget->keyboardModelComboBox->setCurrentIndex(idx);
	}
}

void KCMKeyboardWidget::populateWithCurrentLayouts()
{
	QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
	foreach(LayoutUnit layoutUnit, layouts) {
		keyboardConfig->layouts.append(layoutUnit);
	}
}

void KCMKeyboardWidget::populateWithCurrentXkbOptions()
{
	XkbConfig xkbConfig;
	if( X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::ALL) ) {
		foreach(QString xkbOption, xkbConfig.options) {
			keyboardConfig->xkbOptions.append(xkbOption);
		}
	}
}
