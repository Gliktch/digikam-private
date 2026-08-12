/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 02-08-2013
 * Description : Thumbnail bar for Showfoto
 *
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotothumbnailbar.h"

// Qt includes

#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "showfotosettings.h"
#include "showfotodelegate.h"
#include "showfotofiltermodel.h"
#include "itemviewtooltip.h"
#include "showfototooltipfiller.h"
#include "itemselectionoverlay.h"
#include "showfotokineticscroller.h"
#include "showfotocoordinatesoverlay.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoThumbnailBar::Private
{
public:

    Private() = default;

    Qt::ScrollBarPolicy              scrollPolicy     = Qt::ScrollBarAlwaysOn;
    NoDuplicatesShowfotoFilterModel* duplicatesFilter = nullptr;
    ShowfotoKineticScroller*         kScroller        = nullptr;
};

ShowfotoThumbnailBar::ShowfotoThumbnailBar(QWidget* const parent)
    : ShowfotoCategorizedView(parent),
      d                      (new Private)
{
    setItemDelegate(new ShowfotoThumbnailDelegate(this));
    setSpacing(3);
    setUsePointingHandCursor(false);
    setScrollStepGranularity(3);
    setScrollCurrentToCenter(ShowfotoSettings::instance()->getItemCenter());
    setScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    // Disable QListView::Batched optimization
    // for the thumbnail bar, see bug #468593

    setLayoutMode(QListView::SinglePass);

    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    this->slotSetupChanged();

    d->kScroller = new ShowfotoKineticScroller();
    d->kScroller->enableKineticScrollFor(this);
}

ShowfotoThumbnailBar::~ShowfotoThumbnailBar()
{
    delete d;
}

int ShowfotoThumbnailBar::thumbnailIndexForUrl(const QUrl& url) const
{
    int index = 0;

    for (int i = 0 ; i < showfotoItemInfos().size() ; ++i)
    {
        QUrl iurl = showfotoItemInfos().at(i).url;

        if (iurl.matches(url, QUrl::None))
        {
            index = i + 1;
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Thumb index for" << url << ":" << index;

    return index;
}

void ShowfotoThumbnailBar::installOverlays()
{
    addOverlay(new ShowfotoCoordinatesOverlay(this));
}

void ShowfotoThumbnailBar::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if ((area == Qt::LeftDockWidgetArea) || (area == Qt::RightDockWidgetArea))
    {
        setFlow(TopToBottom);
        d->kScroller->setScrollFlow(TopToBottom);
    }
    else
    {
        setFlow(LeftToRight);
        d->kScroller->setScrollFlow(LeftToRight);
    }

    scrollTo(currentIndex());
}

void ShowfotoThumbnailBar::setScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
    if (policy == Qt::ScrollBarAsNeeded)
    {
        // Delegate resizing will cause endless relayouting, see bug #228807

        qCDebug(DIGIKAM_GENERAL_LOG) << "The Qt::ScrollBarAsNeeded policy is not supported by ShowfotoThumbnailBar";
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

void ShowfotoThumbnailBar::setFlow(QListView::Flow flow)
{
    setWrapping(false);

    ShowfotoCategorizedView::setFlow(flow);

    ShowfotoThumbnailDelegate* const del = static_cast<ShowfotoThumbnailDelegate*>(delegate());
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

bool ShowfotoThumbnailBar::event(QEvent* e)
{
    // reset widget max/min sizes

    if ((e->type() == QEvent::StyleChange) || (e->type() == QEvent::Show))
    {
        setFlow(flow());
    }

    return ShowfotoCategorizedView::event(e);
}

QModelIndex ShowfotoThumbnailBar::nextIndex(const QModelIndex& index) const
{
    return showfotoFilterModel()->index(index.row() + 1, 0);
}

QModelIndex ShowfotoThumbnailBar::previousIndex(const QModelIndex& index) const
{
    return showfotoFilterModel()->index(index.row() - 1, 0);
}

QModelIndex ShowfotoThumbnailBar::firstIndex() const
{
    return showfotoFilterModel()->index(0, 0);
}

QModelIndex ShowfotoThumbnailBar::lastIndex() const
{
    return showfotoFilterModel()->index(showfotoFilterModel()->rowCount() - 1, 0);
}

ShowfotoItemInfo ShowfotoThumbnailBar::findItemByUrl(const QUrl& url)
{
    ShowfotoItemInfoList lst = showfotoItemInfos();

    for (int i = 0 ; i < lst.size() ; ++i)
    {
        if (lst.at(i).url == url)
        {
            return lst.at(i);
        }
    }

    return ShowfotoItemInfo();
}

} // namespace ShowFoto

#include "moc_showfotothumbnailbar.cpp"
