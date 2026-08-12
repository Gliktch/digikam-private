/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents - similar panel.
 *
 * SPDX-FileCopyrightText: 2016-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fuzzysearchview_p.h"

namespace Digikam
{

QWidget* FuzzySearchView::setupFindSimilarPanel() const
{
    const int spacing     = layoutSpacing();

    DHBox* const imageBox = new DHBox();
    d->imageWidget        = new QLabel(imageBox);
    d->imageWidget->setFixedSize(256, 256);
    d->imageWidget->setText(i18n("<p>Drag & drop an image here<br>to perform similar<br>items search.</p>"
                                 "<p>You can also use the context menu<br>when browsing through<br>your images.</p>"));
    d->imageWidget->setAlignment(Qt::AlignCenter);
    imageBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    imageBox->setLineWidth(1);

    // ---------------------------------------------------------------

    QLabel* const file   = new QLabel(i18n("<b>File</b>:"));
    d->labelFile         = new DAdjustableLabel(nullptr);
    QLabel* const folder = new QLabel(i18n("<b>Folder</b>:"));
    d->labelFolder       = new DAdjustableLabel(nullptr);
    int hgt              = fontMetrics().height() - 2;
    file->setMaximumHeight(hgt);
    folder->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);

    // ---------------------------------------------------------------

    d->fuzzySearchAlbumSelectors = new AlbumSelectors(i18nc("@label", "Search in albums:"),
                                                      QLatin1String("Fuzzy Search View"),
                                                      nullptr, AlbumSelectors::AlbumType::PhysAlbum, true);

    // ---------------------------------------------------------------

    QLabel* const resultsLabel = new QLabel(i18n("Similarity range:"));
    d->similarityRange         = new DIntRangeBox();
    d->similarityRange->setSuffix(QLatin1String("%"));

    if (d->settings)
    {
        d->similarityRange->setRange(d->settings->getMinimumSimilarityBound(), 100);
        d->similarityRange->setInterval(d->settings->getDuplicatesSearchLastMinSimilarity(),
                                        d->settings->getDuplicatesSearchLastMaxSimilarity());
    }
    else
    {
        d->similarityRange->setRange(40, 100);
        d->similarityRange->setInterval(90, 100);
    }

    d->similarityRange->setWhatsThis(i18n("Select here the approximate similarity interval "
                                          "as a percentage. "));

    // ---------------------------------------------------------------

    DHBox* const saveBox = new DHBox();
    saveBox->setContentsMargins(QMargins());
    saveBox->setSpacing(spacing);

    d->nameEditImage = new DTextEdit(saveBox);
    d->nameEditImage->setLinesVisible(1);
    d->nameEditImage->setWhatsThis(i18n("Enter the name of the current similar image search to save in the "
                                        "\"Similarity Searches\" view."));

    d->saveBtnImage  = new QToolButton(saveBox);
    d->saveBtnImage->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveBtnImage->setEnabled(false);
    d->saveBtnImage->setToolTip(i18n("Save current similar image search to a new virtual Album"));
    d->saveBtnImage->setWhatsThis(i18n("If you press this button, the current "
                                       "similar image search will be saved to a new search "
                                       "virtual album using name "
                                       "set on the left side."));

    // ---------------------------------------------------------------

    QWidget* const mainWidget     = new QWidget();
    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(imageBox,                      0, 0, 1, 6);
    mainLayout->addWidget(file,                          1, 0, 1, 1);
    mainLayout->addWidget(d->labelFile,                  1, 1, 1, 5);
    mainLayout->addWidget(folder,                        2, 0, 1, 1);
    mainLayout->addWidget(d->labelFolder,                2, 1, 1, 5);
    mainLayout->addWidget(d->fuzzySearchAlbumSelectors,  3, 0, 1, -1);
    mainLayout->addWidget(resultsLabel,                  4, 0, 1, 1);
    mainLayout->addWidget(d->similarityRange,            4, 2, 1, 1);
    mainLayout->addWidget(saveBox,                       5, 0, 1, 6);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainWidget->setLayout(mainLayout);

    return mainWidget;
}

void FuzzySearchView::dragEnterEvent(QDragEnterEvent* e)
{
    if (dragEventWrapper(e->mimeData()))
    {
        e->acceptProposedAction();
    }
}

void FuzzySearchView::dragMoveEvent(QDragMoveEvent* e)
{
    if (dragEventWrapper(e->mimeData()))
    {
        e->acceptProposedAction();
    }
}

bool FuzzySearchView::dragEventWrapper(const QMimeData* data) const
{
    if      (DItemDrag::canDecode(data))
    {
        return true;
    }
    else if (data->hasUrls())
    {
        QList<QUrl> urls = data->urls();

        // If there is at least one URL and the URL is a local file.

        if (!urls.isEmpty() && urls.first().isLocalFile())
        {
            HaarIface haarIface;
            QString path       = urls.first().toLocalFile();
            const QImage image = haarIface.loadQImage(path);

            if (!image.isNull())
            {
                return true;
            }
        }
    }

    return false;
}

void FuzzySearchView::dropEvent(QDropEvent* e)
{
    if (DItemDrag::canDecode(e->mimeData()))
    {
        QList<QUrl>      urls;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
        {
            return;
        }

        if (imageIDs.isEmpty())
        {
            return;
        }

        setItemInfo(ItemInfo(imageIDs.first()));

        e->acceptProposedAction();
    }

    // Allow dropping urls and handle them as sketch search if the urls represent images.

    if (e->mimeData()->hasUrls())
    {
        QList<QUrl> urls = e->mimeData()->urls();

        // If there is at least one URL and the URL is a local file.

        if (!urls.isEmpty() && urls.first().isLocalFile())
        {
            HaarIface haarIface;
            QString path       = urls.first().toLocalFile();
            const QImage image = haarIface.loadQImage(path);

            if (!image.isNull())
            {
                // Set a temporary image id

                d->imageInfo = ItemInfo(-1);
                d->imageUrl  = urls.first();

                d->imageWidget->setPixmap(QPixmap::fromImage(image).scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));

                AlbumManager::instance()->clearCurrentAlbums();
                QString haarTitle = SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch);

                QList<int> albums = d->fuzzySearchAlbumSelectors->selectedAlbumIds();

                d->imageSAlbum = d->searchModificationHelper->createFuzzySearchFromDropped(haarTitle, path,
                                                                                           d->similarityRange->minValue() / 100.0,
                                                                                           d->similarityRange->maxValue() / 100.0,
                                                                                           albums, true);
                d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->imageSAlbum);
                d->labelFile->setAdjustedText(urls.first().fileName());
                d->labelFolder->setAdjustedText(urls.first().adjusted(QUrl::RemoveFilename).toLocalFile());

                slotCheckNameEditImageConditions();

                e->acceptProposedAction();
            }
        }
    }
}

void FuzzySearchView::slotMaxLevelImageChanged(int /*newValue*/)
{
    if (d->active)
    {
        d->timerImage->start();
    }
}

void FuzzySearchView::slotMinLevelImageChanged(int /*newValue*/)
{
    if (d->active)
    {
        d->timerImage->start();
    }
}

void FuzzySearchView::slotFuzzyAlbumsChanged()
{
    if (d->active)
    {
        d->timerImage->start();
    }
}

void FuzzySearchView::slotTimerImageDone()
{
    if (d->imageInfo.isNull() && (d->imageInfo.id() == -1) && !d->imageUrl.isEmpty())
    {
        AlbumManager::instance()->clearCurrentAlbums();
        QString haarTitle = SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch);

        QList<int> albums = d->fuzzySearchAlbumSelectors->selectedAlbumIds();

        d->imageSAlbum    = d->searchModificationHelper->createFuzzySearchFromDropped(haarTitle,
                                                                                      d->imageUrl.toLocalFile(),
                                                                                      d->similarityRange->minValue() / 100.0,
                                                                                      d->similarityRange->maxValue() / 100.0,
                                                                                      albums, true);
        d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->imageSAlbum);
        return;
    }

    if (!d->imageInfo.isNull() && d->active)
    {
        setItemInfo(d->imageInfo);
    }
}

void FuzzySearchView::setCurrentImage(qlonglong imageid)
{
    setCurrentImage(ItemInfo(imageid));
}

void FuzzySearchView::setCurrentImage(const ItemInfo& info)
{
    d->imageInfo = info;
    d->imageUrl  = info.fileUrl();
    d->labelFile->setAdjustedText(d->imageInfo.name());
    d->labelFolder->setAdjustedText(d->imageInfo.fileUrl().adjusted(QUrl::RemoveFilename).toLocalFile());
    d->thumbLoadThread->find(d->imageInfo.thumbnailIdentifier());
}

void FuzzySearchView::setItemInfo(const ItemInfo& info)
{
    setCurrentImage(info);
    slotCheckNameEditImageConditions();
    createNewFuzzySearchAlbumFromImage(SAlbum::getTemporaryHaarTitle(DatabaseSearch::HaarImageSearch), true);
    d->tabWidget->setCurrentIndex((int)Private::SIMILARS);
}

void FuzzySearchView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    if (!d->imageInfo.isNull() && (QUrl::fromLocalFile(desc.filePath) == d->imageInfo.fileUrl()))
    {
        d->imageWidget->setPixmap(pix.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void FuzzySearchView::createNewFuzzySearchAlbumFromImage(const QString& name, bool force)
{
    AlbumManager::instance()->clearCurrentAlbums();
    QList<int> albums = d->fuzzySearchAlbumSelectors->selectedAlbumIds();

    d->imageSAlbum = d->searchModificationHelper->createFuzzySearchFromImage(name, d->imageInfo,
                                                                             d->similarityRange->minValue() / 100.0,
                                                                             d->similarityRange->maxValue() / 100.0,
                                                                             albums, force);
    d->searchTreeView->setCurrentAlbums(QList<Album*>() << d->imageSAlbum);
}

void FuzzySearchView::slotCheckNameEditImageConditions()
{
    if (!d->imageInfo.isNull())
    {
        bool b = d->nameEditImage->text().isEmpty();
        d->nameEditImage->setEnabled(true);
        d->saveBtnImage->setEnabled(!b);
    }
    else
    {
        d->nameEditImage->setEnabled(false);
        d->saveBtnImage->setEnabled(false);
    }
}

void FuzzySearchView::slotSaveImageSAlbum()
{
    createNewFuzzySearchAlbumFromImage(d->nameEditImage->text());
}

} // namespace Digikam
