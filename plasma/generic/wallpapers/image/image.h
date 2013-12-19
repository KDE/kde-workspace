/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <QTimer>
#include <QPixmap>
#include <QStringList>
#include <QObject>
#include <QPersistentModelIndex>

#include <Plasma/Package>


class QPropertyAnimation;

class KDirWatch;
class KFileDialog;
class KJob;

namespace KNS3 {
    class DownloadDialog;
}

class BackgroundListModel;

class Image : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RenderingMode renderingMode READ renderingMode WRITE setRenderingMode NOTIFY renderingModeChanged)
    Q_PROPERTY(ResizeMethod resizeMethod READ resizeMethod NOTIFY resizeMethodChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QAbstractItemModel *wallpaperModel READ wallpaperModel CONSTANT)
    Q_PROPERTY(int slideTimer READ slideTimer WRITE setSlideTimer NOTIFY slideTimerChanged)
    Q_PROPERTY(QStringList usersWallpapers READ usersWallpapers WRITE setUsersWallpapers NOTIFY usersWallpapersChanged)
    Q_PROPERTY(QStringList slidePaths READ slidePaths WRITE setSlidePaths NOTIFY slidePathsChanged)

    public:
        /**
         * Various resize modes supported by the built in image renderer
         */
        enum ResizeMethod {
            ScaledResize /**< Scales the image to fit the full area*/,
            CenteredResize /**< Centers the image within the area */,
            ScaledAndCroppedResize /**< Scales and crops the image, preserving the aspect ratio */,
            TiledResize /**< Tiles the image to fill the area */,
            CenterTiledResize /**< Tiles the image to fill the area, starting with a centered tile */,
            MaxpectResize /**< Best fit resize */,
            LastResizeMethod = MaxpectResize
        };
        Q_ENUMS(ResizeMethod)

        enum RenderingMode {
            SingleImage,
            SlideShow
        };
        Q_ENUMS(RenderingMode)

        Image(QObject* parent = 0);
        ~Image();

        QString wallpaperPath() const;

        //this is for QML use
        Q_INVOKABLE void addUrl(const QString &url);
        Q_INVOKABLE void addUrls(const QStringList &urls);


        RenderingMode renderingMode() const;
        void setRenderingMode(RenderingMode mode);

        ResizeMethod resizeMethod() const;
        void setResizeMethod(ResizeMethod method);

        QSize targetSize() const;
        void setTargetSize(const QSize &size);

        Plasma::Package *package();

        QAbstractItemModel* wallpaperModel();

        int slideTimer() const;
        void setSlideTimer(int time);

        QStringList usersWallpapers() const;
        void setUsersWallpapers(const QStringList &usersWallpapers);

        QStringList slidePaths() const;
        void setSlidePaths(const QStringList &slidePaths);

    Q_SIGNALS:
        void settingsChanged(bool);
        void wallpaperPathChanged();
        void renderingModeChanged();
        void slideTimerChanged();
        void usersWallpapersChanged();
        void slidePathsChanged();
        void resizeMethodChanged();

    protected Q_SLOTS:
        void removeWallpaper(QString name);
        void timeChanged(const QTime& time);
        void addDir();
        void removeDir();
        void getNewWallpaper();
        void wallpaperBrowseCompleted();
        void nextSlide();
        /**
         * Open the current slide in the default image application
         */
        void openSlide();
        void showFileDialog();
        void startSlideshow();
        void fileDialogFinished();
        void addUrl(const QUrl &url, bool setAsCurrent);
        void addUrls(const QList<QUrl> &urls);
        void setWallpaper(const QString &path);
        void setWallpaperRetrieved(KJob *job);
        void addWallpaperRetrieved(KJob *job);
        void newStuffFinished();
        void updateDirWatch(const QStringList &newDirs);
        void addDirFromSelectionDialog();
        void pathCreated(const QString &path);
        void pathDeleted(const QString &path);
        void pathDirty(const QString &path);
        void backgroundsFound(const QStringList &paths, const QString &token);

    protected:
        void setSingleImage();
        void updateWallpaperActions();
        void useSingleImageDefaults();

    private:
        static bool s_startupResumed;
        static bool s_startupSuspended;

        int m_delay;
        QStringList m_dirs;
        QString m_wallpaper;
        QString m_wallpaperPath;
        QStringList m_usersWallpapers;
        KDirWatch *m_dirWatch;
        bool m_scanDirty;
        ResizeMethod m_resizeMethod;
        QSize m_targetSize;

        RenderingMode m_mode;
        Plasma::Package m_wallpaperPackage;
        QStringList m_slideshowBackgrounds;
        QStringList m_slidePaths;
        QTimer m_timer;
        int m_currentSlide;
        BackgroundListModel *m_model;
        KFileDialog *m_dialog;
        QSize m_size;
        QString m_img;
        QDateTime m_previousModified;
        QWeakPointer<KNS3::DownloadDialog> m_newStuffDialog;
        QString m_findToken;
        QList<QAction *>m_actions;

        QAction* m_nextWallpaperAction;
        QAction* m_openImageAction;
};

#endif
