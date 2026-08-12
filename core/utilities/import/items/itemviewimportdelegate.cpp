/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-08
 * Description : Item delegate for import interface items.
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemviewimportdelegate_p.h"

// Qt includes

#include <QCache>
#include <QPainter>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "thememanager.h"
#include "itemscanner.h"
#include "itempropertiestab.h"
#include "camiteminfo.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"

namespace Digikam
{

ItemViewImportDelegatePrivate::ItemViewImportDelegatePrivate()
{
    makeStarPolygon();
}

void ItemViewImportDelegatePrivate::init(ItemViewImportDelegate* const _q, QWidget* const _widget)
{
    q             = _q;
    displayWidget = _widget;

    q->connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ItemViewImportDelegatePrivate::clearRects()
{
    gridSize   = QSize(0, 0);
    rect       = QRect(0, 0, 0, 0);
    ratingRect = QRect(0, 0, 0, 0);
}

void ItemViewImportDelegatePrivate::makeStarPolygon()
{
    // Pre-computed star polygon for a 15x15 pixmap.

    starPolygon     = RatingWidget::starPolygon();
    starPolygonSize = QSize(15, 15);
}

// ---- ItemViewImportDelegate -----------------------------------------------

ItemViewImportDelegate::ItemViewImportDelegate(QWidget* const parent)
    : DItemDelegate(parent),
      d_ptr        (new ItemViewImportDelegatePrivate)
{
    d_ptr->init(this, parent);
}

ItemViewImportDelegate::ItemViewImportDelegate(ItemViewImportDelegatePrivate& dd, QWidget* const parent)
    : DItemDelegate(parent),
      d_ptr        (&dd)
{
    d_ptr->init(this, parent);
}

ItemViewImportDelegate::~ItemViewImportDelegate()
{
    Q_D(ItemViewImportDelegate);

    removeAllOverlays();
    delete d;
}

ThumbnailSize ItemViewImportDelegate::thumbnailSize() const
{
    Q_D(const ItemViewImportDelegate);

    return d->thumbSize;
}

double ItemViewImportDelegate::displayRatio() const
{
    Q_D(const ItemViewImportDelegate);

    return d->displayWidget->devicePixelRatio();
}

void ItemViewImportDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewImportDelegate);

    if (d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewImportDelegate::setSpacing(int _spacing)
{
    Q_D(ItemViewImportDelegate);

    if (d->spacing == _spacing)
    {
        return;
    }

    d->spacing = _spacing;
    invalidatePaintingCache();
}

int ItemViewImportDelegate::spacing() const
{
    Q_D(const ItemViewImportDelegate);

    return d->spacing;
}

QRect ItemViewImportDelegate::rect() const
{
    Q_D(const ItemViewImportDelegate);

    return d->rect;
}

QRect ItemViewImportDelegate::pixmapRect() const
{
    return QRect();
}

QRect ItemViewImportDelegate::imageInformationRect() const
{
    return QRect();
}

QRect ItemViewImportDelegate::ratingRect() const
{
    Q_D(const ItemViewImportDelegate);

    return d->ratingRect;
}

void ItemViewImportDelegate::setRatingEdited(const QModelIndex& index)
{
    Q_D(ItemViewImportDelegate);

    d->editingRating = index;
}

QSize ItemViewImportDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ItemViewImportDelegate);

    return d->rect.size();
}

QSize ItemViewImportDelegate::gridSize() const
{
    Q_D(const ItemViewImportDelegate);

    return d->gridSize;
}

bool ItemViewImportDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ItemViewImportDelegate::acceptsActivation(const QPoint&, const QRect& visualRect,
                                               const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ItemViewImportDelegate::asDelegate()
{
    return this;
}

void ItemViewImportDelegate::overlayDestroyed(QObject* o)
{
    ItemDelegateOverlayContainer::overlayDestroyed(o);
}

void ItemViewImportDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection AbstractImportItemDelegate -> ItemViewImportDelegate -> ItemDelegateOverlayContainer

    ItemDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ItemViewImportDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewImportDelegate);

    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewImportDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewImportDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewImportDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewImportDelegate);

    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();

    // cppcheck-suppress knownConditionTrueFalse
    if (oldGridSize != d->gridSize)
    {
        Q_EMIT gridSizeChanged(d->gridSize);
/*
        Q_EMIT sizeHintChanged(QModelIndex());
*/
    }

    Q_EMIT visualChange();
}

QRect ItemViewImportDelegate::drawThumbnail(QPainter* p,
                                            const QRect& thumbRect,
                                            const QPixmap& background,
                                            const QPixmap& thumbnail) const
{
    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
    {
        return QRect();
    }

    QRect r           = thumbRect;
    double dpr        =  thumbnail.devicePixelRatio();
    int thumbW        = qRound((double)thumbnail.width()  / dpr);
    int thumbH        = qRound((double)thumbnail.height() / dpr);

    QRect actualPixmapRect(r.x() + (r.width()  - thumbW) / 2,
                           r.y() + (r.height() - thumbH) / 2,
                           thumbW, thumbH);

    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());

    p->drawPixmap(actualPixmapRect.x() - 3,
                  actualPixmapRect.y() - 3, borderPix);

    p->drawPixmap(r.x() + (r.width()  - thumbW) / 2,
                  r.y() + (r.height() - thumbH) / 2,
                  thumbW, thumbH, thumbnail);

    return actualPixmapRect;
}

void ItemViewImportDelegate::drawRating(QPainter* _painter,
                                        const QModelIndex& _index,
                                        const QRect& _ratingRect,
                                        int _rating,
                                        bool _isSelected) const
{
    Q_D(const ItemViewImportDelegate);

    if (d->editingRating != _index)
    {
        _painter->drawPixmap(_ratingRect, ratingPixmap(_rating, _isSelected));
    }
}

void ItemViewImportDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewImportDelegate);

    p->setFont(d->fontReg);
    p->drawText(nameRect, Qt::AlignCenter, name);//squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewImportDelegate::drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewImportDelegate);

    p->setFont(d->fontXtra);
    QString str = asUserDateTime(date);
    str         = i18nc("date of image creation", "created: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, str);//squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewImportDelegate::drawImageFormat(QPainter* p, const QRect& r, const QString& mime) const
{
    Q_D(const ItemViewImportDelegate);

    if (!mime.isEmpty() && !r.isNull())
    {
        QString type = mime.split(QLatin1Char('/')).at(1);
        type         = ItemScanner::formatToString(type);

        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        QRect bRect = p->boundingRect(r, Qt::AlignBottom | Qt::AlignHCenter, type.toUpper());
        bRect.adjust(1, 1, -1, -1);
        bRect.translate(0, 1);

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, Qt::AlignBottom | Qt::AlignHCenter, type.toUpper());

        p->restore();
    }
}

void ItemViewImportDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewImportDelegate);

    if (dims.isValid())
    {
        p->setFont(d->fontXtra);
        QString mpixels, resolution;
        mpixels = QLocale().toString(dims.width() * dims.height() / 1000000.0, 'f', 1);

        if (dims.isValid())
        {
            resolution = i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                               dims.width(), dims.height(), mpixels);
        }
        else
        {
            resolution = i18nc("unknown image resolution", "Unknown");
        }

        p->drawText(dimsRect, Qt::AlignCenter, resolution);
    }
}

void ItemViewImportDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ItemViewImportDelegate);

    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, ItemPropertiesTab::humanReadableBytesCount(bytes));
}

void ItemViewImportDelegate::drawTags(QPainter* p,
                                      const QRect& r,
                                      const QString& tagsString,
                                      bool isSelected) const
{
    Q_D(const ItemViewImportDelegate);

    p->setFont(d->fontCom);
    p->setPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                         : qApp->palette().color(QPalette::Link));

    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tagsString));
}

void ItemViewImportDelegate::drawPickLabelIcon(QPainter* p, const QRect& r, int pickId) const
{
    // Draw Pick Label icon

    if (pickId != NoPickLabel)
    {
        QIcon icon;

        if      (pickId == RejectedLabel)
        {
            icon = QIcon::fromTheme(QLatin1String("flag-red"));
        }
        else if (pickId == PendingLabel)
        {
            icon = QIcon::fromTheme(QLatin1String("flag-yellow"));
        }
        else if (pickId == AcceptedLabel)
        {
            icon = QIcon::fromTheme(QLatin1String("flag-green"));
        }

        icon.paint(p, r);
    }
}

void ItemViewImportDelegate::drawColorLabelLine(QPainter* p, const QRect& pixRect, int colorId) const
{
    if (colorId > NoColorLabel)
    {
        // This draw a line under the thumbnail.

        QLinearGradient gradient(pixRect.x(), 0, pixRect.x() + pixRect.width(), 1);
        QColor color(ColorLabelWidget::labelColor((ColorLabel)colorId));

        color.setAlphaF(0.2);
        gradient.setColorAt(0.0, color);
        gradient.setColorAt(1.0, color);

        color.setAlphaF(1.0);
        gradient.setColorAt(0.3, color);
        gradient.setColorAt(0.7, color);

        p->setPen(QPen(QBrush(gradient), 5));

        p->drawLine(pixRect.x(),
                    pixRect.y() + pixRect.height() - 3,
                    pixRect.x() + pixRect.width(),
                    pixRect.y() + pixRect.height() - 3);
    }
}

void ItemViewImportDelegate::drawGeolocationIndicator(QPainter* p, const QRect& r) const
{
    if (!r.isNull())
    {
        QIcon icon(QIcon::fromTheme(QLatin1String("globe")).pixmap(r.size()));
        QBrush brush = p->brush();
        p->setOpacity(0.50);
        p->setPen(QPen(p->background().color()));
        p->setBrush(QBrush(p->background().color()));
        p->drawEllipse(r);
        p->setBrush(brush);
        p->setOpacity(1.0);
        icon.paint(p, r);
    }
}

void ItemViewImportDelegate::drawDownloadIndicator(QPainter* p, const QRect& r, int itemType) const
{
    QIcon icon;

    if (itemType == CamItemInfo::DownloadUnknown)
    {
        icon = QIcon::fromTheme(QLatin1String("dialog-information"));
    }

    if (itemType == CamItemInfo::DownloadedNo) // TODO: CamItemInfo::NewPicture
    {
        icon = QIcon::fromTheme(QLatin1String("folder-favorites"));
    }

    if (itemType == CamItemInfo::DownloadedYes)
    {
        icon = QIcon::fromTheme(QLatin1String("dialog-ok-apply"));
    }

    qreal op = p->opacity();
    p->setOpacity(0.5);
    icon.paint(p, r);
    p->setOpacity(op);
}

void ItemViewImportDelegate::drawLockIndicator(QPainter* p, const QRect& r, int lockStatus) const
{
    QIcon icon;

    if (lockStatus == 1)
    {
        return; // draw lock only when image is locked
/*
        icon = QIcon::fromTheme(QLatin1String("object-unlocked"));
*/
    }

    if (lockStatus == 0)
    {
        icon = QIcon::fromTheme(QLatin1String("object-locked"));
    }

    qreal op = p->opacity();
    p->setOpacity(0.5);
    icon.paint(p, r);
    p->setOpacity(op);
}

void ItemViewImportDelegate::drawFocusRect(QPainter* p,
                                           const QStyleOptionViewItem& option,
                                           bool isSelected) const
{
    Q_D(const ItemViewImportDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                                  : qApp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width() - 3, d->rect.height() - 3);
    }
}

void ItemViewImportDelegate::drawGroupIndicator(QPainter* p,
                                                const QRect& r,
                                                int numberOfGroupedImages,
                                                bool open) const
{
    if (numberOfGroupedImages)
    {
        QIcon icon;

        if (open)
        {
            icon = QIcon::fromTheme(QLatin1String("document-import"));
        }
        else
        {
            icon = QIcon::fromTheme(QLatin1String("document-multiple"));
        }

        qreal op = p->opacity();
        p->setOpacity(0.5);
        icon.paint(p, r);
        p->setOpacity(op);

        QString text = QString::number(numberOfGroupedImages);
        p->drawText(r, Qt::AlignCenter, text);
    }
}

void ItemViewImportDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewImportDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width() - 3, d->rect.height() - 3);
    }
}

void ItemViewImportDelegate::prepareFonts()
{
    Q_D(ItemViewImportDelegate);

    d->fontReg  = d->font;
    d->fontCom  = d->font;
    d->fontXtra = d->font;
    d->fontCom.setItalic(true);

    int fnSz    = d->fontReg.pointSize();

    if (fnSz > 0)
    {
        d->fontCom.setPointSize(fnSz-1);
        d->fontXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fontReg.pixelSize();
        d->fontCom.setPixelSize(fnSz-1);
        d->fontXtra.setPixelSize(fnSz-2);
    }
}

void ItemViewImportDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ItemViewImportDelegate);

    QFontMetrics fm(d->fontReg);
    d->oneRowRegRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       QLatin1String("XXXXXXXXX"));
    fm = QFontMetrics(d->fontCom);
    d->oneRowComRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       QLatin1String("XXXXXXXXX"));
    fm = QFontMetrics(d->fontXtra);
    d->oneRowXtraRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                        Qt::AlignTop | Qt::AlignHCenter,
                                        QLatin1String("XXXXXXXXX"));
}

void ItemViewImportDelegate::prepareBackground()
{
    Q_D(ItemViewImportDelegate);

    if (!d->rect.isValid())
    {
        d->regPixmap = QPixmap();
        d->selPixmap = QPixmap();
    }
    else
    {
        d->regPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->regPixmap.fill(qApp->palette().color(QPalette::Base));
        QPainter p1(&d->regPixmap);
        p1.setPen(qApp->palette().color(QPalette::Midlight));
        p1.drawRect(0, 0, d->rect.width() - 1, d->rect.height() - 1);

        d->selPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->selPixmap.fill(qApp->palette().color(QPalette::Highlight));
        QPainter p2(&d->selPixmap);
        p2.setPen(qApp->palette().color(QPalette::Midlight));
        p2.drawRect(0, 0, d->rect.width() - 1, d->rect.height() - 1);
    }
}

void ItemViewImportDelegate::prepareRatingPixmaps(bool composeOverBackground)
{
    /// Please call this method after prepareBackground() and when d->ratingPixmap is set

    Q_D(ItemViewImportDelegate);

    if (!d->ratingRect.isValid())
    {
        return;
    }

    // We use antialiasing and want to pre-render the pixmaps.
    // So we need the background at the time of painting,
    // and the background may be a gradient, and will be different for selected items.
    // This makes 5*2 (small) pixmaps.

    for (int sel = 0 ; sel < 2 ; ++sel)
    {
        QPixmap basePix;

        if (composeOverBackground)
        {
            // do this once for regular, once for selected backgrounds

            if (sel)
            {
                basePix = d->selPixmap.copy(d->ratingRect);
            }
            else
            {
                basePix = d->regPixmap.copy(d->ratingRect);
            }
        }
        else
        {
            basePix = QPixmap(d->ratingRect.size());
            basePix.fill(Qt::transparent);
        }

        double dpr = displayRatio();
        basePix    = basePix.scaled(d->ratingRect.size() * dpr);
        basePix.setDevicePixelRatio(dpr);

        for (int rating = 1 ; rating <= 5 ; ++rating)
        {
            // we store first the 5 regular, then the 5 selected pixmaps, for simplicity

            int index = (sel * 5 + rating) - 1;

            // copy background

            d->ratingPixmaps[index] = basePix;

            // open a painter

            QPainter painter(&d->ratingPixmaps[index]);

            // use antialiasing

            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setBrush(qApp->palette().color(QPalette::Link));
            QPen pen(qApp->palette().color(QPalette::Text));

            // set a pen which joins the lines at a filled angle

            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);

            // move painter while drawing polygons

            painter.translate( lround((d->ratingRect.width() - d->margin - rating *
                                      (d->starPolygonSize.width() + 1)) / 2.0) + 2, 1 );

            for (int s = 0 ; s < rating ; ++s)
            {
                painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                painter.translate(d->starPolygonSize.width() + 1, 0);
            }
        }
    }
}

QPixmap ItemViewImportDelegate::ratingPixmap(int rating, bool selected) const
{
    Q_D(const ItemViewImportDelegate);

    if ((rating < 1) || (rating > 5))
    {
/*
        QPixmap pix;

        if (selected)
        {
            pix = d->selPixmap.copy(d->ratingRect);
        }
        else
        {
            pix = d->regPixmap.copy(d->ratingRect);
        }

        return pix;
*/
        return QPixmap();
    }

    --rating;

    if (selected)
    {
        return d->ratingPixmaps.at(5 + rating);
    }
    else
    {
        return d->ratingPixmaps.at(rating);
    }
}

} // namespace Digikam

#include "moc_itemviewimportdelegate.cpp"
