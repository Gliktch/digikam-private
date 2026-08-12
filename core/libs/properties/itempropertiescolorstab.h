/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display item colors information
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QByteArray>
#include <QTabWidget>
#include <QUrl>
#include <QRect>

// Local includes

#include "dimg.h"
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DImg;
class LoadingDescription;

class DIGIKAM_EXPORT ItemPropertiesColorsTab : public QTabWidget
{
    Q_OBJECT

public:

    explicit ItemPropertiesColorsTab(QWidget* const parent);
    ~ItemPropertiesColorsTab() override;

    void setData(const QUrl& url=QUrl(),
                 const QRect& selectionArea = QRect(),
                 DImg* const img=nullptr);

    void setSelection(const QRect& selectionArea);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

private:

    void loadImageFromUrl(const QUrl& url);
    void updateInformation();
    void updateStatistics();
    void getICCData();

private Q_SLOTS:

    void slotRefreshOptions();
    void slotHistogramComputationFailed();
    void slotChannelChanged();
    void slotScaleChanged();
    void slotRenderingChanged(int rendering);
    void slotMinValueChanged(int);
    void slotMaxValueChanged(int);

    void slotUpdateInterval(int min, int max);
    void slotUpdateIntervalFromRGB(int min, int max);
    void slotUpdateIntervRange(int range);

    void slotLoadImageFromUrlComplete(const LoadingDescription& loadingDescription, const DImg& img);
    void slotMoreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                          const LoadingDescription& newLoadingDescription);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
