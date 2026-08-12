/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display a list of items
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ditemslist_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DItemsListViewItem::Private
{
public:

    Private() = default;

public:

    bool                        hasThumb = false;       ///< True if thumbnails is a real photo thumbs

    int                         rating   = -1;          ///< Image Rating from host.
    QString                     comments;               ///< Image comments from host.
    QStringList                 tags;                   ///< List of keywords from host.
    QUrl                        url;                    ///< Image url provided by host.
    QPixmap                     thumb;                  ///< Image thumbnail.
    DItemsListView*             view     = nullptr;
    State                       state    = Waiting;
};

DItemsListViewItem::DItemsListViewItem(DItemsListView* const view, const QUrl& url)
    : QTreeWidgetItem(view),
      d              (new Private)
{
    setUrl(url);
    setRating(-1);
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);

    d->view      = view;
    int iconSize = d->view->iconSize().width();
    setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(iconSize, iconSize, QIcon::Disabled), false);
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Creating new ImageListViewItem with url " << d->url
                                 << " for list view " << d->view;
*/
}

DItemsListViewItem::~DItemsListViewItem()
{
    delete d;
}

bool DItemsListViewItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

void DItemsListViewItem::updateInformation()
{
    if (d->view->iface())
    {
        DItemInfo info(d->view->iface()->itemInfo(d->url));

        setComments(info.comment());
        setTags(info.keywords());
        setRating(info.rating());
    }
}

void DItemsListViewItem::setUrl(const QUrl& url)
{
    d->url = url;
    setText(DItemsListView::Filename, d->url.fileName());
}

QUrl DItemsListViewItem::url() const
{
    return d->url;
}

void DItemsListViewItem::setComments(const QString& comments)
{
    d->comments = comments;
}

QString DItemsListViewItem::comments() const
{
    return d->comments;
}

void DItemsListViewItem::setTags(const QStringList& tags)
{
    d->tags = tags;
}

QStringList DItemsListViewItem::tags() const
{
    return d->tags;
}

void DItemsListViewItem::setRating(int rating)
{
    d->rating = rating;
}

int DItemsListViewItem::rating() const
{
    return d->rating;
}

void DItemsListViewItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);

    // We make sure the preview icon stays the same regardless of the role.

    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
    setIcon(DItemsListView::Thumbnail, icon);
}

void DItemsListViewItem::setThumb(const QPixmap& pix, bool hasThumb)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Received new thumbnail for url " << d->url
                                 << ". My view is " << d->view;
*/
    if (!d->view)
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "This item do not have a tree view. "
                                        << "This should never happen!";
        return;
    }

    int iconSize = qMax<int>(d->view->iconSize().width(), d->view->iconSize().height());
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()  / 2) - (pix.width()  / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);
    d->thumb     = pixmap;
    setPixmap(d->thumb);

    d->hasThumb  = hasThumb;
}

void DItemsListViewItem::setProgressAnimation(const QPixmap& pix)
{
    QPixmap overlay = d->thumb;
    QPixmap mask(overlay.size());
    mask.fill(QColor(128, 128, 128, 192));
    QPainter p(&overlay);
    p.drawPixmap(0, 0, mask);
    p.drawPixmap((overlay.width()  / 2) - (pix.width()  / 2),
                 (overlay.height() / 2) - (pix.height() / 2), pix);
    setPixmap(overlay);
}

void DItemsListViewItem::setProcessedIcon(const QIcon& icon)
{
    setIcon(DItemsListView::Filename, icon);

    // reset thumbnail back to no animation pix.

    setPixmap(d->thumb);
}

void DItemsListViewItem::setState(State state)
{
    d->state = state;
}

DItemsListViewItem::State DItemsListViewItem::state() const
{
    return d->state;
}

DItemsListView* DItemsListViewItem::view() const
{
    return d->view;
}

bool DItemsListViewItem::operator<(const QTreeWidgetItem& other) const
{
    if (d->view->isLessThanHandler())
    {
        return d->view->isLessThanHandler()(this, other);
    }

    return QTreeWidgetItem::operator<(other);
}

} // namespace Digikam
