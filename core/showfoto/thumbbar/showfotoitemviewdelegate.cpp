/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt model view for Showfoto item - the delegate
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotoitemviewdelegate_p.h"

namespace ShowFoto
{

void ShowfotoItemViewDelegatePrivate::init(ShowfotoItemViewDelegate* const _q, QWidget* const _widget)
{
    q             = _q;
    displayWidget = _widget;

    q->connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ShowfotoItemViewDelegatePrivate::clearRects()
{
    gridSize   = QSize(0, 0);
    rect       = QRect(0, 0, 0, 0);
}

// ---- ShowfotoItemViewDelegate -----------------------------------------------

ShowfotoItemViewDelegate::ShowfotoItemViewDelegate(QWidget* const parent)
    : DItemDelegate(parent),
      d_ptr        (new ShowfotoItemViewDelegatePrivate)
{
    d_ptr->init(this, parent);
}

ShowfotoItemViewDelegate::ShowfotoItemViewDelegate(ShowfotoItemViewDelegatePrivate& dd, QWidget* const parent)
    : DItemDelegate(parent),
      d_ptr        (&dd)
{
    d_ptr->init(this, parent);
}

ShowfotoItemViewDelegate::~ShowfotoItemViewDelegate()
{
    Q_D(ShowfotoItemViewDelegate);

    removeAllOverlays();
    delete d;
}

ThumbnailSize ShowfotoItemViewDelegate::thumbnailSize() const
{
    Q_D(const ShowfotoItemViewDelegate);

    return d->thumbSize;
}

double ShowfotoItemViewDelegate::displayRatio() const
{
    Q_D(const ShowfotoItemViewDelegate);

    return d->displayWidget->devicePixelRatio();
}

void ShowfotoItemViewDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ShowfotoItemViewDelegate);

    if (d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ShowfotoItemViewDelegate::setSpacing(int spacing)
{
    Q_D(ShowfotoItemViewDelegate);

    if (d->spacing == spacing)
    {
        return;
    }

    d->spacing = spacing;
    invalidatePaintingCache();
}

int ShowfotoItemViewDelegate::spacing() const
{
    Q_D(const ShowfotoItemViewDelegate);

    return d->spacing;
}

QRect ShowfotoItemViewDelegate::rect() const
{
    Q_D(const ShowfotoItemViewDelegate);

    return d->rect;
}

QRect ShowfotoItemViewDelegate::pixmapRect() const
{
    return QRect();
}

QRect ShowfotoItemViewDelegate::imageInformationRect() const
{
    return QRect();
}

QSize ShowfotoItemViewDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ShowfotoItemViewDelegate);

    return d->rect.size();
}

QSize ShowfotoItemViewDelegate::gridSize() const
{
    Q_D(const ShowfotoItemViewDelegate);

    return d->gridSize;
}

bool ShowfotoItemViewDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ShowfotoItemViewDelegate::acceptsActivation(const QPoint& , const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ShowfotoItemViewDelegate::asDelegate()
{
    return this;
}

void ShowfotoItemViewDelegate::overlayDestroyed(QObject* o)
{
    ItemDelegateOverlayContainer::overlayDestroyed(o);
}

void ShowfotoItemViewDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection AbstractShowfotoItemDelegate -> ShowfotoItemViewDelegate -> ItemDelegateOverlayContainer

    ItemDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ShowfotoItemViewDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ShowfotoItemViewDelegate);

    d->font = option.font;
    invalidatePaintingCache();
}

void ShowfotoItemViewDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ShowfotoItemViewDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ShowfotoItemViewDelegate::invalidatePaintingCache()
{
    Q_D(ShowfotoItemViewDelegate);

    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();

    // cppcheck-suppress knownConditionTrueFalse
    if (oldGridSize != d->gridSize)
    {
        Q_EMIT gridSizeChanged(d->gridSize);
/*
        // Q_EMIT sizeHintChanged(QModelIndex());
*/
    }

    Q_EMIT visualChange();
}

QRect ShowfotoItemViewDelegate::drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background,
                                              const QPixmap& thumbnail) const
{
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

    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());

    p->drawPixmap(actualPixmapRect.x() - 3,
                  actualPixmapRect.y() - 3, borderPix);

    p->drawPixmap(r.x() + (r.width()  - thumbW) / 2,
                  r.y() + (r.height() - thumbH) / 2,
                  thumbW, thumbH, thumbnail);

    return actualPixmapRect;
}

void ShowfotoItemViewDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ShowfotoItemViewDelegate);

    p->setFont(d->fontReg);
    p->drawText(nameRect, Qt::AlignCenter, name);   //squeezedTextCached(p, nameRect.width(), name));
}

void ShowfotoItemViewDelegate::drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ShowfotoItemViewDelegate);

    p->setFont(d->fontXtra);
    QString str = asUserDateTime(date);
    str         = i18nc("date of image creation", "created: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, str);    //squeezedTextCached(p, dateRect.width(), str));
}

void ShowfotoItemViewDelegate::drawImageFormat(QPainter* p, const QRect& r, const QString& mime) const
{
    Q_D(const ShowfotoItemViewDelegate);

    if (!mime.isEmpty() && !r.isNull())
    {
        QString type = mime;

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

void ShowfotoItemViewDelegate::drawGeolocationIndicator(QPainter* p, const QRect& r) const
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

void ShowfotoItemViewDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ShowfotoItemViewDelegate);

    if (dims.isValid())
    {
        p->setFont(d->fontXtra);
        QString mpixels, resolution;
        mpixels = QLocale().toString(dims.width()*dims.height()/1000000.0, 'f', 1);

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

void ShowfotoItemViewDelegate::drawFileSize(QPainter* p, const QRect& r, qlonglong bytes) const
{
    Q_D(const ShowfotoItemViewDelegate);

    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, ItemPropertiesTab::humanReadableBytesCount(bytes));
}

void ShowfotoItemViewDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option,
                                             bool isSelected) const
{
    Q_D(const ShowfotoItemViewDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? qApp->palette().color(QPalette::HighlightedText)
                                  : qApp->palette().color(QPalette::Text),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ShowfotoItemViewDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ShowfotoItemViewDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ShowfotoItemViewDelegate::prepareFonts()
{
    Q_D(ShowfotoItemViewDelegate);

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

void ShowfotoItemViewDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ShowfotoItemViewDelegate);

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

void ShowfotoItemViewDelegate::prepareBackground()
{
    Q_D(ShowfotoItemViewDelegate);

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
        p1.drawRect(0, 0, d->rect.width()-1, d->rect.height()-1);

        d->selPixmap = QPixmap(d->rect.width(), d->rect.height());
        d->selPixmap.fill(qApp->palette().color(QPalette::Highlight));
        QPainter p2(&d->selPixmap);
        p2.setPen(qApp->palette().color(QPalette::Midlight));
        p2.drawRect(0, 0, d->rect.width()-1, d->rect.height()-1);
    }
}

} // namespace ShowFoto

#include "moc_showfotoitemviewdelegate.cpp"
