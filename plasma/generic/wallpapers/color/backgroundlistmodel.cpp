#ifndef BACKGROUNDLISTMODEL_CPP
#define BACKGROUNDLISTMODEL_CPP
/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgroundlistmodel.h"

#include <QPainter>

#include "backgrounddelegate.h"
#include "color.h"

BackgroundListModel::BackgroundListModel(Color *listener, QObject *parent)
    : QAbstractListModel(parent),
      m_structureParent(listener)
{
    m_previewUnavailablePix.fill(Qt::transparent);
}

BackgroundListModel::~BackgroundListModel()
{
}

void BackgroundListModel::reload()
{
    for (int i = 0; i < m_backgroundModes.size(); i++) {
        m_previews.insert(m_backgroundModes[i], createPixmap(m_backgroundModes[i]));
    }
}

void BackgroundListModel::addColor(int mode, const QString &title)
{
    m_titles.insert(mode, title);
    m_backgroundModes.append(mode);
    m_previews.insert(mode, createPixmap(mode));
}

QModelIndex BackgroundListModel::indexOf(const int &path) const
{
    for (int i = 0; i < m_backgroundModes.size(); i++) {
        if (path == m_backgroundModes[i]) {
            return index(i, 0);
        }
    }
    return QModelIndex();
}

int BackgroundListModel::rowCount(const QModelIndex &) const
{
    return m_backgroundModes.size();
}

QVariant BackgroundListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_backgroundModes.size()) {
        return QVariant();
    }

    int mode = backgroundMode(index.row());

    switch (role) {
    case Qt::DisplayRole: {
        if (m_titles.contains(mode)) {
            return m_titles.value(mode);
        }

        return QVariant();
    }
    break;

    case BackgroundDelegate::ScreenshotRole: {
        if (m_previews.contains(mode)) {
            return m_previews.value(mode);
        }

        const_cast<BackgroundListModel *>(this)->m_previews.insert(mode, m_previewUnavailablePix);
        return m_previewUnavailablePix;
    }
    break;

    default:
        return QVariant();
    break;
    }
    return QVariant();
}

int BackgroundListModel::backgroundMode(int index) const
{
    return m_backgroundModes.at(index);
}

QPixmap BackgroundListModel::createPixmap(int mode) const
{
    QPixmap pix(120, 80);
    QPainter p(&pix);
    m_structureParent.data()->generatePainting(mode, &p, pix.rect(), pix.rect());
    p.end();

    return pix;
}
#include "backgroundlistmodel.moc"


#endif // BACKGROUNDLISTMODEL_CPP
