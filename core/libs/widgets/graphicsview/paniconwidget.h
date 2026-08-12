/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a generic widget to display a panel to choose
 *               a rectangular image area.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QRect>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>

// Local includes

#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DIGIKAM_EXPORT PanIconWidget : public QWidget
{
    Q_OBJECT

public:

    explicit PanIconWidget(QWidget* const parent = nullptr);
    ~PanIconWidget()                        override;

    void setImage(int previewWidth, int previewHeight, const QImage& fullOriginalImage);
    void setImage(int previewWidth, int previewHeight, const DImg& fullOriginalImage);
    void setImage(const QImage& scaledPreviewImage, const QSize& fullImageSize);

    void  setRegionSelection(const QRect& regionSelection);
    QRect getRegionSelection() const;

    void setPreviewMode(int previewMode);

Q_SIGNALS:

    /**
     * Emitted when selection have been moved with mouse.
     * 'targetDone' boolean value is used for indicate if the mouse have been released.
     */
    void signalSelectionMoved(const QRect& rect, bool targetDone);

public Q_SLOTS:

    void slotZoomFactorChanged(double);

protected:

    void paintEvent(QPaintEvent*)           override;
    void mousePressEvent(QMouseEvent*)      override;
    void mouseReleaseEvent(QMouseEvent*)    override;
    void mouseMoveEvent(QMouseEvent*)       override;

    /**
     * Recalculate the target selection position and emit 'signalSelectionMoved'.
     */
    void regionSelectionMoved(bool targetDone);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
