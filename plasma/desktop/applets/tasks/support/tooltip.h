/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef TOOLTIP_PROXY_P
#define TOOLTIP_PROXY_P

#include <QObject>
#include <QWeakPointer>
#include <QtCore/QVariant>
#include <qwindowdefs.h>

class QGraphicsObject;
class QGraphicsWidget;
class DeclarativeItemContainer;

Q_DECLARE_METATYPE(WId);

/**
 * QML wrapper for kdelibs Plasma::ToolTip
 *
 * Exposed as `ToolTip` in QML.
 *
 * This is a fork of the canonical kde-runtime version for the purpose of gaining
 * access to the window preview-related functionality of Plasma::ToolTipManager
 * and Plasma::ToolTipContent and setting the tooltip to be clickable. Given the
 * frozen state of the Plasma version 1 QML components and the dubious nature of
 * having the window preview features in the core to begin with this was settled
 * upon as the best way forward for now; a better way will have to be found to
 * inject custom content into Plasma 2 QML Tooltips, and then this component can
 * be removed.
 */
class ToolTipProxy : public QObject
{
    Q_OBJECT

    /**
     * The item that will display this tooltip on mouse over 
     */
    Q_PROPERTY(QGraphicsObject *target READ target WRITE setTarget NOTIFY targetChanged)

    /**
     * The title of the tooltip, not more that 2-3 words
     */
    Q_PROPERTY(QString mainText READ mainText WRITE setMainText NOTIFY mainTextChanged)

    /**
     * subtitle of the tooltip. needed if a longer description is needed
     */
    Q_PROPERTY(QString subText READ subText WRITE setSubText NOTIFY subTextChanged)

    /**
     * Image to display in the tooltip, can be an image full path or a Freedesktop icon name or QIcon or QPixmap
     */
    Q_PROPERTY(QVariant image READ image WRITE setImage NOTIFY imageChanged)

    /**
     * List of window ids to show previews for in the tooltip.
     */
    Q_PROPERTY(QVariant windowsToPreview READ windowsToPreview WRITE setWindowsToPreview NOTIFY windowsToPreviewChanged)

    /**
     * Whether to highlight windows on window preview thumbnail hover.
     */
    Q_PROPERTY(bool highlightWindows READ highlightWindows WRITE setHighlightWindows NOTIFY highlightWindowsChanged)

public:
    ToolTipProxy(QObject *parent = 0);
    ~ToolTipProxy();

    QGraphicsObject *target() const;
    void setTarget(QGraphicsObject *target);

    QString mainText() const;
    void setMainText(const QString &text);

    QString subText() const;
    void setSubText(const QString &text);

    QVariant image() const;
    void setImage(QVariant name);

    QVariant windowsToPreview() const;
    void setWindowsToPreview(QVariant windows);

    bool highlightWindows() const;
    void setHighlightWindows(bool highlight);

    Q_INVOKABLE void hide() const;

Q_SIGNALS:
    void targetChanged();
    void mainTextChanged();
    void subTextChanged();
    void imageChanged();
    void windowsToPreviewChanged();
    void highlightWindowsChanged();

protected Q_SLOTS:
    void syncTarget();
    void updateToolTip();
    void activateWindow(WId window, Qt::MouseButtons buttons);

private:
    QString m_mainText;
    QString m_subText;
    QVariant m_image;
    QWeakPointer<QGraphicsWidget> m_widget;
    QWeakPointer<DeclarativeItemContainer> m_declarativeItemContainer;
    QWeakPointer<QGraphicsObject> m_target;
    QList<WId> m_windowsToPreview;
    bool m_highlightWindows;
};

#endif
