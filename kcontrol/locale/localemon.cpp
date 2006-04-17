/*
 * localemon.cpp
 *
 * Copyright (c) 1999-2003 Hans Petter Bieker <bieker@kde.org>
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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <qlayout.h>
#include <q3groupbox.h>

#include <qregexp.h>
//Added by qt3to4:
#include <QGridLayout>
#include <knuminput.h>
#include <kdialog.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "toplevel.h"
#include "localemon.h"
#include "localemon.moc"

KLocaleConfigMoney::KLocaleConfigMoney(KLocale *locale,
                                       QWidget *parent, const char*name)
  : QWidget(parent, name),
    m_locale(locale)
{
  // Money
  QGridLayout *lay = new QGridLayout(this, 6, 2,
                                     KDialog::marginHint(),
                                     KDialog::spacingHint());

  m_labMonCurSym = new QLabel(this);
  m_labMonCurSym->setObjectName( I18N_NOOP("Currency symbol:") );
  lay->addWidget(m_labMonCurSym, 0, 0);
  m_edMonCurSym = new QLineEdit(this);
  lay->addWidget(m_edMonCurSym, 0, 1);
  connect( m_edMonCurSym, SIGNAL( textChanged(const QString &) ),
           SLOT( slotMonCurSymChanged(const QString &) ) );

  m_labMonDecSym = new QLabel(this);
  m_labMonDecSym->setObjectName( I18N_NOOP("Decimal symbol:") );
  lay->addWidget(m_labMonDecSym, 1, 0);
  m_edMonDecSym = new QLineEdit(this);
  lay->addWidget(m_edMonDecSym, 1, 1);
  connect( m_edMonDecSym, SIGNAL( textChanged(const QString &) ),
           SLOT( slotMonDecSymChanged(const QString &) ) );

  m_labMonThoSep = new QLabel(this);
  m_labMonThoSep->setObjectName( I18N_NOOP("Thousands separator:") );
  lay->addWidget(m_labMonThoSep, 2, 0);
  m_edMonThoSep = new QLineEdit(this);
  lay->addWidget(m_edMonThoSep, 2, 1);
  connect( m_edMonThoSep, SIGNAL( textChanged(const QString &) ),
           SLOT( slotMonThoSepChanged(const QString &) ) );

  m_labMonFraDig = new QLabel(this);
  m_labMonFraDig->setObjectName( I18N_NOOP("Fract digits:") );
  lay->addWidget(m_labMonFraDig, 3, 0);
  m_inMonFraDig = new KIntNumInput(this);
  m_inMonFraDig->setRange(0, 10, 1, false);
  lay->addWidget(m_inMonFraDig, 3, 1);

  connect( m_inMonFraDig, SIGNAL( valueChanged(int) ),
           SLOT( slotMonFraDigChanged(int) ) );

  QWidget *vbox = new QWidget(this);
  QVBoxLayout *vboxLayout1 = new QVBoxLayout(vbox);
  vbox->setLayout(vboxLayout1);
  lay->addMultiCellWidget(vbox, 4, 4, 0, 1);
  Q3GroupBox *vgrp;
  vgrp = new Q3GroupBox( 1, Qt::Horizontal, vbox, I18N_NOOP("Positive") );
  m_chMonPosPreCurSym = new QCheckBox(vgrp, I18N_NOOP("Prefix currency symbol"));
  connect( m_chMonPosPreCurSym, SIGNAL( clicked() ),
           SLOT( slotMonPosPreCurSymChanged() ) );

  QWidget *hbox;
  hbox = new QWidget(vgrp );
  QHBoxLayout *hboxLayout1 = new QHBoxLayout(hbox);
  hbox->setLayout(hboxLayout1);
  m_labMonPosMonSignPos = new QLabel(hbox);
  m_labMonPosMonSignPos->setObjectName( I18N_NOOP("Sign position:") );
  m_cmbMonPosMonSignPos = new QComboBox(hbox, "signpos");
  connect( m_cmbMonPosMonSignPos, SIGNAL( activated(int) ),
           SLOT( slotMonPosMonSignPosChanged(int) ) );

  vgrp = new Q3GroupBox(  1, Qt::Horizontal, vbox, I18N_NOOP("Negative") );
  m_chMonNegPreCurSym = new QCheckBox(vgrp, I18N_NOOP("Prefix currency symbol"));
  connect( m_chMonNegPreCurSym, SIGNAL( clicked() ),
           SLOT( slotMonNegPreCurSymChanged() ) );

  hbox = new QWidget(vgrp );
  QHBoxLayout *hboxLayout2 = new QHBoxLayout(hbox);
  hbox->setLayout(hboxLayout2);
  m_labMonNegMonSignPos = new QLabel(hbox);
  m_labMonNegMonSignPos->setObjectName( I18N_NOOP("Sign position:") );
  m_cmbMonNegMonSignPos = new QComboBox(hbox, "signpos");
  connect( m_cmbMonNegMonSignPos, SIGNAL( activated(int) ),
           SLOT( slotMonNegMonSignPosChanged(int) ) );

  // insert some items
  int i = 5;
  while (i--)
  {
    m_cmbMonPosMonSignPos->addItem(QString());
    m_cmbMonNegMonSignPos->addItem(QString());
  }

  lay->setColStretch(1, 1);
  lay->addRowSpacing(5, 0);

  adjustSize();
}

KLocaleConfigMoney::~KLocaleConfigMoney()
{
}

void KLocaleConfigMoney::save()
{
  KConfig *config = KGlobal::config();
  KConfigGroup group(config, "Locale");

  KSimpleConfig ent(locate("locale",
                           QString::fromLatin1("l10n/%1/entry.desktop")
                           .arg(m_locale->country())), true);
  ent.setGroup("KCM Locale");

  QString str;
  int i;
  bool b;

  str = ent.readEntry("CurrencySymbol", QString::fromLatin1("$"));
  group.deleteEntry("CurrencySymbol", KConfigBase::Global);
  if (str != m_locale->currencySymbol())
    group.writeEntry("CurrencySymbol",
                       m_locale->currencySymbol(), KConfigBase::Persistent|KConfigBase::Global);

  str = ent.readEntry("MonetaryDecimalSymbol", QString::fromLatin1("."));
  group.deleteEntry("MonetaryDecimalSymbol", KConfigBase::Global);
  if (str != m_locale->monetaryDecimalSymbol())
    group.writeEntry("MonetaryDecimalSymbol",
                       m_locale->monetaryDecimalSymbol(), KConfigBase::Persistent|KConfigBase::Global);

  str = ent.readEntry("MonetaryThousandsSeparator", QString::fromLatin1(","));
  str.replace(QString::fromLatin1("$0"), QString());
  group.deleteEntry("MonetaryThousandsSeparator", KConfigBase::Global);
  if (str != m_locale->monetaryThousandsSeparator())
    group.writeEntry("MonetaryThousandsSeparator",
                       QString::fromLatin1("$0%1$0")
                       .arg(m_locale->monetaryThousandsSeparator()),
                       KConfigBase::Persistent|KConfigBase::Global);

  i = ent.readEntry("FracDigits", 2);
  group.deleteEntry("FracDigits", KConfigBase::Global);
  if (i != m_locale->fracDigits())
    group.writeEntry("FracDigits", m_locale->fracDigits(), KConfigBase::Persistent|KConfigBase::Global);

  b = ent.readEntry("PositivePrefixCurrencySymbol", true);
  group.deleteEntry("PositivePrefixCurrencySymbol", KConfigBase::Global);
  if (b != m_locale->positivePrefixCurrencySymbol())
    group.writeEntry("PositivePrefixCurrencySymbol",
                       m_locale->positivePrefixCurrencySymbol(), KConfigBase::Persistent|KConfigBase::Global);

  b = ent.readEntry("NegativePrefixCurrencySymbol", true);
  group.deleteEntry("NegativePrefixCurrencySymbol", KConfigBase::Global);
  if (b != m_locale->negativePrefixCurrencySymbol())
    group.writeEntry("NegativePrefixCurrencySymbol",
                       m_locale->negativePrefixCurrencySymbol(), KConfigBase::Persistent|KConfigBase::Global);

  i = ent.readEntry("PositiveMonetarySignPosition",
                       (int)KLocale::BeforeQuantityMoney);
  group.deleteEntry("PositiveMonetarySignPosition", KConfigBase::Global);
  if (i != m_locale->positiveMonetarySignPosition())
    group.writeEntry("PositiveMonetarySignPosition",
                       (int)m_locale->positiveMonetarySignPosition(),
                       KConfigBase::Persistent|KConfigBase::Global);

  i = ent.readEntry("NegativeMonetarySignPosition",
                       (int)KLocale::ParensAround);
  group.deleteEntry("NegativeMonetarySignPosition", KConfigBase::Global);
  if (i != m_locale->negativeMonetarySignPosition())
    group.writeEntry("NegativeMonetarySignPosition",
                       (int)m_locale->negativeMonetarySignPosition(),
                       KConfigBase::Persistent|KConfigBase::Global);

  group.sync();
}

void KLocaleConfigMoney::slotLocaleChanged()
{
  m_edMonCurSym->setText( m_locale->currencySymbol() );
  m_edMonDecSym->setText( m_locale->monetaryDecimalSymbol() );
  m_edMonThoSep->setText( m_locale->monetaryThousandsSeparator() );
  m_inMonFraDig->setValue( m_locale->fracDigits() );

  m_chMonPosPreCurSym->setChecked( m_locale->positivePrefixCurrencySymbol() );
  m_chMonNegPreCurSym->setChecked( m_locale->negativePrefixCurrencySymbol() );
  m_cmbMonPosMonSignPos->setCurrentIndex( m_locale->positiveMonetarySignPosition() );
  m_cmbMonNegMonSignPos->setCurrentIndex( m_locale->negativeMonetarySignPosition() );
}

void KLocaleConfigMoney::slotMonCurSymChanged(const QString &t)
{
  m_locale->setCurrencySymbol(t);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonDecSymChanged(const QString &t)
{
  m_locale->setMonetaryDecimalSymbol(t);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonThoSepChanged(const QString &t)
{
  m_locale->setMonetaryThousandsSeparator(t);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonFraDigChanged(int value)
{
  m_locale->setFracDigits(value);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonPosPreCurSymChanged()
{
  m_locale->setPositivePrefixCurrencySymbol(m_chMonPosPreCurSym->isChecked());
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonNegPreCurSymChanged()
{
  m_locale->setNegativePrefixCurrencySymbol(m_chMonNegPreCurSym->isChecked());
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonPosMonSignPosChanged(int i)
{
  m_locale->setPositiveMonetarySignPosition((KLocale::SignPosition)i);
  emit localeChanged();
}

void KLocaleConfigMoney::slotMonNegMonSignPosChanged(int i)
{
  m_locale->setNegativeMonetarySignPosition((KLocale::SignPosition)i);
  emit localeChanged();
}

void KLocaleConfigMoney::slotTranslate()
{
  QList<QComboBox*> list;
  list.append(m_cmbMonPosMonSignPos);
  list.append(m_cmbMonNegMonSignPos);

  foreach (QComboBox* wc, list)
  {
    wc->setItemText(0, ki18n("Parentheses Around").toString(m_locale));
    wc->setItemText(1, ki18n("Before Quantity Money").toString(m_locale));
    wc->setItemText(2, ki18n("After Quantity Money").toString(m_locale));
    wc->setItemText(3, ki18n("Before Money").toString(m_locale));
    wc->setItemText(4, ki18n("After Money").toString(m_locale));
  }

  QString str;

  str = ki18n( "Here you can enter your usual currency "
               "symbol, e.g. $ or DM."
               "<p>Please note that the Euro symbol may not be "
               "available on your system, depending on the "
               "distribution you use." ).toString( m_locale );
  m_labMonCurSym->setWhatsThis( str );
  m_edMonCurSym->setWhatsThis( str );
  str = ki18n( "Here you can define the decimal separator used "
               "to display monetary values."
               "<p>Note that the decimal separator used to "
               "display other numbers has to be defined "
               "separately (see the 'Numbers' tab)." ).toString( m_locale );
  m_labMonDecSym->setWhatsThis( str );
  m_edMonDecSym->setWhatsThis( str );

  str = ki18n( "Here you can define the thousands separator "
               "used to display monetary values."
               "<p>Note that the thousands separator used to "
               "display other numbers has to be defined "
               "separately (see the 'Numbers' tab)." ).toString( m_locale );
  m_labMonThoSep->setWhatsThis( str );
  m_edMonThoSep->setWhatsThis( str );

  str = ki18n( "This determines the number of fract digits for "
               "monetary values, i.e. the number of digits you "
               "find <em>behind</em> the decimal separator. "
               "Correct value is 2 for almost all people." ).toString( m_locale );
  m_labMonFraDig->setWhatsThis( str );
  m_inMonFraDig->setWhatsThis( str );

  str = ki18n( "If this option is checked, the currency sign "
               "will be prefixed (i.e. to the left of the "
               "value) for all positive monetary values. If "
               "not, it will be postfixed (i.e. to the right)." ).toString( m_locale );
  m_chMonPosPreCurSym->setWhatsThis( str );

  str = ki18n( "If this option is checked, the currency sign "
               "will be prefixed (i.e. to the left of the "
               "value) for all negative monetary values. If "
               "not, it will be postfixed (i.e. to the right)." ).toString( m_locale );
  m_chMonNegPreCurSym->setWhatsThis( str );

  str = ki18n( "Here you can select how a positive sign will be "
               "positioned. This only affects monetary values." ).toString( m_locale );
  m_labMonPosMonSignPos->setWhatsThis( str );
  m_cmbMonPosMonSignPos->setWhatsThis( str );

  str = ki18n( "Here you can select how a negative sign will "
               "be positioned. This only affects monetary "
               "values." ).toString( m_locale );
  m_labMonNegMonSignPos->setWhatsThis( str );
  m_cmbMonNegMonSignPos->setWhatsThis( str );
}
