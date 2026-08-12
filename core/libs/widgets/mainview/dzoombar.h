/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : a zoom bar used in status bar.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QAction>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DZoomBar : public DHBox
{
    Q_OBJECT

public:

    enum BarMode
    {
        PreviewZoomCtrl = 0,      ///< Preview Zoom controller.
        ThumbsSizeCtrl,           ///< Thumb Size controller. Preview zoom controller still visible but disabled.
        NoPreviewZoomCtrl         ///< Thumb Size controller alone. Preview Zoom controller is hidden.
    };

public:

    explicit DZoomBar(QWidget* const parent = nullptr);
    ~DZoomBar() override;

    void setBarMode(BarMode mode);
    void setZoom(double zoom, double zmin, double zmax);
    void setThumbsSize(int size);

    void setZoomToFitAction(QAction* const action);
    void setZoomTo100Action(QAction* const action);
    void setZoomPlusAction(QAction* const action);
    void setZoomMinusAction(QAction* const action);

    void triggerZoomTrackerToolTip();

    static int    sizeFromZoom(double zoom, double zmin, double zmax);
    static double zoomFromSize(int size, double zmin, double zmax);

Q_SIGNALS:

    void signalZoomSliderChanged(int);
    void signalDelayedZoomSliderChanged(int);
    void signalZoomSliderReleased(int);
    void signalZoomValueEdited(double);

public Q_SLOTS:

    void slotUpdateTrackerPos();

private Q_SLOTS:

    void slotZoomSliderChanged(int);
    void slotDelayedZoomSliderChanged();
    void slotZoomSliderReleased();
    void slotZoomSelected(int);
    void slotZoomTextChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
