/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-06-17
 * Description : Find Duplicates tree-view search album item.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "findduplicatesalbumitem.h"

// Qt includes

#include <QtConcurrentRun>
#include <QPainter>
#include <QIcon>
#include <QMutexLocker>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "coredbsearchxml.h"
#include "deletedialog.h"
#include "itemviewutilities.h"
#include "itemsortcollator.h"
#include "dio.h"
#include "actionthreadbase.h"
#include "privacyanalysisgate.h"

namespace Digikam
{

class Q_DECL_HIDDEN FindDuplicatesAlbumItem::Private
{
public:

    Private() = default;

public:

    bool           hasThumb  = false;
    SAlbum*        album     = nullptr;
    int            itemCount = 0;

    ItemInfo       refImgInfo;
    QFuture<void>  calcTask;
    mutable QMutex mutex;       ///< Mutex to protect access to shared data
};

FindDuplicatesAlbumItem::FindDuplicatesAlbumItem(QTreeWidget* const parent, SAlbum* const album)
    : QObject        (parent),
      QTreeWidgetItem(parent),
      d              (new Private)
{
    d->album = album;

    // Connect signals for UI updates

    connect(this, &FindDuplicatesAlbumItem::signalUpdateItemText,
            this, [this](int column, const QString& text)
        {
            setText(column, text);
        }
    );

    connect(this, &FindDuplicatesAlbumItem::signalSetItemHidden,
            this, [this](bool hidden)
        {
            setHidden(hidden);
        }
    );

    if (d->album)
    {
        qlonglong refImage = d->album->title().toLongLong();

        if (!PrivacyAnalysisGate::mayAnalyze(refImage))
        {
            setHidden(true);
            return;
        }

        d->refImgInfo      = ItemInfo(refImage);
        setText(Column::REFERENCE_IMAGE, d->refImgInfo.name());
        setText(Column::REFERENCE_DATE,  d->refImgInfo.dateTime().toString(Qt::ISODate));

        const PAlbum* const physicalAlbum = AlbumManager::instance()->findPAlbum(d->refImgInfo.albumId());

        if (physicalAlbum)
        {
            setText(Column::REFERENCE_ALBUM, physicalAlbum->prettyUrl());
        }

        calculateInfos();
    }

    /// @note Parent can be null only with the unit-test.

    if (parent)
    {
        setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(parent->iconSize().width(),
                                                                        QIcon::Disabled), false);
    }
}

FindDuplicatesAlbumItem::~FindDuplicatesAlbumItem()
{
    // Cancel the task if it is running

    d->calcTask.cancel();
    d->calcTask.waitForFinished();

    delete d;
}

bool FindDuplicatesAlbumItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

QList<ItemInfo> FindDuplicatesAlbumItem::duplicatedItems()
{
    if (itemCount() <= 1)
    {
        return QList<ItemInfo>();
    }

    SearchXmlReader reader(d->album->query());
    reader.readToFirstField();

    QList<ItemInfo> toRemove;

    const QList<qlonglong>& list = reader.valueToLongLongList();
    const qlonglong refImage     = d->album->title().toLongLong();

    for (const qlonglong& imageId : std::as_const(list))
    {
        if ((imageId == refImage) ||
            !PrivacyAnalysisGate::mayAnalyze(imageId))
        {
            continue;
        }

        toRemove.append(ItemInfo(imageId));
    }

    return toRemove;
}

void FindDuplicatesAlbumItem::calculateInfos(const QList<qlonglong>& deletedImages)
{
    d->calcTask = QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                    &FindDuplicatesAlbumItem::calculateInfosMultithreaded, this,
                                    deletedImages

#else

                                    this, &FindDuplicatesAlbumItem::calculateInfosMultithreaded,
                                    deletedImages

#endif

    );
}

void FindDuplicatesAlbumItem::waitForCalculate()
{
    d->calcTask.waitForFinished();
}

int FindDuplicatesAlbumItem::itemCount() const
{
    QMutexLocker locker(&d->mutex);

    return d->itemCount;
}

void FindDuplicatesAlbumItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()  / 2) - (pix.width()  / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);

    QIcon icon = QIcon(pixmap);

    // We make sure the preview icon stays the same regardless of the role

    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(Column::REFERENCE_IMAGE, icon);

    d->hasThumb = hasThumb;
}

SAlbum* FindDuplicatesAlbumItem::album() const
{
    QMutexLocker locker(&d->mutex);

    return d->album;
}

QUrl FindDuplicatesAlbumItem::refUrl() const
{
    QMutexLocker locker(&d->mutex);

    return PrivacyAnalysisGate::mayAnalyze(d->refImgInfo.id())
         ? d->refImgInfo.fileUrl()
         : QUrl();
}

bool FindDuplicatesAlbumItem::operator<(const QTreeWidgetItem& other) const
{
    int result = 0;
    int column = treeWidget()->sortColumn();

    if      (column == Column::AVG_SIMILARITY)
    {
        result = ((text(column).toDouble() < other.text(column).toDouble()) ? -1 : 0);
    }
    else if (column == Column::RESULT_COUNT)
    {
        result = ((text(column).toInt() < other.text(column).toInt()) ? -1 : 0);
    }
    else
    {
        result = ItemSortCollator::instance()->albumCompare(text(column),
                                                            other.text(column),
                                                            Qt::CaseSensitive, true);
    }

    return (result < 0);
}

void FindDuplicatesAlbumItem::calculateInfosMultithreaded(const QList<qlonglong>& deletedImages)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name
    double avgSim = 0.0;

    QMutexLocker locker(&d->mutex);
    {
        if (!d->album || d->calcTask.isCanceled())
        {
            return;
        }

        qlonglong refImage = d->album->title().toLongLong();

        if (!PrivacyAnalysisGate::mayAnalyze(refImage))
        {
            d->itemCount = 0;
            Q_EMIT signalSetItemHidden(true);
            return;
        }

        SearchXmlReader reader(d->album->query());
        reader.readToFirstField();

        // Get the defined image ids.

        const QList<qlonglong>& list = reader.valueToLongLongList();

        // Only images that are not removed/obsolete should be shown.

        QList<qlonglong> filteredList;

        for (const qlonglong& imageId : std::as_const(list))
        {
            if (d->calcTask.isCanceled())
            {
                return;
            }

            ItemInfo info(imageId);

            // If image is not deleted in this moment and was also not removed before.

            if (!deletedImages.contains(imageId) && !info.isRemoved() &&
                PrivacyAnalysisGate::mayAnalyze(imageId))
            {
                filteredList << imageId;

                if (imageId != refImage)
                {
                    avgSim += info.similarityTo(refImage);
                }
            }
        }

        d->itemCount = filteredList.count();

        if (d->itemCount > 1)
        {
            avgSim /= d->itemCount - (filteredList.contains(refImage) ? 1 : 0);
        }
    }

    // Emit signals to update UI in the main thread

    Q_EMIT signalUpdateItemText(Column::RESULT_COUNT, QString::number(d->itemCount));
    Q_EMIT signalUpdateItemText(Column::AVG_SIMILARITY, QString::number(static_cast<int>(avgSim * 100)));

    if (d->itemCount <= 1)
    {
        Q_EMIT signalSetItemHidden(true);
    }
}

} // namespace Digikam

#include "moc_findduplicatesalbumitem.cpp"
