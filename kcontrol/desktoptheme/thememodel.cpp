/*
 * ThemeModel
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 * Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 by Davide Bettio <davide.bettio@kdemail.net>

 * Portions Copyright (C) 2007 Paolo Capriotti <p.capriotti@gmail.com>
 * Portions Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 * Portions Copyright (C) 2008 by Petri Damsten <damu@iki.fi>
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "thememodel.h"

#include <QApplication>
#include <QFile>
#include <QPainter>

#include <KDesktopFile>
#include <KColorScheme>
#include <KStandardDirs>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

ThemeModel::ThemeModel( QObject *parent )
: QAbstractListModel( parent )
{
    reload();
}

ThemeModel::~ThemeModel()
{
    clearThemeList();
}

void ThemeModel::clearThemeList()
{
    foreach (const ThemeInfo& themeInfo, m_themes) {
        delete themeInfo.svg;
    }
    m_themes.clear();
}

void ThemeModel::reload()
{
    reset();
    clearThemeList();

    // get all desktop themes
    KStandardDirs dirs;
    const QStringList themes = dirs.findAllResources("data", "desktoptheme/*/metadata.desktop",
                                               KStandardDirs::NoDuplicates);
    foreach (const QString &theme, themes) {
        int themeSepIndex = theme.lastIndexOf('/', -1);
        QString themeRoot = theme.left(themeSepIndex);
        int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
        QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);

        KDesktopFile df(theme);

        if (df.noDisplay()) {
            continue;
        }

        QString name = df.readName();
        if (name.isEmpty()) {
            name = packageName;
        }
        const QString comment = df.readComment();
        const QString author = df.desktopGroup().readEntry("X-KDE-PluginInfo-Author",QString());
        const QString version = df.desktopGroup().readEntry("X-KDE-PluginInfo-Version",QString());


        Plasma::FrameSvg *svg = new Plasma::FrameSvg(this);
        const QString svgFile = themeRoot + "/widgets/background.svg";
        if (QFile::exists(svgFile)) {
            svg->setImagePath(svgFile);
        } else {
            svg->setImagePath(svgFile + "z");
        }
        svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        ThemeInfo info;
        info.package = packageName;
        info.description = comment;
        info.author = author;
        info.version = version;
        info.svg = svg;
        info.themeRoot = themeRoot;
        m_themes[name] = info;
    }

    beginInsertRows(QModelIndex(), 0, m_themes.size());
    endInsertRows();
}

int ThemeModel::rowCount(const QModelIndex &) const
{
    return m_themes.size();
}

QVariant ThemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_themes.size()) {
        return QVariant();
    }

    QMap<QString, ThemeInfo>::const_iterator it = m_themes.constBegin();
    for (int i = 0; i < index.row(); ++i) {
        ++it;
    }

    switch (role) {
        case Qt::DisplayRole:
            return it.key();
        case PackageNameRole:
            return (*it).package;
        case SvgRole:
            return qVariantFromValue((void*)(*it).svg);
        case PackageDescriptionRole:
            return (*it).description;
        case PackageAuthorRole:
            return (*it).author;
        case PackageVersionRole:
            return (*it).version;
        default:
            return QVariant();
    }
}

QModelIndex ThemeModel::indexOf(const QString &name) const
{
    QMapIterator<QString, ThemeInfo> it(m_themes);
    int i = -1;
    while (it.hasNext()) {
        ++i;
        if (it.next().value().package == name) {
            return index(i, 0);
        }
    }

    return QModelIndex();
}

ThemeDelegate::ThemeDelegate(QObject* parent)
: QAbstractItemDelegate(parent)
{
}

void ThemeDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString package = index.model()->data(index, ThemeModel::PackageNameRole).toString();

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    // draw image
    Plasma::FrameSvg *svg = static_cast<Plasma::FrameSvg *>(
            index.model()->data(index, ThemeModel::SvgRole).value<void *>());
    svg->resizeFrame(QSize(option.rect.width() - (2 * MARGIN), 100 - (2 * MARGIN)));
    QRect imgRect = QRect(option.rect.topLeft(),
            QSize(option.rect.width() - (2 * MARGIN), 100 - (2 * MARGIN)))
            .translated(MARGIN, MARGIN);
    svg->paintFrame(painter, QPoint(option.rect.left() + MARGIN, option.rect.top() + MARGIN));

    // draw text
    painter->save();
    QFont font = painter->font();
    font.setWeight(QFont::Bold);
    const QString colorFile = KStandardDirs::locate("data", "desktoptheme/" + package + "/colors");
    if (!colorFile.isEmpty()) {
        KSharedConfigPtr colors = KSharedConfig::openConfig(colorFile);
        KColorScheme colorScheme(QPalette::Active, KColorScheme::Window, colors);
        painter->setPen(colorScheme.foreground(KColorScheme::NormalText).color());
    }
    painter->setFont(font);
    painter->drawText(option.rect, Qt::AlignCenter | Qt::TextWordWrap, title);
    painter->restore();
}

QSize ThemeDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(152, 100);
}


