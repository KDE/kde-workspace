/*
  Copyright (c) 2007 by Paolo Capriotti <p.capriotti@gmail.com>
  Copyright (c) 2007 by Aaron Seigo <aseigo@kde.org>
  Copyright (c) 2008 by Alexis Ménard <darktears31@gmail.com>
  Copyright (c) 2008 by Petri Damsten <damu@iki.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "image.h"

#include <QApplication>
#include <QPainter>
#include <QFile>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QTimer>

#include <KDebug>
#include <KDirSelectDialog>
#include <KDirWatch>
#include <KFileDialog>
#include <KRandom>
#include <KStandardDirs>
#include <KIO/Job>
#include <krun.h>
#include <knewstuff3/downloaddialog.h>

#include <Plasma/Theme>
#include "backgroundlistmodel.h"
#include "backgrounddelegate.h"
#include "removebuttonmanager.h"
#include "ksmserver_interface.h"

K_EXPORT_PLASMA_WALLPAPER(image, Image)

Image::Image(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args),
      m_delay(10),
      m_dirWatch(0),
      m_scanDirty(false),
      m_configWidget(0),
      m_wallpaperPackage(0),
      m_currentSlide(-1),
      m_fadeValue(0),
      m_animation(0),
      m_model(0),
      m_dialog(0),
      m_nextWallpaperAction(0),
      m_openImageAction(0)
{
    connect(this, SIGNAL(renderCompleted(QImage)), this, SLOT(wallpaperRenderComplete(QImage)));
    connect(this, SIGNAL(renderHintsChanged()), this, SLOT(checkSize()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(nextSlide()));
    connect(&m_delayedRenderTimer, SIGNAL(timeout()), this, SLOT(actuallyRenderWallpaper()));
    m_delayedRenderTimer.setSingleShot(true);
}

Image::~Image()
{
    delete m_animation;
}

void Image::init(const KConfigGroup &config)
{
    m_timer.stop();

    if (renderingMode().name().isEmpty()) {
        m_mode = "SingleImage";
    } else {
        m_mode = renderingMode().name();
    }

    calculateGeometry();

    m_delay = config.readEntry("slideTimer", 10);
    setResizeMethodHint((ResizeMethod)config.readEntry("wallpaperposition", (int)ScaledResize));
    m_wallpaper = config.readEntry("wallpaper", QString());
    if (m_wallpaper.isEmpty()) {
        useSingleImageDefaults();
    }

    m_color = config.readEntry("wallpapercolor", QColor(Qt::black));
    m_usersWallpapers = config.readEntry("userswallpapers", QStringList());
    QStringList dirs = config.readEntry("slidepaths", QStringList());

    if (dirs.isEmpty()) {
        dirs << KStandardDirs::installPath("wallpaper");
    }

    setUsingRenderingCache(m_mode == "SingleImage");

    if (m_mode == "SingleImage") {
        setSingleImage();
        setContextualActions(QList<QAction*>());
    } else {
        m_nextWallpaperAction = new QAction(KIcon("user-desktop"), i18n("Next Wallpaper Image"), this);
        connect(m_nextWallpaperAction, SIGNAL(triggered(bool)), this, SLOT(nextSlide()));
        m_openImageAction = new QAction(KIcon("document-open"), i18n("Open Wallpaper Image"), this);
        connect(m_openImageAction, SIGNAL(triggered(bool)), this, SLOT(openSlide()));
        QTimer::singleShot(200, this, SLOT(startSlideshow()));
        updateDirWatch(dirs);
        QList<QAction*> actions;
        actions.push_back(m_nextWallpaperAction);
        actions.push_back(m_openImageAction);
        setContextualActions(actions);
        updateWallpaperActions();
    }
}

void Image::useSingleImageDefaults()
{
    m_wallpaper = Plasma::Theme::defaultTheme()->wallpaperPath();
    int index = m_wallpaper.indexOf("/contents/images/");
    if (index > -1) { // We have file from package -> get path to package
        m_wallpaper = m_wallpaper.left(index);
    }
}

void Image::save(KConfigGroup &config)
{
    config.writeEntry("slideTimer", m_delay);
    config.writeEntry("wallpaperposition", (int)resizeMethodHint());
    config.writeEntry("slidepaths", m_dirs);
    config.writeEntry("wallpaper", m_wallpaper);
    config.writeEntry("wallpapercolor", m_color);
    config.writeEntry("userswallpapers", m_usersWallpapers);
}

void Image::configWidgetDestroyed()
{
    m_configWidget = 0;
    m_model = 0;
}

QWidget* Image::createConfigurationInterface(QWidget* parent)
{
    m_configWidget = new QWidget(parent);
    connect(m_configWidget, SIGNAL(destroyed(QObject*)), this, SLOT(configWidgetDestroyed()));

    if (m_mode == "SingleImage") {
        m_uiImage.setupUi(m_configWidget);

        m_model = new BackgroundListModel(this, m_configWidget);
        m_model->setResizeMethod(resizeMethodHint());
        m_model->setWallpaperSize(m_size);
        m_model->reload(m_usersWallpapers);
        QTimer::singleShot(0, this, SLOT(setConfigurationInterfaceModel()));
        m_uiImage.m_view->setItemDelegate(new BackgroundDelegate(m_uiImage.m_view));
        //FIXME: setting the minimum width is rather ugly, but this gets us 3 columns of papers
        //which looks quite good as a default. the magic number 7 at the end of the calculation is
        //evidently making up for some other PM involved in the QListView that isn't being caught.
        //if a cleaner way can be found to achieve all this, that would be great
        m_uiImage.m_view->setMinimumWidth((BackgroundDelegate::SCREENSHOT_SIZE + BackgroundDelegate::MARGIN * 2 +
                                           BackgroundDelegate::BLUR_INCREMENT) * 3 +
                                           m_uiImage.m_view->spacing() * 4 +
                                           QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
                                           QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2 + 7);
        m_uiImage.m_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        RemoveButtonManager *rmManager = new RemoveButtonManager(m_uiImage.m_view, &m_usersWallpapers);
        connect(rmManager, SIGNAL(removeClicked(QString)), this, SLOT(removeWallpaper(QString)));

        m_uiImage.m_pictureUrlButton->setIcon(KIcon("document-open"));
        connect(m_uiImage.m_pictureUrlButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));

        m_uiImage.m_resizeMethod->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Scaled"), ScaledResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Centered"), CenteredResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Tiled"), TiledResize);
        m_uiImage.m_resizeMethod->addItem(i18n("Center Tiled"), CenterTiledResize);
        for (int i = 0; i < m_uiImage.m_resizeMethod->count(); ++i) {
            if (resizeMethodHint() == m_uiImage.m_resizeMethod->itemData(i).value<int>()) {
                m_uiImage.m_resizeMethod->setCurrentIndex(i);
                break;
            }
        }
        connect(m_uiImage.m_resizeMethod, SIGNAL(currentIndexChanged(int)),
                this, SLOT(positioningChanged(int)));

        m_uiImage.m_color->setColor(m_color);
        //Color button is useless with some resize methods
        m_uiImage.m_color->setEnabled(resizeMethodHint() == MaxpectResize || resizeMethodHint() == CenteredResize);
        connect(m_uiImage.m_color, SIGNAL(changed(QColor)), this, SLOT(colorChanged(QColor)));

        m_uiImage.m_newStuff->setIcon(KIcon("get-hot-new-stuff"));
        connect(m_uiImage.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));

        connect(m_uiImage.m_color, SIGNAL(changed(QColor)), this, SLOT(modified()));
        connect(m_uiImage.m_resizeMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));
        connect(m_uiImage.m_view, SIGNAL(clicked(QModelIndex)), this, SLOT(modified()));

    } else {
        m_uiSlideshow.setupUi(m_configWidget);
        m_uiSlideshow.m_newStuff->setIcon(KIcon("get-hot-new-stuff"));
        m_uiSlideshow.m_dirlist->clear();
        m_uiSlideshow.m_systemCheckBox->setChecked(false);
        m_uiSlideshow.m_downloadedCheckBox->setChecked(false);

        QString systemPath = KStandardDirs::installPath("wallpaper");
        QString localPath = KGlobal::dirs()->saveLocation("wallpaper");

        foreach (const QString &dir, m_dirs) {
            if (dir == KStandardDirs::installPath("wallpaper")) {
                m_uiSlideshow.m_systemCheckBox->setChecked(true);
            } else if (dir == localPath) {
                m_uiSlideshow.m_downloadedCheckBox->setChecked(true);
            } else {
                m_uiSlideshow.m_dirlist->addItem(dir);
            }
        }
        m_uiSlideshow.m_dirlist->setCurrentRow(0);
        updateDirs();
        m_uiSlideshow.m_addDir->setIcon(KIcon("list-add"));
        connect(m_uiSlideshow.m_addDir, SIGNAL(clicked()), this, SLOT(addDir()));
        m_uiSlideshow.m_removeDir->setIcon(KIcon("list-remove"));
        connect(m_uiSlideshow.m_removeDir, SIGNAL(clicked()), this, SLOT(removeDir()));

        QTime time(0, 0, 0);
        time = time.addSecs(m_delay);
        m_uiSlideshow.m_slideshowDelay->setTime(time);
        m_uiSlideshow.m_slideshowDelay->setMinimumTime(QTime(0, 0, 10));
        connect(m_uiSlideshow.m_slideshowDelay, SIGNAL(timeChanged(QTime)),
                this, SLOT(timeChanged(QTime)));

        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled"), ScaledResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Centered"), CenteredResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Tiled"), TiledResize);
        m_uiSlideshow.m_resizeMethod->addItem(i18n("Center Tiled"), CenterTiledResize);
        for (int i = 0; i < m_uiSlideshow.m_resizeMethod->count(); ++i) {
            if (resizeMethodHint() == m_uiSlideshow.m_resizeMethod->itemData(i).value<int>()) {
                m_uiSlideshow.m_resizeMethod->setCurrentIndex(i);
                break;
            }
        }
        connect(m_uiSlideshow.m_resizeMethod, SIGNAL(currentIndexChanged(int)),
                this, SLOT(positioningChanged(int)));

        m_uiSlideshow.m_color->setColor(m_color);
        //Color button is useless with some resize methods
        m_uiSlideshow.m_color->setEnabled(resizeMethodHint() == MaxpectResize || resizeMethodHint() == CenteredResize);
        connect(m_uiSlideshow.m_color, SIGNAL(changed(QColor)), this, SLOT(colorChanged(QColor)));
        connect(m_uiSlideshow.m_newStuff, SIGNAL(clicked()), this, SLOT(getNewWallpaper()));

        connect(m_uiSlideshow.m_systemCheckBox, SIGNAL(toggled(bool)),
                this, SLOT(systemCheckBoxToggled(bool)));
        connect(m_uiSlideshow.m_downloadedCheckBox, SIGNAL(toggled(bool)),
                this, SLOT(downloadedCheckBoxToggled(bool)));
        connect(m_uiSlideshow.m_color, SIGNAL(changed(QColor)), this, SLOT(modified()));
        connect(m_uiSlideshow.m_resizeMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));
        connect(m_uiSlideshow.m_addDir, SIGNAL(clicked()), this, SLOT(modified()));
        connect(m_uiSlideshow.m_removeDir, SIGNAL(clicked()), this, SLOT(modified()));
        connect(m_uiSlideshow.m_slideshowDelay, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(modified()));
	connect(m_uiSlideshow.m_dirlist, SIGNAL(currentRowChanged(int)), SLOT(updateDirs()));
    }

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    return m_configWidget;
}

void Image::systemCheckBoxToggled(bool checked)
{
    if (checked) {
        m_dirs << KStandardDirs::installPath("wallpaper");
    } else {
        m_dirs.removeAll(KStandardDirs::installPath("wallpaper"));
    }
    modified();
}

void Image::downloadedCheckBoxToggled(bool checked)
{
    if (checked) {
        m_dirs << KGlobal::dirs()->saveLocation("wallpaper");
    } else {
        m_dirs.removeAll(KGlobal::dirs()->saveLocation("wallpaper"));
    }
    modified();
}

void Image::setConfigurationInterfaceModel()
{
    m_uiImage.m_view->setModel(m_model);
    connect(m_uiImage.m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(pictureChanged(QModelIndex)));

    QModelIndex index = m_model->indexOf(m_wallpaper);
    if (index.isValid()) {
        m_uiImage.m_view->setCurrentIndex(index);
    }
}

void Image::modified()
{
    emit settingsChanged(true);
}

void Image::calculateGeometry()
{
    m_size = boundingRect().size().toSize();

    if (m_model) {
        m_model->setWallpaperSize(m_size);
    }
}

void Image::paint(QPainter *painter, const QRectF& exposedRect)
{
    // Check if geometry changed
    //kDebug() << m_size << boundingRect().size().toSize();
    if (m_pixmap.isNull()) {
        painter->fillRect(exposedRect, QBrush(m_color));
        //kDebug() << "pixmap null";
        return;
    }

    if (painter->worldMatrix() == QMatrix()) {
        // draw the background untransformed when possible;(saves lots of per-pixel-math)
        painter->resetTransform();
    }

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    // for pixmaps we draw only the exposed part (untransformed since the
    // bitmapBackground already has the size of the viewport)
    painter->drawPixmap(exposedRect, m_pixmap, exposedRect.translated(-boundingRect().topLeft()));

    if (!m_oldFadedPixmap.isNull()) {
        // Put old faded image on top.
        painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter->drawPixmap(exposedRect, m_oldFadedPixmap,
                            exposedRect.translated(-boundingRect().topLeft()));
    }
}

void Image::timeChanged(const QTime& time)
{
    m_delay = QTime(0, 0, 0).secsTo(time);
    if (!m_slideshowBackgrounds.isEmpty()) {
        m_timer.start(m_delay * 1000);
    }
}

void Image::addDir()
{
    KUrl empty;
    KDirSelectDialog *dialog = new KDirSelectDialog(empty, true, m_configWidget);
    connect(dialog, SIGNAL(accepted()), this, SLOT(addDirFromSelectionDialog()));
    dialog->show();
}

void Image::addDirFromSelectionDialog()
{
    KDirSelectDialog *dialog = qobject_cast<KDirSelectDialog *>(sender());
    if (dialog) {
        QString urlDir = dialog->url().path();
        if (!urlDir.isEmpty() && m_uiSlideshow.m_dirlist->findItems(urlDir, Qt::MatchExactly).isEmpty()) {
            m_uiSlideshow.m_dirlist->addItem(urlDir);
            updateDirs();
            startSlideshow();
        }
    }
}

void Image::removeDir()
{
    int row = m_uiSlideshow.m_dirlist->currentRow();
    if (row != -1) {
        m_uiSlideshow.m_dirlist->takeItem(row);
        updateDirs();
        startSlideshow();
    }
}

void Image::updateDirs()
{
    m_dirs.clear();

    if (m_uiSlideshow.m_systemCheckBox->isChecked()) {
        m_dirs << KStandardDirs::installPath("wallpaper");
    }
    if (m_uiSlideshow.m_downloadedCheckBox->isChecked()) {
        m_dirs << KGlobal::dirs()->saveLocation("wallpaper");
    }

    const int dirCount = m_uiSlideshow.m_dirlist->count();
    for (int i = 0; i < dirCount; ++i) {
        m_dirs.append(m_uiSlideshow.m_dirlist->item(i)->text());
    }

    m_uiSlideshow.m_removeDir->setEnabled(m_uiSlideshow.m_dirlist->currentRow() != -1);
}

void Image::updateDirWatch(const QStringList &newDirs)
{
    if (isPreviewing()) {
        return;
    }

    if (!m_dirWatch) {
        m_dirWatch = new KDirWatch(this);
        connect(m_dirWatch, SIGNAL(created(QString)), SLOT(pathCreated(QString)));
        connect(m_dirWatch, SIGNAL(dirty(QString)),   SLOT(pathDirty(QString)));
        connect(m_dirWatch, SIGNAL(deleted(QString)), SLOT(pathDeleted(QString)));
    }

    foreach (const QString &oldDir, m_dirs) {
        if (!newDirs.contains(oldDir)) {
            m_dirWatch->removeDir(oldDir);
        }
    }

    foreach (const QString &newDir, newDirs) {
        if (!m_dirWatch->contains(newDir)) {
            m_dirWatch->addDir(newDir, KDirWatch::WatchSubDirs | KDirWatch::WatchFiles);
        }
    }

    m_dirWatch->startScan();
    m_dirs = newDirs;
}

void Image::setSingleImage()
{
    if (isPreviewing()) {
        return;
    }

    if (m_wallpaper.isEmpty()) {
        useSingleImageDefaults();
    }

    QString img;

    if (QDir::isAbsolutePath(m_wallpaper)) {
        Plasma::Package b(m_wallpaper, packageStructure(this));
        img = b.filePath("preferred");
        //kDebug() << img << m_wallpaper;

        if (img.isEmpty() && QFile::exists(m_wallpaper)) {
            img = m_wallpaper;
        }
    } else {
        //if it's not an absolute path, check if it's just a wallpaper name
        const QString path = KStandardDirs::locate("wallpaper", m_wallpaper + "/metadata.desktop");

        if (!path.isEmpty()) {
            QDir dir(path);
            dir.cdUp();

            Plasma::Package b(dir.path(), packageStructure(this));
            img = b.filePath("preferred");
        }
    }

    if (img.isEmpty()) {
        // ok, so the package we have failed to work out; let's try the default
        // if we have already
        const QString wallpaper = m_wallpaper;
        useSingleImageDefaults();
        if (wallpaper != m_wallpaper) {
            setSingleImage();
        }
    }

    if (!m_size.isEmpty()) {
        renderWallpaper(img);
    }
}

void Image::addUrls(const KUrl::List &urls)
{
    bool first = true;
    foreach (const KUrl &url, urls) {
        // set the first drop as the current paper, just add the rest to the roll
        addUrl(url, first);
        first = false;
    }
}

void Image::addUrl(const KUrl &url, bool setAsCurrent)
{
    ///kDebug() << "droppage!" << url << url.isLocalFile();
    if (url.isLocalFile()) {
        const QString path = url.toLocalFile();
        if (setAsCurrent) {
            setWallpaper(path);
        } else {
            if (m_mode != "SingleImage") {
                // it's a slide show, add it to the slide show
                m_slideshowBackgrounds.append(path);
                m_unseenSlideshowBackgrounds.append(path);
            }

            // always add it to the user papers, though
            if (!m_usersWallpapers.contains(path)) {
                m_usersWallpapers.append(path);
            }
        }
    } else {
        QString wallpaperPath = KGlobal::dirs()->locateLocal("wallpaper", url.fileName());

        if (!wallpaperPath.isEmpty()) {
            KIO::FileCopyJob *job = KIO::file_copy(url, KUrl(wallpaperPath));
            if (setAsCurrent) {
                connect(job, SIGNAL(result(KJob*)), this, SLOT(setWallpaperRetrieved(KJob*)));
            } else {
                connect(job, SIGNAL(result(KJob*)), this, SLOT(addWallpaperRetrieved(KJob*)));
            }
        }
    }
}

void Image::setWallpaperRetrieved(KJob *job)
{
    KIO::FileCopyJob *copyJob = qobject_cast<KIO::FileCopyJob *>(job);
    if (copyJob && !copyJob->error()) {
        setWallpaper(copyJob->destUrl().toLocalFile());
    }
}

void Image::addWallpaperRetrieved(KJob *job)
{
    KIO::FileCopyJob *copyJob = qobject_cast<KIO::FileCopyJob *>(job);
    if (copyJob && !copyJob->error()) {
        addUrl(copyJob->destUrl(), false);
    }
}

void Image::setWallpaper(const QString &path)
{
    if (m_mode == "SingleImage") {
        m_wallpaper = path;
        setSingleImage();
    } else {
        m_slideshowBackgrounds.append(path);
        m_unseenSlideshowBackgrounds.clear();
        m_currentSlide = -1;
        nextSlide();
        updateWallpaperActions();
    }

    if (!m_usersWallpapers.contains(path)) {
        m_usersWallpapers.append(path);
    }
}

void Image::startSlideshow()
{
    if (isPreviewing()) {
        return;
    }

    if (m_findToken.isEmpty()) {
        // populate background list
        m_timer.stop();
        m_slideshowBackgrounds.clear();
        m_unseenSlideshowBackgrounds.clear();
        BackgroundFinder *finder = new BackgroundFinder(this, m_dirs);
        m_findToken = finder->token();
        connect(finder, SIGNAL(backgroundsFound(QStringList,QString)), this, SLOT(backgroundsFound(QStringList,QString)));
        finder->start();
        //TODO: what would be cool: paint on the wallpaper itself a busy widget and perhaps some text
        //about loading wallpaper slideshow while the thread runs
    } else {
        m_scanDirty = true;
    }
}

void Image::backgroundsFound(const QStringList &paths, const QString &token)
{
    if (token != m_findToken) {
        return;
    }

    m_findToken.clear();

    if(m_scanDirty) {
        m_scanDirty = false;
        startSlideshow();
        return;
    }

    m_slideshowBackgrounds = paths;
    m_unseenSlideshowBackgrounds.clear();
    updateWallpaperActions();
    // start slideshow
    if (m_slideshowBackgrounds.isEmpty()) {
        // no image has been found, which is quite weird... try again later (this is useful for events which
        // are not detected by KDirWatch, like a NFS directory being mounted)
        QTimer::singleShot(1000, this, SLOT(startSlideshow()));
        m_pixmap = QPixmap();
        emit update(boundingRect());
    } else {
        m_currentSlide = -1;
        nextSlide();
        m_timer.start(m_delay * 1000);
    }
}

void Image::updateWallpaperActions()
{
    if (m_nextWallpaperAction) {
        m_nextWallpaperAction->setEnabled(!m_slideshowBackgrounds.isEmpty());
    }

    if (m_openImageAction) {
        m_openImageAction->setEnabled(!m_slideshowBackgrounds.isEmpty());
    }
}

void Image::getNewWallpaper()
{
    if (!m_newStuffDialog) {
        m_newStuffDialog = new KNS3::DownloadDialog( "wallpaper.knsrc", m_configWidget );
        connect(m_newStuffDialog.data(), SIGNAL(accepted()), SLOT(newStuffFinished()));
    }
    m_newStuffDialog.data()->show();
}

void Image::newStuffFinished()
{
    if (m_model && (!m_newStuffDialog || m_newStuffDialog.data()->changedEntries().size() > 0)) {
        m_model->reload();
    }
}

void Image::colorChanged(const QColor& color)
{
    m_color = color;
    setSingleImage();
}

void Image::pictureChanged(const QModelIndex &index)
{
    if (index.row() == -1 || !m_model) {
        return;
    }

    Plasma::Package *b = m_model->package(index.row());
    if (!b) {
        return;
    }

    if (b->structure()->contentsPrefixPaths().isEmpty()) {
        // it's not a full package, but a single paper
        m_wallpaper = b->filePath("preferred");
    } else {
        m_wallpaper = b->path();
    }
}

void Image::positioningChanged(int index)
{
    if (m_mode == "SingleImage") {
        setResizeMethodHint((ResizeMethod)m_uiImage.m_resizeMethod->itemData(index).value<int>());
        setSingleImage();
    } else {
        setResizeMethodHint((ResizeMethod)m_uiSlideshow.m_resizeMethod->itemData(index).value<int>());
        startSlideshow();
    }

    //Color button is useless with some resize methods
    const bool colorizable = resizeMethodHint() == MaxpectResize || resizeMethodHint() == CenteredResize;
    if (m_mode == "SingleImage") {
        m_uiImage.m_color->setEnabled(colorizable);
    } else {
        m_uiSlideshow.m_color->setEnabled(colorizable);
    }

    if (m_model) {
        m_model->setResizeMethod(resizeMethodHint());
    }
}

void Image::showFileDialog()
{
    if (!m_dialog) {
        KUrl baseUrl;
        if(m_wallpaper.indexOf(QDir::homePath()) > -1){
            baseUrl = KUrl(m_wallpaper);
        }

        m_dialog = new KFileDialog(baseUrl, "*.png *.jpeg *.jpg *.xcf *.svg *.svgz *.bmp", m_configWidget);
        m_dialog->setOperationMode(KFileDialog::Opening);
        m_dialog->setInlinePreviewShown(true);
        m_dialog->setCaption(i18n("Select Wallpaper Image File"));
        m_dialog->setModal(false);

        connect(m_dialog, SIGNAL(okClicked()), this, SLOT(wallpaperBrowseCompleted()));
        connect(m_dialog, SIGNAL(destroyed(QObject*)), this, SLOT(fileDialogFinished()));
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void Image::fileDialogFinished()
{
    m_dialog = 0;
}

void Image::wallpaperBrowseCompleted()
{
    Q_ASSERT(m_model);

    const QFileInfo info(m_dialog->selectedFile());

    //the full file path, so it isn't broken when dealing with symlinks
    const QString wallpaper = info.canonicalFilePath();

    if (wallpaper.isEmpty()) {
        return;
    }

    if (m_model->contains(wallpaper)) {
        m_uiImage.m_view->setCurrentIndex(m_model->indexOf(wallpaper));
        return;
    }

    // add background to the model
    m_model->addBackground(wallpaper);

    // select it
    QModelIndex index = m_model->indexOf(wallpaper);
    if (index.isValid()) {
        m_uiImage.m_view->setCurrentIndex(index);
        pictureChanged(index);
        modified();
    }

    // save it
    m_usersWallpapers << wallpaper;
}

void Image::nextSlide()
{
    if (m_slideshowBackgrounds.isEmpty()) {
        return;
    }

    QString previousPath;
    if (m_currentSlide > -1 && m_currentSlide < m_unseenSlideshowBackgrounds.size()) {
        previousPath = m_unseenSlideshowBackgrounds.takeAt(m_currentSlide);
    }

    if (m_unseenSlideshowBackgrounds.isEmpty()) {
        m_unseenSlideshowBackgrounds = m_slideshowBackgrounds;

        // We're filling the queue again, make sure we can't pick up again
        // the last one picked from the previous set
        if (!previousPath.isEmpty()) {
            m_unseenSlideshowBackgrounds.removeAll(previousPath);

            // prevent empty list
            if (m_unseenSlideshowBackgrounds.isEmpty()) {
                m_unseenSlideshowBackgrounds = m_slideshowBackgrounds;
            }
        }
    }

    m_currentSlide = KRandom::random() % m_unseenSlideshowBackgrounds.size();
    const QString currentPath = m_unseenSlideshowBackgrounds.at(m_currentSlide);

    if (!m_wallpaperPackage) {
        m_wallpaperPackage = new Plasma::Package(currentPath, packageStructure(this));
    } else {
        m_wallpaperPackage->setPath(currentPath);
    }

    m_timer.stop();
    renderWallpaper(m_wallpaperPackage->filePath("preferred"));
    m_timer.start(m_delay * 1000);
}

void Image::openSlide()
{
    if (!m_wallpaperPackage) {
        return;
    }

    // open in image viewer
    KUrl filepath(m_wallpaperPackage->filePath("preferred"));
    //kDebug() << "opening file " << filepath.path();
    new KRun(filepath, NULL);
}

void Image::renderWallpaper(const QString& image)
{
    if (!image.isEmpty()) {
        m_img = image;
    }

    if (m_img.isEmpty()) {
        return;
    }

    m_delayedRenderTimer.start(100);
}

void Image::actuallyRenderWallpaper()
{
    render(m_img, m_size, resizeMethodHint(), m_color);
}

void Image::pathCreated(const QString &path)
{
    if (!m_slideshowBackgrounds.contains(path)) {
        QFileInfo fileInfo(path);
        if (fileInfo.isFile() && BackgroundFinder::suffixes().contains(fileInfo.suffix().toLower())) {
            m_slideshowBackgrounds.append(path);
            m_unseenSlideshowBackgrounds.append(path);
            if (m_slideshowBackgrounds.count() == 1) {
                nextSlide();
            }
        }
    }
}

void Image::pathDirty(const QString &path)
{
    if (path == m_img) {
        renderWallpaper(path);
    }
}

void Image::pathDeleted(const QString &path)
{
    if (m_slideshowBackgrounds.removeAll(path)) {
        m_unseenSlideshowBackgrounds.removeAll(path);
        if (path == m_img) {
            nextSlide();
        }
    }
}

void Image::wallpaperRenderComplete(const QImage &img)
{
    m_oldPixmap = m_pixmap;
    m_oldFadedPixmap = m_oldPixmap;
    m_pixmap = QPixmap::fromImage(img);

    if (!m_oldPixmap.isNull()) {
        if (!m_animation) {
            m_animation = new QPropertyAnimation(this, "fadeValue");
            m_animation->setProperty("easingCurve", QEasingCurve::OutQuad);
            m_animation->setProperty("duration", 300);
            m_animation->setProperty("startValue", 0.2);
            m_animation->setProperty("endValue", 1.0);
        }

        m_animation->start();
        setFadeValue(0.1);
    } else {
        emit update(boundingRect());
    }
}

void Image::updateScreenshot(QPersistentModelIndex index)
{
    m_uiImage.m_view->update(index);
}

qreal Image::fadeValue() const
{
    return m_fadeValue;
}

void Image::setFadeValue(qreal value)
{
    m_fadeValue = value;

    //If we are done, delete the pixmaps and don't draw.
    if (qFuzzyCompare(m_fadeValue, qreal(1.0))) {
        m_oldFadedPixmap = QPixmap();
        m_oldPixmap = QPixmap();
        emit update(boundingRect());
        return;
    }

    //Create the faded image.
    m_oldFadedPixmap.fill(Qt::transparent);

    QPainter p;
    p.begin(&m_oldFadedPixmap);
    p.drawPixmap(0, 0, m_oldPixmap);

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(m_oldFadedPixmap.rect(), QColor(0, 0, 0, 254 * (1-m_fadeValue)));//255*((150 - m_fadeValue)/150)));

    p.end();

    emit update(boundingRect());
}

//FIXME: we have to save the configuration also when the dialog cancel button is clicked.
void Image::removeWallpaper(QString name)
{
    int wallpaperIndex = m_usersWallpapers.indexOf(name);
    if (wallpaperIndex >= 0){
        m_usersWallpapers.removeAt(wallpaperIndex);
        m_model->reload(m_usersWallpapers);
        //TODO: save the configuration in the right way
        emit settingsChanged(true);
    }
}

bool Image::checkSize()
{
    if (m_size != boundingRect().size().toSize()) {
        calculateGeometry();
        if (!m_size.isEmpty()) { // We have a size set
            if (m_mode == "SingleImage") {
                // make sure we pick the best size wallpaper for the new size
                setSingleImage();
            } else {
                renderWallpaper();
            }
            //kDebug() << "re-rendering";
        }

        return false;
    }

    return true;
}

#include "image.moc"
