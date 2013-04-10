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

#include "iso_codes.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtXml/QXmlAttributes>


class IsoCodesPrivate {
public:
	IsoCodesPrivate(const QString& isoCode_, const QString& isoCodesXmlDir_):
		isoCode(isoCode_),
		isoCodesXmlDir(isoCodesXmlDir_),
		loaded(false)
	{}
	void buildIsoEntryList();

	const QString isoCode;
	const QString isoCodesXmlDir;
	QList<IsoCodeEntry> isoEntryList;
	bool loaded;
};

class XmlHandler : public QXmlDefaultHandler
{
public:
	XmlHandler(const QString& isoCode_, QList<IsoCodeEntry>& isoEntryList_):
		isoCode(isoCode_),
		qName("iso_"+isoCode+"_entry"),
		isoEntryList(isoEntryList_) {}

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
//    bool fatalError(const QXmlParseException &exception);
//    QString errorString() const;

private:
    const QString isoCode;
    const QString qName;
    QList<IsoCodeEntry>& isoEntryList;
};

bool XmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/,
                      const QString &qName, const QXmlAttributes &attributes)
{
	if( qName == this->qName ) {
		IsoCodeEntry entry;
		for(int i=0; i<attributes.count(); i++) {
			entry.insert(attributes.qName(i), attributes.value(i));
		}
		isoEntryList.append(entry);
	}
	return true;
}


IsoCodes::IsoCodes(const QString& isoCode, const QString& isoCodesXmlDir):
	d(new IsoCodesPrivate(isoCode, isoCodesXmlDir))
{
	KGlobal::locale()->insertCatalog(QString("iso_")+d->isoCode);
}

IsoCodes::~IsoCodes()
{
	KGlobal::locale()->removeCatalog(QString("iso_")+d->isoCode);
	delete d;
}

QList<IsoCodeEntry> IsoCodes::getEntryList()
{
	if( ! d->loaded ) {
		d->buildIsoEntryList();
	}
	return d->isoEntryList;
}

//const char* IsoCodes::iso_639="639";
const char* IsoCodes::iso_639_3="639_3";
const char* IsoCodes::attr_name="name";
//const char* IsoCodes::attr_iso_639_2B_code="iso_639_2B_code";
//const char* IsoCodes::attr_iso_639_2T_code="iso_639_2T_code";
//const char* IsoCodes::attr_iso_639_1_code="iso_639_1_code";
const char* IsoCodes::attr_iso_639_3_id="id";

const IsoCodeEntry* IsoCodes::getEntry(const QString& attributeName, const QString& attributeValue)
{
	if( ! d->loaded ) {
		d->buildIsoEntryList();
	}
	for(QList<IsoCodeEntry>::Iterator it = d->isoEntryList.begin(); it != d->isoEntryList.end(); ++it) {
		const IsoCodeEntry* isoCodeEntry = &(*it);
		if( isoCodeEntry->value(attributeName) == attributeValue )
			return isoCodeEntry;
	}
	return NULL;
}

void IsoCodesPrivate::buildIsoEntryList()
{
	loaded = true;

	QFile file(QString("%1/iso_%2.xml").arg(isoCodesXmlDir, isoCode));
	if( !file.open(QFile::ReadOnly | QFile::Text) ) {
		kError() << "Can't open the xml file" << file.fileName();
		return;
	}

	XmlHandler xmlHandler(isoCode, isoEntryList);

	QXmlSimpleReader reader;
	reader.setContentHandler(&xmlHandler);
	reader.setErrorHandler(&xmlHandler);

	QXmlInputSource xmlInputSource(&file);

	if( ! reader.parse(xmlInputSource) ) {
		kError() << "Failed to parse the xml file" << file.fileName();
		return;
	}

	kDebug() << "Loaded" << isoEntryList.count() << ("iso entry definitions for iso"+isoCode) << "from" << file.fileName();
}
