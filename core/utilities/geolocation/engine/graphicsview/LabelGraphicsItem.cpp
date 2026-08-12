/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#include "LabelGraphicsItem.h"
#include "LabelGraphicsItem_p.h"

// Qt includes

#include <QString>
#include <QApplication>
#include <QFont>
#include <QPainter>

namespace Marble
{

LabelGraphicsItemPrivate::LabelGraphicsItemPrivate(LabelGraphicsItem* labelGraphicsItem,
                                                   MarbleGraphicsItem* parent)
    : FrameGraphicsItemPrivate(labelGraphicsItem, parent)
{
}

QFont LabelGraphicsItemPrivate::font()
{
    return QApplication::font();
}

void LabelGraphicsItem::setContentSize(const QSizeF& contentSize)
{
    Q_D(LabelGraphicsItem);
    QSizeF updatedSize = contentSize;

    if (updatedSize.isEmpty())
    {
        updatedSize.setHeight(0);
        updatedSize.setWidth(0);
    }

    else
    {
        if (d->m_minimumSize.width() > updatedSize.width())
        {
            updatedSize.setWidth(d->m_minimumSize.width());
        }

        if (d->m_minimumSize.height() > updatedSize.height())
        {
            updatedSize.setHeight(d->m_minimumSize.height());
        }
    }

    FrameGraphicsItem::setContentSize(updatedSize);
}

// ----------------------------------------------------------------

LabelGraphicsItem::LabelGraphicsItem(MarbleGraphicsItem* parent)
    : FrameGraphicsItem(new LabelGraphicsItemPrivate(this, parent))
{
}

LabelGraphicsItem::~LabelGraphicsItem()
{
}

QString LabelGraphicsItem::text() const
{
    Q_D(const LabelGraphicsItem);
    return d->m_text;
}

void LabelGraphicsItem::setText(const QString& text)
{
    Q_D(LabelGraphicsItem);
    clear();
    d->m_text = text;
    QFontMetrics metrics(d->font());
    QSizeF size = metrics.boundingRect(text).size() + QSizeF(14, 2);
    setContentSize(size);
}

QImage LabelGraphicsItem::image() const
{
    Q_D(const LabelGraphicsItem);
    return d->m_image;
}

void LabelGraphicsItem::setImage(const QImage& image, const QSize& size)
{
    Q_D(LabelGraphicsItem);
    clear();
    d->m_image = image;

    if (size.isEmpty())
    {
        setContentSize(image.size());
    }

    else
    {
        setContentSize(size);
    }
}

QIcon LabelGraphicsItem::icon() const
{
    Q_D(const LabelGraphicsItem);
    return d->m_icon;
}

void LabelGraphicsItem::setIcon(const QIcon& icon, const QSize& size)
{
    Q_D(LabelGraphicsItem);
    clear();
    d->m_icon = icon;
    setContentSize(size);
}

QSizeF LabelGraphicsItem::minimumSize() const
{
    Q_D(const LabelGraphicsItem);
    return d->m_minimumSize;
}

void LabelGraphicsItem::setMinimumSize(const QSizeF& size)
{
    Q_D(LabelGraphicsItem);
    const QSizeF oldContentSize = contentSize();
    d->m_minimumSize = size;
    setContentSize(oldContentSize);
}

void LabelGraphicsItem::clear()
{
    Q_D(LabelGraphicsItem);
    d->m_text.clear();
    d->m_image = QImage();
    d->m_icon = QIcon();
    setContentSize(QSizeF(0.0, 0.0));
}

void LabelGraphicsItem::paintContent(QPainter* painter)
{
    Q_D(LabelGraphicsItem);
    painter->save();

    if (!d->m_text.isNull())
    {
        painter->setFont(d->font());
        painter->setPen(QColor(Qt::black));
        painter->drawText(QRect(QPoint(0, 0), contentSize().toSize()),
                          Qt::AlignVCenter | Qt::AlignLeft,
                          d->m_text);
    }

    else if (!d->m_image.isNull())
    {
        painter->drawImage(QRectF(QPointF(0, 0), contentSize()),
                           d->m_image);
    }

    else if (!d->m_icon.isNull())
    {
        d->m_icon.paint(painter,
                        QRect(QPoint(0, 0), contentSize().toSize()),
                        Qt::AlignCenter);
    }

    painter->restore();
}

} // namespace Marble
