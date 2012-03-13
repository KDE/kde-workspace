/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QtGui/QPixmap>

class QString;
class QMimeData;
class QDataStream;

/**
 * An entry in the clipboard history.
 */
class HistoryItem
{
public:
    HistoryItem(const QByteArray& uuid);
    virtual ~HistoryItem();

    /**
     * Return the current item as text
     * An image would be returned as a descriptive
     * text, such as 32x43 image.
     */
    virtual QString text() const = 0;

    /**
     * @return uuid of current item.
     */
    const QByteArray& uuid() const {
        return m_uuid;
    }

    /**
     * Return the current item as pixmap
     * A text would be returned as a null pixmap,
     * which is also the default implementation
     */
    inline virtual const QPixmap& image() const;

    /**
     * Returns a pointer to a QMimeData suitable for QClipboard::setMimeData().
     */
    virtual QMimeData* mimeData() const = 0;

    /**
     * Write object on datastream
     */
    virtual void write( QDataStream& stream ) const = 0;

    /**
     * Equality.
     */
    virtual bool operator==(const HistoryItem& rhs) const = 0;

    /**
     * Create an HistoryItem from MimeSources (i.e., clipboard data)
     * returns null if create fails (e.g, unsupported mimetype)
     */
    static HistoryItem* create( const QMimeData* data );

    /**
     * Create an HistoryItem from data stream (i.e., disk file)
     * returns null if creation fails. In this case, the datastream
     * is left in an undefined state.
     */
    static HistoryItem* create( QDataStream& dataStream );

    /**
     * Inserts this item between prev and next
     */
    void insertBetweeen(HistoryItem* prev, HistoryItem* next);

    /**
     * Chain this with next
     */
    void chain(HistoryItem* next);

    /**
     * previous item's uuid
     */
    const QByteArray& previous_uuid() const {
        return m_previous_uuid;
    }

    /**
     * next item's uuid
     */
    const QByteArray& next_uuid() const {
        return m_next_uuid;
    }
private:
    QByteArray m_previous_uuid;
    QByteArray m_uuid;
    QByteArray m_next_uuid;
};

inline
const QPixmap& HistoryItem::image() const {
    static QPixmap nullPixmap;
    return nullPixmap;
}

inline
QDataStream& operator<<( QDataStream& lhs, HistoryItem const * const rhs ) {
    if ( rhs ) {
        rhs->write( lhs );
    }
    return lhs;

}

#endif
