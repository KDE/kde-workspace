/*
 *  Copyright (C) 2007 Andriy Rysin (rysin@kde.org)
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

#ifndef __kxkb_part_h
#define __kxkb_part_h

#include <kparts/part.h>

class QWidget;
class QString;

class KxkbPart : public KParts::ReadWritePart
{
  Q_OBJECT
 public:
  KxkbPart( QWidget* parentWidget,
               QObject* parent,
               const QStringList& args = QStringList() );
  virtual ~KxkbPart() {}

 protected slots:
  bool setLayout(const QString& layoutPair);

};

#endif
