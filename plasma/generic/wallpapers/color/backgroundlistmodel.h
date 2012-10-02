/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef BACKGROUNDLISTMODEL_H
#define BACKGROUNDLISTMODEL_H

#include <QAbstractListModel>
#include <QPixmap>

#include <Plasma/Wallpaper>

class Color;

class BackgroundListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    BackgroundListModel(Color *listener, QObject *parent);
    virtual ~BackgroundListModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int backgroundMode(int index) const;

    void reload();
    QModelIndex indexOf(const int &path) const;

    //void setWallpaperSize(const QSize& size);
    void setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod);

    void addColor(int mode, const QString &title);

private:
    QPixmap createPixmap(int mode) const;

private:
    QWeakPointer<Color> m_structureParent;
    QList<int> m_backgroundModes;
    QHash<int, QPixmap> m_previews;
    QHash<int, QString> m_titles;
    QPixmap m_previewUnavailablePix;
};

#endif // BACKGROUNDLISTMODEL_H
