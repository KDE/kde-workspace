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

#include "kcm_add_layout_dialog.h"
#include <QApplication>
#include <klocalizedstring.h>

#include "xkb_rules.h"
#include "flags.h"
#include "iso_codes.h"

#include "ui_kcm_add_layout_dialog.h"


AddLayoutDialog::AddLayoutDialog(const Rules* rules_, Flags* flags_, bool showLabel, QWidget* parent):
		QDialog(parent),
		rules(rules_),
		flags(flags_),
		selectedLanguage("no_language")
{
    layoutDialogUi = new Ui_AddLayoutDialog();
    layoutDialogUi->setupUi(this);

    QSet<QString> languages;
    foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
    	QSet<QString> langs = QSet<QString>::fromList(layoutInfo->languages);
    	languages.unite( langs );
    }
    IsoCodes isoCodes(IsoCodes::iso_639_3);
    foreach(const QString& lang, languages) {
    	const IsoCodeEntry* isoCodeEntry = isoCodes.getEntry(IsoCodes::attr_iso_639_3_id, lang);
//    	const IsoCodeEntry* isoCodeEntry = isoCodes.getEntry(IsoCodes::attr_iso_639_2B_code, lang);
//    	if( isoCodeEntry == NULL ) {
//    		isoCodeEntry = isoCodes.getEntry(IsoCodes::attr_iso_639_2T_code, lang);
//    	}
    	QString name = isoCodeEntry != NULL ? i18n(isoCodeEntry->value(IsoCodes::attr_name).toUtf8()) : lang;
    	layoutDialogUi->languageComboBox->addItem(name, lang);
    }
    layoutDialogUi->languageComboBox->model()->sort(0);
	layoutDialogUi->languageComboBox->insertItem(0, i18n("Any language"), "");
	layoutDialogUi->languageComboBox->setCurrentIndex(0);

	if( showLabel ) {
		layoutDialogUi->labelEdit->setMaxLength(LayoutUnit::MAX_LABEL_LENGTH);
	}
	else {
		layoutDialogUi->labelLabel->setVisible(false);
		layoutDialogUi->labelEdit->setVisible(false);
	}

    languageChanged(0);
    connect(layoutDialogUi->languageComboBox, SIGNAL(activated(int)), this, SLOT(languageChanged(int)));
    connect(layoutDialogUi->layoutComboBox, SIGNAL(activated(int)), this, SLOT(layoutChanged(int)));
    connect(layoutDialogUi->prevbutton,SIGNAL(clicked()),this,SLOT(preview()));
}

void AddLayoutDialog::languageChanged(int langIdx)
{
	QString lang = layoutDialogUi->languageComboBox->itemData(langIdx).toString();
	if( lang == selectedLanguage )
		return;

	QPixmap emptyPixmap(layoutDialogUi->layoutComboBox->iconSize());
	emptyPixmap.fill(Qt::transparent);

	layoutDialogUi->layoutComboBox->clear();
	int defaultIndex = -1;
	int i = 0;
    foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
    	if( lang.isEmpty() || layoutInfo->isLanguageSupportedByLayout(lang) ) {
    		if( flags ) {
    			QIcon icon(flags->getIcon(layoutInfo->name));
    			if( icon.isNull() ) {
    				icon = QIcon(emptyPixmap);	// align text with no icons
    			}
    			layoutDialogUi->layoutComboBox->addItem(icon, layoutInfo->description, layoutInfo->name);
    		}
    		else {
    			layoutDialogUi->layoutComboBox->addItem(layoutInfo->description, layoutInfo->name);
    		}

    		// try to guess best default layout selection for given language
    		if( ! lang.isEmpty() && defaultIndex == -1 && layoutInfo->isLanguageSupportedByDefaultVariant(lang) ) {
    			defaultIndex = i;
    		}
    		i++;
    	}
    }
    if( defaultIndex == -1 ) {
    	defaultIndex = 0;
    }

    layoutDialogUi->layoutComboBox->model()->sort(0);
	layoutDialogUi->layoutComboBox->setCurrentIndex(defaultIndex);
	layoutChanged(defaultIndex);

	selectedLanguage = lang;
}

void AddLayoutDialog::layoutChanged(int layoutIdx)
{
	QString layoutName = layoutDialogUi->layoutComboBox->itemData(layoutIdx).toString();
	if( layoutName == selectedLayout )
		return;

	QString lang = layoutDialogUi->languageComboBox->itemData(layoutDialogUi->languageComboBox->currentIndex()).toString();

	layoutDialogUi->variantComboBox->clear();
	const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutName);
    foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
        if( lang.isEmpty() || layoutInfo->isLanguageSupportedByVariant(variantInfo, lang) ) {
        	layoutDialogUi->variantComboBox->addItem(variantInfo->description, variantInfo->name);
        }
    }

	layoutDialogUi->variantComboBox->model()->sort(0);

    if( lang.isEmpty() || layoutInfo->isLanguageSupportedByDefaultVariant(lang) ) {
    	layoutDialogUi->variantComboBox->insertItem(0, i18nc("variant", "Default"), "");
    }
	layoutDialogUi->variantComboBox->setCurrentIndex(0);

	layoutDialogUi->labelEdit->setText(layoutName);

	selectedLayout = layoutName;
}

void AddLayoutDialog::accept()
{
	selectedLayoutUnit.layout = layoutDialogUi->layoutComboBox->itemData(layoutDialogUi->layoutComboBox->currentIndex()).toString();
	selectedLayoutUnit.variant = layoutDialogUi->variantComboBox->itemData(layoutDialogUi->variantComboBox->currentIndex()).toString();
    QString label = layoutDialogUi->labelEdit->text();
	if( label == selectedLayoutUnit.layout ) {
		label = "";
	}
	selectedLayoutUnit.setDisplayName( label );
	selectedLayoutUnit.setShortcut(layoutDialogUi->kkeysequencewidget->keySequence());
	QDialog::accept();
}


void AddLayoutDialog::preview(){
    int index = layoutDialogUi->variantComboBox->currentIndex();
    QString variant = layoutDialogUi->variantComboBox->itemData(index).toString();
    KeyboardPainter* layoutPreview = new KeyboardPainter();
    layoutPreview->generateKeyboardLayout(selectedLayout, variant);
    layoutPreview->setModal(true);
    layoutPreview->exec();
    delete layoutPreview;
}
