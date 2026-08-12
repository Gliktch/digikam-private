/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-20-07
 * Description : Thumbnail bar for import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importthumbnailbar.h"

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "importsettings.h"
#include "importdelegate.h"
#include "importfiltermodel.h"
#include "importoverlays.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportThumbnailBar::Private
{
public:

    Private() = default;

    Qt::ScrollBarPolicy            scrollPolicy     = Qt::ScrollBarAlwaysOn;
    NoDuplicatesImportFilterModel* duplicatesFilter = nullptr;
};

ImportThumbnailBar::ImportThumbnailBar(QWidget* const parent)
    : ImportCategorizedView(parent),
      d                    (new Private)
{
    setItemDelegate(new ImportThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(5);
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    // Disable QListView::Batched optimization
    // for the thumbnail bar, see bug #468593

    setLayoutMode(QListView::SinglePass);

    setScrollCurrentToCenter(ApplicationSettings::instance()->getScrollItemToCenter());
    setToolTipEnabled(ImportSettings::instance()->showToolTipsIsValid());

    // --- NOTE: use dynamic binding as slotSetupChanged() is a virtual method which can be re-implemented in derived classes.

    connect(ImportSettings::instance(), &ImportSettings::setupChanged,
            this, &ImportThumbnailBar::slotSetupChanged);

    this->slotSetupChanged();

    // ---

    setFlow(LeftToRight);
}

ImportThumbnailBar::~ImportThumbnailBar()
{
    delete d;
}

void ImportThumbnailBar::setModelsFiltered(ImportItemModel* model, ImportSortFilterModel* filterModel)
{
    if (!d->duplicatesFilter)
    {
        d->duplicatesFilter = new NoDuplicatesImportFilterModel(this);
    }

    d->duplicatesFilter->setSourceFilterModel(filterModel);
    ImportCategorizedView::setModels(model, d->duplicatesFilter);
}

void ImportThumbnailBar::installOverlays()
{
    ImportRatingOverlay* const ratingOverlay = new ImportRatingOverlay(this);
    addOverlay(ratingOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    addOverlay(new ImportLockOverlay(this));
    addOverlay(new ImportDownloadOverlay(this));
    addOverlay(new ImportCoordinatesOverlay(this));
}

void ImportThumbnailBar::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if ((area == Qt::LeftDockWidgetArea) || (area == Qt::RightDockWidgetArea))
    {
        if (flow() != TopToBottom)
        {
            setFlow(TopToBottom);
        }
    }
    else
    {
        if (flow() != LeftToRight)
        {
            setFlow(LeftToRight);
        }
    }

    scrollTo(currentIndex());
}

void ImportThumbnailBar::setScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    if (policy == Qt::ScrollBarAsNeeded)
    {
        // Delegate resizing will cause endless relayouting, see bug #228807

        qCDebug(DIGIKAM_IMPORTUI_LOG) << "The Qt::ScrollBarAsNeeded policy is not supported by ImportThumbnailBar";
    }

    d->scrollPolicy = policy;

    if (flow() == TopToBottom)
    {
        setVerticalScrollBarPolicy(d->scrollPolicy);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        setHorizontalScrollBarPolicy(d->scrollPolicy);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void ImportThumbnailBar::setFlow(QListView::Flow flow)
{
    setWrapping(false);

    ImportCategorizedView::setFlow(flow);

    ImportThumbnailDelegate* del = static_cast<ImportThumbnailDelegate*>(delegate());
    del->setFlow(flow);

    // Reset the minimum and maximum sizes.

    setMinimumSize(QSize(0, 0));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    // Adjust minimum and maximum width to thumbnail sizes.

    if (flow == TopToBottom)
    {
        int viewportFullWidgetOffset = size().width() - viewport()->size().width();
        setMinimumWidth(del->minimumSize() + viewportFullWidgetOffset);
        setMaximumWidth(del->maximumSize() + viewportFullWidgetOffset);
    }
    else
    {
        int viewportFullWidgetOffset = size().height() - viewport()->size().height();
        setMinimumHeight(del->minimumSize() + viewportFullWidgetOffset);
        setMaximumHeight(del->maximumSize() + viewportFullWidgetOffset);
    }

    setScrollBarPolicy(d->scrollPolicy);
}

void ImportThumbnailBar::slotSetupChanged()
{
    setScrollCurrentToCenter(ApplicationSettings::instance()->getScrollItemToCenter());
    setToolTipEnabled(ImportSettings::instance()->showToolTipsIsValid());
    setFont(ImportSettings::instance()->getIconViewFont());

    ImportCategorizedView::slotSetupChanged();
}

void ImportThumbnailBar::assignRating(const QList<QModelIndex>& indexes, int rating)
{
   QList<QModelIndex> mappedIndexes = importSortFilterModel()->mapListToSource(indexes);

   for (const QModelIndex& index : std::as_const(mappedIndexes))
   {
       if (index.isValid())
       {
            importItemModel()->camItemInfoRef(index).rating = rating;
       }
   }
}

bool ImportThumbnailBar::event(QEvent* e)
{
    // reset widget max/min sizes

    if ((e->type() == QEvent::StyleChange) || (e->type() == QEvent::Show))
    {
        setFlow(flow());
    }

    return ImportCategorizedView::event(e);
}

QModelIndex ImportThumbnailBar::nextIndex(const QModelIndex& index) const
{
    return importFilterModel()->index(index.row() + 1, 0);
}

QModelIndex ImportThumbnailBar::previousIndex(const QModelIndex& index) const
{
    return importFilterModel()->index(index.row() - 1, 0);
}

QModelIndex ImportThumbnailBar::firstIndex() const
{
    return importFilterModel()->index(0, 0);
}

QModelIndex ImportThumbnailBar::lastIndex() const
{
    return importFilterModel()->index(importFilterModel()->rowCount() - 1, 0);
}

} // namespace Digikam

#include "moc_importthumbnailbar.cpp"
