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

#pragma once

// Qt includes

#include <QSize>

// Local includes

#include "FrameGraphicsItem.h"
#include "digikam_export.h"

class QImage;
class QIcon;

namespace Marble
{

class LabelGraphicsItemPrivate;

/**
 * A label item provides an Item that displays text or images/pixmaps.
 * The text is displayed as plain text.
 * The item also provides frames.
 */
class DIGIKAM_EXPORT LabelGraphicsItem : public FrameGraphicsItem
{
public:

    explicit LabelGraphicsItem(MarbleGraphicsItem* parent = nullptr);
    ~LabelGraphicsItem() override;

    void setContentSize(const QSizeF& contentSize) override;

    QString text() const;
    void setText(const QString& text);

    QImage image() const;
    void setImage(const QImage& image, const QSize& size = QSize());

    QIcon icon() const;
    void setIcon(const QIcon& icon, const QSize& size);

    void setMinimumSize(const QSizeF& size);
    QSizeF minimumSize() const;

    void clear();

protected:

    void paintContent(QPainter* painter) override;

private:

    Q_DISABLE_COPY(LabelGraphicsItem)
    Q_DECLARE_PRIVATE(LabelGraphicsItem)
};

} // namespace Marble
