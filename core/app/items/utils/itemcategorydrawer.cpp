/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-20
 * Description : Qt model-view for items - category drawer
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemcategorydrawer.h"

// Qt includes

#include <QPainter>
#include <QApplication>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "itemalbummodel.h"
#include "itemcategorizedview.h"
#include "itemdelegate.h"
#include "itemfiltermodel.h"
#include "itemmodel.h"
#include "itemscanner.h"
#include "searchfolderview.h"
#include "facetagsiface.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemCategoryDrawer::Private
{
public:

    Private() = default;

public:

    QFont                font;
    QRect                rect;
    QPixmap              pixmap;
    int                  lowerSpacing   = 0;
    ItemCategorizedView* view           = nullptr;
};

ItemCategoryDrawer::ItemCategoryDrawer(ItemCategorizedView* const parent)
    : DCategoryDrawer(nullptr),
      d              (new Private)
{
    d->view = parent;
}

ItemCategoryDrawer::~ItemCategoryDrawer()
{
    delete d;
}

int ItemCategoryDrawer::categoryHeight(const QModelIndex& /*index*/, const QStyleOption& /*option*/) const
{
    return (d->rect.height() + d->lowerSpacing);
}

int ItemCategoryDrawer::maximumHeight() const
{
    return (d->rect.height() + d->lowerSpacing);
}

void ItemCategoryDrawer::setLowerSpacing(int spacing)
{
    d->lowerSpacing = spacing;
}

void ItemCategoryDrawer::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    d->font = option.font;

    if (option.rect.width() != d->rect.width())
    {
        updateRectsAndPixmaps(option.rect.width());
    }
}

void ItemCategoryDrawer::invalidatePaintingCache()
{
    if (d->rect.isNull())
    {
        return;
    }

    updateRectsAndPixmaps(d->rect.width());
}

void ItemCategoryDrawer::drawCategory(const QModelIndex& index, int /*sortRole*/,
                                      const QStyleOption& option, QPainter* p) const
{
    if (option.rect.width() != d->rect.width())
    {
        const_cast<ItemCategoryDrawer*>(this)->updateRectsAndPixmaps(option.rect.width());
    }

    p->save();

    p->translate(option.rect.topLeft());

    ItemSortSettings::CategorizationMode mode = (ItemSortSettings::CategorizationMode)index.data(ItemFilterModel::CategorizationModeRole).toInt();

    p->drawPixmap(0, 0, d->pixmap);

    QFont fontBold(d->font);
    QFont fontNormal(d->font);
    fontBold.setBold(true);
    int fnSize = fontBold.pointSize();

    if (fnSize > 0)
    {
        fontBold.setPointSize(fnSize+2);
    }
    else
    {
        fnSize = fontBold.pixelSize();
        fontBold.setPixelSize(fnSize+2);
    }

    QString header;
    QString subLine;

    switch (mode)
    {
        case ItemSortSettings::NoCategories:
        {
            break;
        }

        case ItemSortSettings::OneCategory:
        {
            viewHeaderText(index, &header, &subLine);
            break;
        }

        case ItemSortSettings::CategoryByAlbum:
        {
            textForAlbum(index, &header, &subLine);
            break;
        }

        case ItemSortSettings::CategoryByFormat:
        {
            textForFormat(index, &header, &subLine);
            break;
        }

        case ItemSortSettings::CategoryByFaces:
        {
            textForFace(index, &header, &subLine);
            break;
        }

        case ItemSortSettings::CategoryByMonth:
        {
            textForMonth(index, &header, &subLine);
            break;
        }

        case ItemSortSettings::CategoryByDay:
        {
            textForDay(index, &header, &subLine);
            break;
        }
    }

    p->setPen(qApp->palette().color(QPalette::HighlightedText));
    p->setFont(fontBold);

    QRect tr;
    p->drawText(5, 5, d->rect.width(), d->rect.height(),
                Qt::AlignLeft | Qt::AlignTop,
                p->fontMetrics().elidedText(header, Qt::ElideRight, d->rect.width() - 10), &tr);

    int y = tr.height() + 2;

    p->setFont(fontNormal);

    p->drawText(5, y, d->rect.width(), d->rect.height() - y,
                Qt::AlignLeft | Qt::AlignVCenter,
                p->fontMetrics().elidedText(subLine, Qt::ElideRight, d->rect.width() - 10));

    p->restore();
}

void ItemCategoryDrawer::viewHeaderText(const QModelIndex& index, QString* header, QString* subLine) const
{
    ItemModel* const sourceModel = index.data(ItemModel::ItemModelPointerRole).value<ItemModel*>();

    if (!sourceModel)
    {
        return;
    }

    int count = d->view->categoryRange(index).height();

    // Add here further model subclasses in use with ItemCategoryDrawer.
    // Note you need a Q_OBJECT in the class's header for this to work.

    const ItemAlbumModel* const albumModel = qobject_cast<ItemAlbumModel*>(sourceModel);

    if (albumModel)
    {
        QList<Album*> albums = albumModel->currentAlbums();
        Album* album         = nullptr;

        if (albums.isEmpty())
        {
            return;
        }

        album = albums.first();

        if (!album)
        {
            return;
        }

        switch (album->type())
        {
            case Album::PHYSICAL:
            {
                textForPAlbum(static_cast<PAlbum*>(album), albumModel->isRecursingAlbums(), count, header, subLine);
                break;
            }

            case Album::TAG:
            {
                textForTAlbum(static_cast<TAlbum*>(album), albumModel->isRecursingTags(), count, header, subLine);
                break;
            }

            case Album::DATE:
            {
                textForDAlbum(static_cast<DAlbum*>(album), count, header, subLine);
                break;
            }

            case Album::SEARCH:
            {
                textForSAlbum(static_cast<SAlbum*>(album), count, header, subLine);
                break;
            }

            case Album::FACE:
            default:
            {
                break;
            }
        }
    }
}

void ItemCategoryDrawer::textForAlbum(const QModelIndex& index, QString* header, QString* subLine) const
{
    int albumId         = index.data(ItemFilterModel::CategoryAlbumIdRole).toInt();
    PAlbum* const album = AlbumManager::instance()->findPAlbum(albumId);
    int count           = d->view->categoryRange(index).height();
    textForPAlbum(album, false, count, header, subLine);
}

void ItemCategoryDrawer::textForFormat(const QModelIndex& index, QString* header, QString* subLine) const
{
    QString format = index.data(ItemFilterModel::CategoryFormatRole).toString();
    format         = ItemScanner::formatToString(format);
    *header        = format;
    int count      = d->view->categoryRange(index).height();
    *subLine       = i18np("1 Item", "%1 Items", count);
}

void ItemCategoryDrawer::textForFace(const QModelIndex& index, QString* header, QString* subLine) const
{
    *header    = index.data(ItemFilterModel::CategoryFaceRole).toString();
    int count  = d->view->categoryRange(index).height();
    *subLine   = i18np("1 Item", "%1 Items", count);
}

void ItemCategoryDrawer::textForMonth(const QModelIndex& index, QString* header, QString* subLine) const
{
    QDate date = index.data(ItemFilterModel::CategoryDateRole).toDate();
    *header    = date.isValid() ? QLocale().toString(date, QLatin1String("MMMM yyyy"))
                                : i18n("Invalid date");
    int count  = d->view->categoryRange(index).height();
    *subLine   = i18np("1 Item", "%1 Items", count);
}

void ItemCategoryDrawer::textForDay(const QModelIndex& index, QString* header, QString* subLine) const
{
    QDate date = index.data(ItemFilterModel::CategoryDateRole).toDate();
    *header    = date.isValid() ? QLocale().toString(date, QLatin1String("dd (dddd) MMMM yyyy"))
                                : i18n("Invalid date");
    int count  = d->view->categoryRange(index).height();
    *subLine   = i18np("1 Item", "%1 Items", count);
}

void ItemCategoryDrawer::textForPAlbum(PAlbum* album, bool recursive, int count, QString* header, QString* subLine) const
{
    Q_UNUSED(recursive);

    if (!album)
    {
        return;
    }

    QDate date    = album->date();

    QLocale tmpLocale;

    // day of month with two digits

    QString day   = tmpLocale.toString(date, QLatin1String("dd"));

    // short form of the month

    QString month = tmpLocale.toString(date, QLatin1String("MMM"));

    // long form of the year

    QString year  = tmpLocale.toString(date, QLatin1String("yyyy"));

    *subLine      = i18ncp("%1: day of month with two digits, %2: short month name, %3: year",
                           "Album Date: %2 %3 %4 - 1 Item", "Album Date: %2 %3 %4 - %1 Items",
                           count, day, month, year);

    if (!album->caption().isEmpty())
    {
        QString caption = album->caption();
        *subLine       += QLatin1String(" - ") + caption.replace(QLatin1Char('\n'), QLatin1Char(' '));
    }

    *header = album->prettyUrl();
}

void ItemCategoryDrawer::textForTAlbum(TAlbum* talbum, bool recursive, int count, QString* header,
                                       QString* subLine) const
{
    *header = talbum->title();

    if (recursive && talbum->firstChild())
    {
        int n = 0;

        for (AlbumIterator it(talbum) ; it.current() ; ++it)
        {
            n++;
        }

        QString firstPart = i18ncp("%2: a tag title; %3: number of subtags",
                                   "%2 including 1 subtag", "%2 including %1 subtags",
                                   n, talbum->tagPath(false));

        *subLine = i18ncp("%2: the previous string (e.g. 'Foo including 7 subtags'); %1: number of items in tag",
                          "%2 - 1 Item", "%2 - %1 Items",
                          count, firstPart);
    }
    else
    {
        *subLine = i18np("%2 - 1 Item", "%2 - %1 Items", count, talbum->tagPath(false));
    }
}

void ItemCategoryDrawer::textForSAlbum(SAlbum* salbum, int count, QString* header, QString* subLine) const
{
    QString title = salbum->displayTitle();

    *header = title;

    if      (salbum->isNormalSearch())
    {
        *subLine = i18np("Keyword Search - 1 Item", "Keyword Search - %1 Items", count);
    }
    else if (salbum->isAdvancedSearch())
    {
        *subLine = i18np("Advanced Search - 1 Item", "Advanced Search - %1 Items", count);
    }
    else
    {
        *subLine = i18np("1 Item", "%1 Items", count);
    }
}

void ItemCategoryDrawer::textForDAlbum(DAlbum* album, int count, QString* header, QString* subLine) const
{
    QString year = QLocale().toString(album->date(), QLatin1String("yyyy"));

    if (album->range() == DAlbum::Month)
    {
        *header = i18nc("Month String - Year String", "%1 %2",
                        QLocale().standaloneMonthName(album->date().month(), QLocale::LongFormat), year);
    }
    else
    {
        *header = year;
    }

    *subLine = i18np("1 Item", "%1 Items", count);
}

void ItemCategoryDrawer::updateRectsAndPixmaps(int width)
{
    d->rect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------

    QFont fn(d->font);
    int fnSize = fn.pointSize();
    bool usePointSize;

    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize       = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, width,
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               QLatin1String("XXX"));
    d->rect.setHeight(tr.height());

    if (usePointSize)
    {
        fn.setPointSize(d->font.pointSize());
    }
    else
    {
        fn.setPixelSize(d->font.pixelSize());
    }

    fn.setBold(false);
    fm = QFontMetrics(fn);
    tr = fm.boundingRect(0, 0, width,
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         QLatin1String("XXX"));

    d->rect.setHeight(d->rect.height() + tr.height() + 10);
    d->rect.setWidth(width);

    d->pixmap = QPixmap(d->rect.width(), d->rect.height());
    d->pixmap.fill(qApp->palette().color(QPalette::Highlight));
}

} // namespace Digikam

#include "moc_itemcategorydrawer.cpp"
