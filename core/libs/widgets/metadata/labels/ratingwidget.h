/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * SPDX-FileCopyrightText: 2005      by Owen Hirst <n8rider@sbcglobal.net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPolygon>
#include <QIcon>
#include <QMenu>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RatingWidget : public QWidget
{
    Q_OBJECT

public:

    explicit RatingWidget(QWidget* const parent);
    ~RatingWidget()                                 override;

    void setRating(int val);
    int  rating() const;

    void setTracking(bool tracking);
    bool hasTracking() const;

    void setFading(bool fading);
    bool hasFading() const;

    void startFading();
    void stopFading();
    void setVisibleImmediately();

    void setVisible(bool visible)                   override;
    int  maximumVisibleWidth() const;

    /**
     * Pre-computed star polygon for a 15x15 pixmap.
     */
    static QPolygon starPolygon();
    static QIcon buildIcon(int rate, int size);

Q_SIGNALS:

    void signalRatingModified(int);    ///< Not managed by tracking properties
    void signalRatingChanged(int);

protected:

    int regPixmapWidth()         const;

    QPixmap starPixmap()         const;
    QPixmap starPixmapFilled()   const;
    QPixmap starPixmapDisabled() const;

    void regeneratePixmaps();
    void setupTimeLine();
    void applyFading(QPixmap& pix);

    void mousePressEvent(QMouseEvent*)              override;
    void mouseMoveEvent(QMouseEvent*)               override;
    void mouseReleaseEvent(QMouseEvent*)            override;
    void paintEvent(QPaintEvent*)                   override;

protected Q_SLOTS:

    void setFadingValue(int value);

private Q_SLOTS:

    void slotThemeChanged();

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT RatingBox : public DVBox
{
    Q_OBJECT

public:

    explicit RatingBox(QWidget* const parent);
    ~RatingBox() override;

Q_SIGNALS:

    void signalRatingChanged(int);

private Q_SLOTS:

    void slotUpdateDescription(int);

private:

    class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------

class DIGIKAM_EXPORT RatingMenuAction : public QMenu
{
    Q_OBJECT

public:

    explicit RatingMenuAction(QMenu* const parent = nullptr);
    ~RatingMenuAction() override = default;

Q_SIGNALS:

    void signalRatingChanged(int);
};

} // namespace Digikam
