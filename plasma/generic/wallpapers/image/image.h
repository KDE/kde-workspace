/***************************************************************************
 *   Copyright 2007 Paolo Capriotti <p.capriotti@gmail.com>                *
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>                         *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
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


#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <QTimer>
#include <QPixmap>
#include <QStringList>
#include <QObject>
#include <QPersistentModelIndex>

#include <Plasma/Package>


class QPropertyAnimation;
class QFileDialog;

class KDirWatch;
class KJob;

namespace KNS3 {
    class DownloadDialog;
}

class BackgroundListModel;

class Image : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RenderingMode renderingMode READ renderingMode WRITE setRenderingMode NOTIFY renderingModeChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QAbstractItemModel *wallpaperModel READ wallpaperModel CONSTANT)
    Q_PROPERTY(int slideTimer READ slideTimer WRITE setSlideTimer NOTIFY slideTimerChanged)
    Q_PROPERTY(QStringList usersWallpapers READ usersWallpapers WRITE setUsersWallpapers NOTIFY usersWallpapersChanged)
    Q_PROPERTY(QStringList slidePaths READ slidePaths WRITE setSlidePaths NOTIFY slidePathsChanged)
    Q_PROPERTY(int width MEMBER m_width READ width WRITE setWidth NOTIFY sizeChanged)
    Q_PROPERTY(int height MEMBER m_height READ height WRITE setHeight NOTIFY sizeChanged)

    public:

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

        Q_INVOKABLE void addSlidePath(const QString &path);
        Q_INVOKABLE void removeSlidePath(const QString &path);

        Q_INVOKABLE void getNewWallpaper();
        Q_INVOKABLE void showFileDialog();

        Q_INVOKABLE void addUsersWallpaper(const QString &file);

        RenderingMode renderingMode() const;
        void setRenderingMode(RenderingMode mode);

        QSize targetSize() const;
        void setTargetSize(const QSize &size);

        int width() const;
        int height() const;
        void setWidth(int w);
        void setHeight(int h);

        Plasma::Package *package();

        QAbstractItemModel* wallpaperModel();

        int slideTimer() const;
        void setSlideTimer(int time);

        QStringList usersWallpapers() const;
        void setUsersWallpapers(const QStringList &usersWallpapers);

        QStringList slidePaths() const;
        void setSlidePaths(const QStringList &slidePaths);

    public Q_SLOTS:
        void nextSlide();

    Q_SIGNALS:
        void settingsChanged(bool);
        void wallpaperPathChanged();
        void renderingModeChanged();
        void slideTimerChanged();
        void usersWallpapersChanged();
        void slidePathsChanged();
        void resizeMethodChanged();
        void sizeChanged(QSize s);

    protected Q_SLOTS:
        void removeWallpaper(QString name);
        void timeChanged(const QTime& time);
        void showAddSlidePathsDialog();
        void wallpaperBrowseCompleted();
        /**
         * Open the current slide in the default image application
         */
        void openSlide();
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
        QSize m_targetSize;

        RenderingMode m_mode;
        Plasma::Package m_wallpaperPackage;
        QStringList m_slideshowBackgrounds;
        QStringList m_unseenSlideshowBackgrounds;
        QStringList m_slidePaths;
        QTimer m_timer;
        int m_currentSlide;
        BackgroundListModel *m_model;
        QFileDialog *m_dialog;
        QSize m_size;
        int m_width;
        int m_height;
        QString m_img;
        QDateTime m_previousModified;
        QWeakPointer<KNS3::DownloadDialog> m_newStuffDialog;
        QString m_findToken;
};

#endif
