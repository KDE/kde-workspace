/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#include <QLabel>
#include <QComboBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QPushButton>
#include <Q3ListView>
#include <Q3ListViewItem>
#include <Q3CheckListItem>
#include <QHeaderView>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QtGui>

#include <kshortcutsdialog.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <ktoolinvocation.h>

#include "extension.h"
#include "kxkbconfig.h"
#include "rules.h"
#include "pixmap.h"
#include "kcmmisc.h"
#include "ui_kcmlayoutwidget.h"

#include "kcmlayout.h"
#include <KPluginFactory>
#include <KPluginLoader>
#include "kcmlayout.moc"


static inline QString i18n(const QString& str) { return i18n( str.toLatin1().constData() ); }

enum {
 LAYOUT_COLUMN_FLAG = 0,
 LAYOUT_COLUMN_NAME = 1,
 LAYOUT_COLUMN_MAP = 2,
 LAYOUT_COLUMN_VARIANT = 3,
 LAYOUT_COLUMN_DISPLAY_NAME = 4,
 SRC_LAYOUT_COLUMN_COUNT = 3,
 DST_LAYOUT_COLUMN_COUNT = 5
};

static const QString DEFAULT_VARIANT_NAME("<default>");


class OptionListItem : public Q3CheckListItem
{
	public:

		OptionListItem(  OptionListItem *parent, const QString &text, Type tt,
						 const QString &optionName );
		OptionListItem(  Q3ListView *parent, const QString &text, Type tt,
						 const QString &optionName );
		~OptionListItem() {}

		QString optionName() const { return m_OptionName; }

		OptionListItem *findChildItem(  const QString& text );

	protected:
		QString m_OptionName;
};


static QString lookupLocalized(const QHash<QString, QString> &dict, const QString& text)
{
  QHashIterator<QString, QString> it(dict);
  while (it.hasNext())
  {
	  it.next();
		if ( i18n( it.value() ) == text )
        return it.key();
    }

  return QString();
}

static Q3ListViewItem* copyLVI(const Q3ListViewItem* src, Q3ListView* parent)
{
    Q3ListViewItem* ret = new Q3ListViewItem(parent);
	for(int i = 0; i < SRC_LAYOUT_COLUMN_COUNT; i++)
    {
        ret->setText(i, src->text(i));
        if ( src->pixmap(i) )
            ret->setPixmap(i, *src->pixmap(i));
    }

    return ret;
}

K_PLUGIN_FACTORY_DECLARATION(KeyboardConfigFactory)

LayoutConfig::LayoutConfig(QWidget *parent, const QVariantList &)
  : KCModule(KeyboardConfigFactory::componentData(), parent),
    m_rules(NULL)
{
 // QVBoxLayout *main = new QVBoxLayout(this, 0, KDialog::spacingHint());

  widget = new Ui_LayoutConfigWidget();
  widget->setupUi(this);
//  main->addWidget(widget);

  connect( widget->chkEnable, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkIndicatorOnly, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkShowSingle, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->chkShowFlag, SIGNAL( toggled( bool )), this, SLOT(changed()));
  connect( widget->comboModel, SIGNAL(activated(int)), this, SLOT(changed()));

  connect( widget->listLayoutsSrc, SIGNAL(doubleClicked(Q3ListViewItem*,const QPoint&, int)),
									this, SLOT(add()));
  connect( widget->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
  connect( widget->btnRemove, SIGNAL(clicked()), this, SLOT(remove()));

//  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(changed()));
  connect( widget->comboVariant, SIGNAL(activated(int)), this, SLOT(variantChanged()));
  connect( widget->listLayoutsDst, SIGNAL(selectionChanged(Q3ListViewItem *)),
		this, SLOT(layoutSelChanged(Q3ListViewItem *)));

//  connect( widget->editDisplayName, SIGNAL(textChanged(const QString&)), this, SLOT(displayNameChanged(const QString&)));

  widget->btnUp->setIconSet(KIcon("arrow-up"));
//  connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->btnUp, SIGNAL(clicked()), this, SLOT(moveUp()));
  widget->btnDown->setIconSet(KIcon("arrow-down"));
//  connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(changed()));
  connect( widget->btnDown, SIGNAL(clicked()), this, SLOT(moveDown()));

  connect( widget->grpSwitching, SIGNAL( clicked( int ) ), SLOT(changed()));

  connect( widget->chkEnableSticky, SIGNAL(toggled(bool)), this, SLOT(changed()));
  connect( widget->spinStickyDepth, SIGNAL(valueChanged(int)), this, SLOT(changed()));

  widget->listLayoutsSrc->setColumnText(LAYOUT_COLUMN_FLAG, "");
  widget->listLayoutsDst->setColumnText(LAYOUT_COLUMN_FLAG, "");
//  widget->listLayoutsDst->setColumnText(LAYOUT_COLUMN_DISPLAY_NAME, "");

  widget->listLayoutsSrc->setColumnWidth(LAYOUT_COLUMN_FLAG, 28);
  widget->listLayoutsDst->setColumnWidth(LAYOUT_COLUMN_FLAG, 28);

  widget->listLayoutsDst->header()->setResizeEnabled(false, LAYOUT_COLUMN_DISPLAY_NAME);
//  widget->listLayoutsDst->setColumnWidth(LAYOUT_COLUMN_DISPLAY_NAME, 0);

  widget->listLayoutsDst->setSorting(-1);
#if 0
  widget->listLayoutsDst->setResizeMode(QListView::LastColumn);
  widget->listLayoutsSrc->setResizeMode(QListView::LastColumn);
#endif
  widget->listLayoutsDst->setResizeMode(Q3ListView::LastColumn);

  //Read rules - we _must_ read _before_ creating xkb-options comboboxes
  loadRules();

  makeOptionsTab();

  load();
}


LayoutConfig::~LayoutConfig()
{
	delete m_rules;
}


void LayoutConfig::load()
{
	m_kxkbConfig.load(KxkbConfig::LOAD_ALL);

	initUI();
}

void LayoutConfig::initUI()
{
	QString modelName = m_rules->models()[m_kxkbConfig.m_model];
	if( modelName.isEmpty() )
		modelName = DEFAULT_MODEL;

	widget->comboModel->setCurrentText(i18n(modelName));

	QList<LayoutUnit> otherLayouts = m_kxkbConfig.m_layouts;
	widget->listLayoutsDst->clear();
// to optimize we should have gone from it.end to it.begin
	for (QListIterator<LayoutUnit> it(otherLayouts); it.hasNext();  ) {
		LayoutUnit layoutUnit = it.next();

		Q3ListViewItemIterator src_it( widget->listLayoutsSrc );

		for ( ; src_it.current(); ++src_it) {
			Q3ListViewItem* srcItem = src_it.current();

			if ( layoutUnit.layout == srcItem->text(LAYOUT_COLUMN_MAP) ) {	// check if current config knows about this layout
				Q3ListViewItem* newItem = copyLVI(srcItem, widget->listLayoutsDst);

				newItem->setText(LAYOUT_COLUMN_VARIANT, layoutUnit.variant);
				newItem->setText(LAYOUT_COLUMN_DISPLAY_NAME, layoutUnit.displayName);
				widget->listLayoutsDst->insertItem(newItem);
				newItem->moveItem(widget->listLayoutsDst->lastItem());

				break;
			}
		}
	}

	// display KXKB switching options
	widget->chkShowSingle->setChecked(m_kxkbConfig.m_showSingle);
	widget->chkShowFlag->setChecked(m_kxkbConfig.m_showFlag);

	widget->chkEnableOptions->setChecked( m_kxkbConfig.m_enableXkbOptions );
	widget->checkResetOld->setChecked(m_kxkbConfig.m_resetOldOptions);

	switch( m_kxkbConfig.m_switchingPolicy ) {
		default:
		case SWITCH_POLICY_GLOBAL:
			widget->grpSwitching->setButton(0);
			break;
		case SWITCH_POLICY_WIN_CLASS:
			widget->grpSwitching->setButton(1);
			break;
		case SWITCH_POLICY_WINDOW:
			widget->grpSwitching->setButton(2);
			break;
	}

	widget->chkEnableSticky->setChecked(m_kxkbConfig.m_stickySwitching);
	widget->spinStickyDepth->setEnabled(m_kxkbConfig.m_stickySwitching);
	widget->spinStickyDepth->setValue( m_kxkbConfig.m_stickySwitchingDepth);

	updateStickyLimit();

	widget->chkEnable->setChecked( m_kxkbConfig.m_useKxkb );
	widget->grpLayouts->setEnabled( m_kxkbConfig.m_useKxkb );
	widget->grpOptions->setEnabled( m_kxkbConfig.m_useKxkb );

	// display xkb options
	QStringList options = m_kxkbConfig.m_options.split(',');
	for (QListIterator<QString> it(options); it.hasNext(); )
	{
		QString optionName = it.next();
		if( optionName.trimmed().isEmpty() ) {
			kWarning() << "skipping empty option name" ;
  			continue;
		}

		const XkbOption& option = m_rules->options()[optionName];
		OptionListItem *item = m_optionGroups[ option.group->name ];

		if (item != NULL) {
			OptionListItem *child = item->findChildItem( option.name );

			if ( child )
				child->setState( Q3CheckListItem::On );
			else
				kDebug() << "load: Unknown option: " << optionName;
		}
		else {
			kDebug() << "load: Unknown option group: " << option.group->name << " of " << optionName;
		}
	}

	updateLayoutCommand();
	updateOptionsCommand();
	emit KCModule::changed( false );
}


void LayoutConfig::save()
{
	QString model = lookupLocalized(m_rules->models(), widget->comboModel->currentText());
	m_kxkbConfig.m_model = model;

	m_kxkbConfig.m_enableXkbOptions = widget->chkEnableOptions->isChecked();
	m_kxkbConfig.m_resetOldOptions = widget->checkResetOld->isChecked();
	m_kxkbConfig.m_options = createOptionString();

	Q3ListViewItem *item = widget->listLayoutsDst->firstChild();
	QList<LayoutUnit> layouts;
	while (item) {
		QString layout = item->text(LAYOUT_COLUMN_MAP);
		QString variant = item->text(LAYOUT_COLUMN_VARIANT);
		QString displayName = item->text(LAYOUT_COLUMN_DISPLAY_NAME);

		LayoutUnit layoutUnit(layout, variant);
		layoutUnit.displayName = displayName;
		layouts.append( layoutUnit );

		item = item->nextSibling();
		kDebug() << "To save: layout " << layoutUnit.toPair()
				<< ", disp: " << layoutUnit.displayName << endl;
	}
	m_kxkbConfig.m_layouts = layouts;

	if( m_kxkbConfig.m_layouts.count() == 0 ) {
		m_kxkbConfig.m_layouts.append(LayoutUnit(DEFAULT_LAYOUT_UNIT));
 		widget->chkEnable->setChecked(false);
 	}

	m_kxkbConfig.m_useKxkb = widget->chkEnable->isChecked();
	m_kxkbConfig.m_indicatorOnly = widget->chkIndicatorOnly->isChecked();
	m_kxkbConfig.m_showSingle = widget->chkShowSingle->isChecked();
	m_kxkbConfig.m_showFlag = widget->chkShowFlag->isChecked();

	int modeId = widget->grpSwitching->id(widget->grpSwitching->selected());
	switch( modeId ) {
		default:
		case 0:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_GLOBAL;
			break;
		case 1:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_WIN_CLASS;
			break;
		case 2:
			m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_WINDOW;
			break;
	}

	m_kxkbConfig.m_stickySwitching = widget->chkEnableSticky->isChecked();
	m_kxkbConfig.m_stickySwitchingDepth = widget->spinStickyDepth->value();

	m_kxkbConfig.save();

	KToolInvocation::kdeinitExec("kxkb");
	emit KCModule::changed( false );
}


void LayoutConfig::updateStickyLimit()
{
    int layoutsCnt = widget->listLayoutsDst->childCount();
	int maxDepth = layoutsCnt - 1;

	if( maxDepth < 2 ) {
		maxDepth = 2;
	}

	widget->spinStickyDepth->setMaximum(maxDepth);
/*	if( value > maxDepth )
		setValue(maxDepth);*/
}

void LayoutConfig::add()
{
    Q3ListViewItem* sel = widget->listLayoutsSrc->selectedItem();
    if( sel == 0 || widget->listLayoutsDst->childCount() >= GROUP_LIMIT )
		return;

    // Create a copy of the sel widget, as one might add the same layout more
    // than one time, with different variants.
    Q3ListViewItem* toadd = copyLVI(sel, widget->listLayoutsDst);

    widget->listLayoutsDst->insertItem(toadd);
    if( widget->listLayoutsDst->childCount() > 1 )
		toadd->moveItem(widget->listLayoutsDst->lastItem());
// disabling temporary: does not work reliable in Qt :(
//    widget->listLayoutsDst->setSelected(sel, true);
//    layoutSelChanged(sel);

	updateAddButton();
	updateLayoutCommand();
    updateStickyLimit();
    changed();
}

void LayoutConfig::updateAddButton()
{
    bool aboveLimit = widget->listLayoutsDst->childCount() >= GROUP_LIMIT;
	widget->btnAdd->setEnabled(! aboveLimit);
}

void LayoutConfig::remove()
{
    Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();
    Q3ListViewItem* newSel = 0;

    if( sel != NULL ) {
		if( sel->itemBelow() )
			newSel = sel->itemBelow();
		else
			if( sel->itemAbove() )
				newSel = sel->itemAbove();

		delete sel;
		if( newSel )
			widget->listLayoutsSrc->setSelected(newSel, true);
		layoutSelChanged(newSel);
	}

	updateAddButton();
	updateLayoutCommand();
	updateStickyLimit();
    changed();
}

void LayoutConfig::moveUp()
{
    Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();
    if( sel == 0 || sel->itemAbove() == 0 )
		return;

    if( sel->itemAbove()->itemAbove() == 0 ) {
		widget->listLayoutsDst->takeItem(sel);
		widget->listLayoutsDst->insertItem(sel);
		widget->listLayoutsDst->setSelected(sel, true);
    }
    else
		sel->moveItem(sel->itemAbove()->itemAbove());

	updateLayoutCommand();
    changed();
}

void LayoutConfig::moveDown()
{
    Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();
    if( sel == 0 || sel->itemBelow() == 0 )
		return;

    sel->moveItem(sel->itemBelow());
	updateLayoutCommand();
    changed();
}

void LayoutConfig::variantChanged()
{
    Q3ListViewItem* selLayout = widget->listLayoutsDst->selectedItem();
    if( selLayout == NULL ) {
      widget->comboVariant->clear();
      widget->comboVariant->setEnabled(false);
      return;
    }

	QString selectedVariant = widget->comboVariant->currentText();
	if( selectedVariant == DEFAULT_VARIANT_NAME )
		selectedVariant = "";
	selLayout->setText(LAYOUT_COLUMN_VARIANT, selectedVariant);

	updateLayoutCommand();
    changed();
}

// helper
LayoutUnit LayoutConfig::getLayoutUnitKey(Q3ListViewItem *sel)
{
	QString kbdLayout = sel->text(LAYOUT_COLUMN_MAP);
	QString kbdVariant = sel->text(LAYOUT_COLUMN_VARIANT);
	return LayoutUnit(kbdLayout, kbdVariant);
}

void LayoutConfig::displayNameChanged(const QString& newDisplayName)
{
	Q3ListViewItem* selLayout = widget->listLayoutsDst->selectedItem();
	if( selLayout == NULL )
		return;

	const LayoutUnit layoutUnitKey = getLayoutUnitKey( selLayout );
	LayoutUnit& layoutUnit = *m_kxkbConfig.m_layouts.find(layoutUnitKey);

	QString oldName = selLayout->text(LAYOUT_COLUMN_DISPLAY_NAME);

	if( oldName.isEmpty() )
		oldName = KxkbConfig::getDefaultDisplayName( layoutUnit );

	if( oldName != newDisplayName ) {
		kDebug() << "setting label for " << layoutUnit.toPair() << " : " << newDisplayName;
		selLayout->setText(LAYOUT_COLUMN_DISPLAY_NAME, newDisplayName);
		updateIndicator(selLayout);
		changed();
	}
}

/** will update flag with label if layout label has been edited
*/
void LayoutConfig::updateIndicator(Q3ListViewItem*)
{
}

void LayoutConfig::layoutSelChanged(Q3ListViewItem *sel)
{
    widget->comboVariant->clear();
    widget->comboVariant->setEnabled( sel != NULL );

    if( sel == NULL ) {
        return;
    }

	LayoutUnit layoutUnitKey = getLayoutUnitKey(sel);
	QString kbdLayout = layoutUnitKey.layout;

	QStringList vars = m_rules->getAvailableVariants(kbdLayout);
	kDebug() << "layout " << kbdLayout << " has " << vars.count() << " variants";

	if( vars.count() > 0 ) {
		vars.prepend(DEFAULT_VARIANT_NAME);
		widget->comboVariant->addItems(vars);

		QString variant = sel->text(LAYOUT_COLUMN_VARIANT);
		if( variant != NULL && variant.isEmpty() == false ) {
			widget->comboVariant->setCurrentText(variant);
		}
		else {
			widget->comboVariant->setCurrentIndex(0);
		}
	}
//	updateDisplayName();
}

QWidget* LayoutConfig::makeOptionsTab()
{
  Q3ListView *listView = widget->listOptions;

  listView->setMinimumHeight(150);
  listView->setSortColumn( -1 );
  listView->setColumnText( 0, i18n( "Options" ) );
  listView->clear();

  connect(listView, SIGNAL(clicked(Q3ListViewItem *)), SLOT(changed()));
  connect(listView, SIGNAL(clicked(Q3ListViewItem *)), SLOT(updateOptionsCommand()));

  connect(widget->chkEnableOptions, SIGNAL(toggled(bool)), SLOT(changed()));

  connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(changed()));
  connect(widget->checkResetOld, SIGNAL(toggled(bool)), SLOT(updateOptionsCommand()));

  //Create controllers for all options
  QHashIterator<QString, XkbOptionGroup> it( m_rules->optionGroups() );
  for (; it.hasNext(); )
  {
	  OptionListItem *parent;
	  const XkbOptionGroup& optionGroup = it.next().value();

      if( optionGroup.exclusive ) {
        parent = new OptionListItem(listView, i18n( optionGroup.description ),
          		Q3CheckListItem::RadioButtonController, optionGroup.name);
        OptionListItem *item = new OptionListItem(parent, i18n( "None" ),
          		Q3CheckListItem::RadioButton, "none");
        item->setState(Q3CheckListItem::On);
      }
      else {
        parent = new OptionListItem(listView, i18n( optionGroup.description ),
            Q3CheckListItem::CheckBoxController, optionGroup.name);
      }

      parent->setOpen(true);
      m_optionGroups.insert( optionGroup.name, parent);
	  kDebug() << "optionGroup insert: " << optionGroup.name;
  }


  QHashIterator<QString, XkbOption> it2( m_rules->options() );
  for (; it2.hasNext(); )
  {
	  const XkbOption& option = it2.next().value();

	  OptionListItem *parent = m_optionGroups[option.group->name];
	  if( parent == NULL ) {
		kError() << "no option group item for group: " << option.group->name
			   << " for option " << option.name << endl;
		exit(1);
	  }

     if( parent->type() == Q3CheckListItem::RadioButtonController )
		new OptionListItem(parent, i18n( option.description ),
             Q3CheckListItem::RadioButton, option.name);
     else
	 	new OptionListItem(parent, i18n( option.description ),
            Q3CheckListItem::CheckBox, option.name);

//	  kDebug() << "option insert: " << option.name;
  }

  //scroll->setMinimumSize(450, 330);

  return listView;
}

void LayoutConfig::updateOptionsCommand()
{
  QString setxkbmap;
  QString options = createOptionString();

  if( !options.isEmpty() ) {
    setxkbmap = "setxkbmap -option "; //-rules " + m_rule
    if( widget->checkResetOld->isChecked() )
      setxkbmap += "-option ";
    setxkbmap += options;
  }
  widget->editCmdLineOpt->setText(setxkbmap);
}

void LayoutConfig::updateLayoutCommand()
{
	QString kbdLayouts;
	QString kbdVariants;

	Q3ListViewItem *item = widget->listLayoutsDst->firstChild();
	QList<LayoutUnit> layouts;
	while (item) {
		QString layout = item->text(LAYOUT_COLUMN_MAP);
		QString variant = item->text(LAYOUT_COLUMN_VARIANT);
		QString displayName = item->text(LAYOUT_COLUMN_DISPLAY_NAME);

		if( variant == DEFAULT_VARIANT_NAME )
			variant = "";

		if( kbdLayouts.length() > 0 ) {
			kbdLayouts += ',';
			kbdVariants += ',';
		}

		kbdLayouts += layout;
		kbdVariants += variant;

		item = item->nextSibling();
	}

    QString setxkbmap = "setxkbmap"; //-rules " + m_rule
    setxkbmap += " -model " + lookupLocalized(m_rules->models(), widget->comboModel->currentText());
    setxkbmap += " -layout " + kbdLayouts;
    setxkbmap += " -variant " + kbdVariants;

	widget->editCmdLine->setText(setxkbmap);
}

/*
void LayoutConfig::updateDisplayName()
{
	Q3ListViewItem* sel = widget->listLayoutsDst->selectedItem();

	QString layoutDisplayName;
		kDebug() << "sel: '" << sel << "'";
	if( sel != NULL ) {
		LayoutUnit layoutUnitKey = getLayoutUnitKey(sel);
		QString kbdLayout = layoutUnitKey.layout;
		QString variant = layoutUnitKey.variant;
	//	QString layoutDisplayName = m_kxkbConfig.getLayoutDisplayName( *m_kxkbConfig.m_layouts.find(layoutUnitKey) );
		layoutDisplayName = sel->text(LAYOUT_COLUMN_DISPLAY_NAME);
		if( layoutDisplayName.isEmpty() ) {
			int count = 0;
			Q3ListViewItem *item = widget->listLayoutsDst->firstChild();
			while (item) {
				QString layout_ = item->text(LAYOUT_COLUMN_MAP);
				if( layout_ == kbdLayout )
					++count;
				item = item->nextSibling();
			}
			bool single = count < 2;
			layoutDisplayName = m_kxkbConfig.getDefaultDisplayName(LayoutUnit(kbdLayout, variant), single);
		}
		kDebug() << "disp: '" << layoutDisplayName << "'";
	}

	widget->editDisplayName->setEnabled( sel != NULL );
	widget->editDisplayName->setText(layoutDisplayName);
}
*/

void LayoutConfig::changed()
{
  bool enabled = widget->chkEnable->isChecked();

  widget->chkIndicatorOnly->setEnabled(enabled);
  if( ! enabled )
	widget->chkIndicatorOnly->setChecked(false);

  widget->grpLayouts->setEnabled(enabled);
  widget->grpSwitching->setEnabled(enabled);
//  widget->grpStickySwitching->setEnabled(enabled);

//  bool indicatorOnly = widget->chkIndicatorOnly->isChecked();
//  widget->grpIndicator->setEnabled(indicatorOnly);
  
  emit KCModule::changed( true );
}


void LayoutConfig::loadRules()
{
    // do we need this ?
    // this could obly be used if rules are changed and 'Defaults' is pressed
    delete m_rules;
    m_rules = new XkbRules();

    QStringList modelsList;
    QHashIterator<QString, QString> it(m_rules->models());
    while (it.hasNext()) {
		modelsList.append(i18n(it.next().value()));
    }
    modelsList.sort();

	widget->comboModel->clear();
	widget->comboModel->addItems(modelsList);
	widget->comboModel->setCurrentIndex(0);

	// fill in the additional layouts
	widget->listLayoutsSrc->clear();
	widget->listLayoutsDst->clear();
	QHashIterator<QString, QString> it2(m_rules->layouts());

	while (it2.hasNext())
	{
		it2.next();
		QString layout = it2.key();
		QString layoutName = it2.value();
		Q3ListViewItem *item = new Q3ListViewItem(widget->listLayoutsSrc);

		item->setPixmap(LAYOUT_COLUMN_FLAG, LayoutIcon::getInstance().findPixmap(layout, true));
		item->setText(LAYOUT_COLUMN_NAME, i18n( layoutName.toLatin1().constData() ));
		item->setText(LAYOUT_COLUMN_MAP, layout);
	}
	widget->listLayoutsSrc->setSorting(LAYOUT_COLUMN_NAME);	// from Qt3 QListView sorts by language

	//TODO: reset options and xkb options
}


QString LayoutConfig::createOptionString()
{
  QString options;
  for (QHashIterator<QString, XkbOption> it(m_rules->options()); it.hasNext(); )
  {
    const XkbOption& option = it.next().value();

      OptionListItem *item = m_optionGroups[ option.group->name ];

      if( !item ) {
        kDebug() << "WARNING: skipping empty group for " << option.name
          << " - could not found group: " << option.group->name << endl;
        continue;
      }

      OptionListItem *child = item->findChildItem( option.name );

      if ( child ) {
        if ( child->state() == Q3CheckListItem::On ) {
          QString selectedName = child->optionName();
          if ( !selectedName.isEmpty() && selectedName != "none" ) {
            if (!options.isEmpty())
              options.append(',');
            options.append(selectedName);
          }
        }
      }
      else
        kDebug() << "Empty option button for group " << it.key();
  }
  return options;
}


void LayoutConfig::defaults()
{
	loadRules();
	m_kxkbConfig.setDefaults();

	initUI();

	emit KCModule::changed( true );
}


OptionListItem::OptionListItem( OptionListItem *parent, const QString &text,
								Type tt, const QString &optionName )
	: Q3CheckListItem( parent, text, tt ), m_OptionName( optionName )
{
}

OptionListItem::OptionListItem( Q3ListView *parent, const QString &text,
								Type tt, const QString &optionName )
	: Q3CheckListItem( parent, text, tt ), m_OptionName( optionName )
{
}

OptionListItem * OptionListItem::findChildItem( const QString& optionName )
{
	OptionListItem *child = static_cast<OptionListItem *>( firstChild() );

	while ( child )
	{
		if ( child->optionName() == optionName )
			break;
		child = static_cast<OptionListItem *>( child->nextSibling() );
	}

	return child;
}

extern "C"
{
	KDE_EXPORT void kcminit_keyboard()
	{
		KeyboardConfig::init_keyboard();

		KxkbConfig m_kxkbConfig;
		m_kxkbConfig.load(KxkbConfig::LOAD_INIT_OPTIONS);

		if( m_kxkbConfig.m_useKxkb == true ) {
			KToolInvocation::startServiceByDesktopName("kxkb");
		}
		else {
		// Even if the layouts have been disabled we still want to set Xkb options
		// user can always switch them off now in the "Options" tab
			if( m_kxkbConfig.m_enableXkbOptions ) {
				if( !XKBExtension::setXkbOptions(m_kxkbConfig.m_options, m_kxkbConfig.m_resetOldOptions) ) {
					kDebug() << "Setting XKB options failed!";
				}
			}
		}
	}
}
