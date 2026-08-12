/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for itemss - the delegate
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemviewdelegate_p.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCache>
#include <QPainter>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itempropertiestab.h"
#include "thememanager.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"

namespace Digikam
{

ItemViewDelegatePrivate::ItemViewDelegatePrivate()
{
    makeStarPolygon();
}

void ItemViewDelegatePrivate::init(ItemViewDelegate* const _q, QWidget* const _widget)
{
    q             = _q;
    displayWidget = _widget;

    q->connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ItemViewDelegatePrivate::clearRects()
{
    gridSize        = QSize(0, 0);
    rect            = QRect(0, 0, 0, 0);
    ratingRect      = QRect(0, 0, 0, 0);
    groupedItemRect = QRect(0, 0, 0, 0);
}

void ItemViewDelegatePrivate::makeStarPolygon()
{
    // Pre-computed star polygon for a 15x15 pixmap.

    starPolygon     = RatingWidget::starPolygon();
    starPolygonSize = QSize(15, 15);
}

ItemViewDelegate::ItemViewDelegate(QWidget* const parent)
    : DItemDelegate(parent),
      d_ptr        (new ItemViewDelegatePrivate)
{
    d_ptr->init(this, parent);
}

ItemViewDelegate::ItemViewDelegate(ItemViewDelegatePrivate& dd, QWidget* const parent)
    : DItemDelegate(parent),
      d_ptr        (&dd)
{
    d_ptr->init(this, parent);
}

ItemViewDelegate::~ItemViewDelegate()
{
    Q_D(ItemViewDelegate);

    removeAllOverlays();

    delete d;
}

ThumbnailSize ItemViewDelegate::thumbnailSize() const
{
    Q_D(const ItemViewDelegate);

    return d->thumbSize;
}

double ItemViewDelegate::displayRatio() const
{
    Q_D(const ItemViewDelegate);

    return d->displayWidget->devicePixelRatio();
}

void ItemViewDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewDelegate);

    if (d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewDelegate::setSpacing(int spacing)
{
    Q_D(ItemViewDelegate);

    if (d->spacing == spacing)
    {
        return;
    }

    d->spacing = spacing;
    invalidatePaintingCache();
}

int ItemViewDelegate::spacing() const
{
    Q_D(const ItemViewDelegate);

    return d->spacing;
}

QRect ItemViewDelegate::rect() const
{
    Q_D(const ItemViewDelegate);

    return d->rect;
}

QRect ItemViewDelegate::pixmapRect() const
{
    return QRect();
}

QRect ItemViewDelegate::imageInformationRect() const
{
    return QRect();
}

QRect ItemViewDelegate::ratingRect() const
{
    Q_D(const ItemViewDelegate);

    return d->ratingRect;
}

void ItemViewDelegate::setRatingEdited(const QModelIndex& index)
{
    Q_D(ItemViewDelegate);

    d->editingRating = index;
}

QSize ItemViewDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ItemViewDelegate);

    return d->rect.size();
}

QSize ItemViewDelegate::gridSize() const
{
    Q_D(const ItemViewDelegate);

    return d->gridSize;
}

bool ItemViewDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ItemViewDelegate::acceptsActivation(const QPoint& , const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ItemViewDelegate::asDelegate()
{
    return this;
}

void ItemViewDelegate::overlayDestroyed(QObject* o)
{
    ItemDelegateOverlayContainer::overlayDestroyed(o);
}

void ItemViewDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection DItemDelegate -> ItemViewDelegate -> ItemDelegateOverlayContainer

    ItemDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ItemViewDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewDelegate);

    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewDelegate);

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

QRect ItemViewDelegate::drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background,
                                      const QPixmap& thumbnail, bool isGrouped) const
{
    Q_D(const ItemViewDelegate);

    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
    {
        return QRect();
    }

    QRect r    = thumbRect;
    double dpr = displayRatio();
    int thumbW = qRound((double)thumbnail.width()  / dpr);
    int thumbH = qRound((double)thumbnail.height() / dpr);

    QRect actualPixmapRect(r.x() + (r.width()  - thumbW) / 2,
                           r.y() + (r.height() - thumbH) / 2,
                           thumbW, thumbH);

    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size(),
                                              isGrouped);

    if (isGrouped)
    {
        const int xPadding = (borderPix.width()  - actualPixmapRect.width())  / 2;
        const int yPadding = (borderPix.height() - actualPixmapRect.height()) / 2;

        p->drawPixmap(actualPixmapRect.x() - xPadding,
                      actualPixmapRect.y() - yPadding, borderPix);

        QPixmap groupThumbnail = thumbnail.scaled(thumbnail.width()  - 10,
                                                  thumbnail.height() - 10,
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);

        p->drawPixmap(r.x() + (r.width()  - (thumbW - 10)) / 2,
                      r.y() + (r.height() - (thumbH - 10)) / 2,
                      thumbW - 10, thumbH - 10, groupThumbnail);
    }
    else
    {
        p->drawPixmap(actualPixmapRect.x() - d->radius,
                      actualPixmapRect.y() - d->radius, borderPix);

        p->drawPixmap(r.x() + (r.width()  - thumbW) / 2,
                      r.y() + (r.height() - thumbH) / 2,
                      thumbW, thumbH, thumbnail);
    }

    return actualPixmapRect;
}

void ItemViewDelegate::drawRating(QPainter* p, const QModelIndex& index, const QRect& ratingRect,
                                  int rating, bool isSelected) const
{
    Q_D(const ItemViewDelegate);

    if (d->editingRating != index)
    {
        p->drawPixmap(ratingRect, ratingPixmap(rating, isSelected));
    }
}

void ItemViewDelegate::drawSpecialInfo(QPainter* p,const QRect& r, const QString& text) const
{
    Q_D(const ItemViewDelegate);

    if (!text.isEmpty() && !r.isNull())
    {
        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        int flags   = Qt::AlignCenter | Qt::TextWordWrap;
        QRect bRect = p->boundingRect(r, flags, text);

        if (bRect.width() > r.width())
        {
            flags = Qt::AlignCenter | Qt::TextWrapAnywhere;
            bRect = p->boundingRect(r, flags, text);
        }

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, flags, text);

        p->restore();
    }
}

void ItemViewDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontReg);

    // NOTE: in case of file name are long, use squeezedTextCached to adjust string elide mode.
    // See bug #278664 for details

    p->drawText(nameRect, Qt::AlignCenter, squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewDelegate::drawTitle(QPainter* p, const QRect& titleRect, const QString& title) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontReg);
    p->drawText(titleRect, Qt::AlignCenter, squeezedTextCached(p, titleRect.width(), title));
}

void ItemViewDelegate::drawComments(QPainter* p, const QRect& commentsRect, const QString& comments) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontCom);
    p->drawText(commentsRect, Qt::AlignCenter, squeezedTextCached(p, commentsRect.width(), comments));
}

void ItemViewDelegate::drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontXtra);
    QString str = asUserDateTime(date);
/*
    str         = i18nc("date of image creation", "created: %1", str);
*/
    p->drawText(dateRect, Qt::AlignCenter, str); //squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewDelegate::drawModificationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontXtra);
    QString str = asUserDateTime(date);
    str         = i18nc("Date of last image modification, shortened", "Mod.: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, str); //squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewDelegate);

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

        p->drawText(dimsRect, Qt::AlignCenter, resolution); //squeezedTextCached(p, dimsRect.width(), resolution));
    }
}

void ItemViewDelegate::drawAspectRatio(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontXtra);
    QString resolution;

    if (dims.isValid())
    {
        ItemPropertiesTab::aspectRatioToString(dims.width(), dims.height(), resolution);
    }
    else
    {
        resolution = i18nc("unknown image resolution", "Unknown");
    }

    p->drawText(dimsRect, Qt::AlignCenter, resolution); //squeezedTextCached(p, dimsRect.width(), resolution));
}

void ItemViewDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, ItemPropertiesTab::humanReadableBytesCount(bytes));
}

void ItemViewDelegate::drawTags(QPainter* p, const QRect& r, const QString& tagsString,
                                bool isSelected) const
{
    Q_D(const ItemViewDelegate);

    p->setFont(d->fontCom);
    p->setPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                         : qApp->palette().color(QPalette::Link));

    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tagsString));
}

void ItemViewDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                                     bool isSelected) const
{
    Q_D(const ItemViewDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                                  : qApp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width() - 3, d->rect.height() - 3);
    }
}

void ItemViewDelegate::drawImageFormat(QPainter* p, const QRect& r, const QString& f, bool drawTop) const
{
    Q_D(const ItemViewDelegate);

    if (!f.isEmpty() && !r.isNull())
    {
        Qt::Alignment alignment = Qt::AlignBottom;

        if (drawTop)
        {
            alignment = Qt::AlignTop;
        }

        p->save();

        QFont fnt(d->fontReg);
        fnt.setWeight(QFont::Black);
        fnt.setItalic(false);
        p->setFont(fnt);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);

        QRect bRect = p->boundingRect(r, alignment | Qt::AlignHCenter, f.toUpper());
        bRect.adjust(0, 0, 0, -2);

        if (!drawTop)
        {
            bRect.translate(0, 1);
        }

        p->fillRect(bRect, Qt::SolidPattern);
        p->setPen(QPen(Qt::white));
        p->setOpacity(1.0);
        p->drawText(bRect, Qt::AlignBottom | Qt::AlignHCenter, f.toUpper());

        p->restore();
    }
}

void ItemViewDelegate::drawPickLabelIcon(QPainter* p, const QRect& r, int pickId) const
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

void ItemViewDelegate::drawPanelSideIcon(QPainter* p, bool left, bool right) const
{
    Q_D(const ItemViewDelegate);

    const int iconSize = qBound(16, d->rect.width() / 8 - 2, 48);

    if (left)
    {
        QRect r(3, d->rect.height() / 2 - iconSize / 2, iconSize, iconSize);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);
        p->fillRect(r, Qt::SolidPattern);
        QIcon icon = QIcon::fromTheme(QLatin1String("arrow-left"));
        p->setOpacity(1.0);
        icon.paint(p, r, Qt::AlignCenter, QIcon::Active, QIcon::On);
    }

    if (right)
    {
        QRect r(d->rect.width() - 3 - iconSize, d->rect.height() / 2 - iconSize / 2, iconSize, iconSize);
        p->setPen(QPen(Qt::gray));
        p->setOpacity(0.50);
        p->fillRect(r, Qt::SolidPattern);
        QIcon icon = QIcon::fromTheme(QLatin1String("arrow-right"));
        p->setOpacity(1.0);
        icon.paint(p, r, Qt::AlignCenter, QIcon::Active, QIcon::On);
    }
}

void ItemViewDelegate::drawGeolocationIndicator(QPainter* p, const QRect& r) const
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

void ItemViewDelegate::drawGroupIndicator(QPainter* p, const QRect& r,
                                          int numberOfGroupedImages, bool open) const
{
    if (numberOfGroupedImages)
    {
        QIcon icon     = QIcon::fromTheme(QLatin1String("box"));

        icon.paint(p, r);

        QPixmap pixmap = icon.pixmap(r.size());
        QPainter iconPainter(&pixmap);
        iconPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        iconPainter.fillRect(pixmap.rect(), qApp->palette().color(QPalette::Base));
        iconPainter.end();
        p->drawPixmap(r, pixmap);

        int fontSize   = qMin(r.width(), r.height()) / 2;
        fontSize       = qMax(fontSize, 8);

        QString text   = QString::number(numberOfGroupedImages + 1);
        QFont font     = p->font();
        font.setBold(true);
        font.setPixelSize(fontSize);
        p->setFont(font);

        if (open)
        {
            p->setPen(qApp->palette().color(QPalette::HighlightedText));
        }
        else
        {
            p->setPen(qApp->palette().color(QPalette::Text));
        }

        p->drawText(r, Qt::AlignCenter, text);
    }
}

void ItemViewDelegate::drawColorLabelLine(QPainter* p, const QRect& pixRect, int colorId) const
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

void ItemViewDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width() - 3, d->rect.height() - 3);
    }
}

void ItemViewDelegate::drawGroupedRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewDelegate);

    p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
    p->drawRect(d->groupedItemRect);
}

void ItemViewDelegate::prepareFonts()
{
    Q_D(ItemViewDelegate);

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

void ItemViewDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ItemViewDelegate);

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

void ItemViewDelegate::prepareBackground()
{
    Q_D(ItemViewDelegate);

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

void ItemViewDelegate::prepareRatingPixmaps(bool composeOverBackground)
{
    // Please call this method after prepareBackground() and when d->ratingPixmap is set

    Q_D(ItemViewDelegate);

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

            painter.translate(lround((d->ratingRect.width() - d->margin - rating *
                                     (d->starPolygonSize.width() + 1)) / 2.0) + 2, 0);

            for (int s = 0 ; s < rating ; ++s)
            {
                painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                painter.translate(d->starPolygonSize.width() + 1, 0);
            }
        }
    }
}

QPixmap ItemViewDelegate::ratingPixmap(int rating, bool selected) const
{
    Q_D(const ItemViewDelegate);

    if ((rating < 1) || (rating > 5))
    {
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

#include "moc_itemviewdelegate.cpp"
