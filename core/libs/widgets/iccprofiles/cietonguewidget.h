/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-10
 * Description : a widget to display CIE tongue from an ICC profile.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <QUrl>

// Local includes

#include "digikam-lcms.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT CIETongueWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CIETongueWidget(int w, int h,
                             QWidget* const parent = nullptr,
                             cmsHPROFILE hMonitor = nullptr);
    ~CIETongueWidget() override;

    bool setProfileData(const QByteArray& profileData = QByteArray());
    bool setProfileFromFile(const QUrl& file = QUrl());

    void loadingStarted();
    void loadingFailed();
    void uncalibratedColor();

protected:

    int  grids(double val) const;

    void outlineTongue();
    void fillTongue();
    void drawTongueAxis();
    void drawTongueGrid();
    void drawLabels();

    QRgb colorByCoord(double x, double y);
    void drawSmallElipse(LPcmsCIExyY xyY, BYTE r, BYTE g, BYTE b, int sz);

    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent*)         override;

private:

    void drawColorantTriangle();
    void drawWhitePoint();
    void drawPatches();
    void updatePixmap();

    void mapPoint(int& icx, int& icy, LPcmsCIExyY xyY);
    void biasedLine(int x1, int y1, int x2, int y2);
    void biasedText(int x, int y, const QString& txt);

    void setProfile(cmsHPROFILE hProfile);

private Q_SLOTS:

    void slotProgressTimerDone();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
