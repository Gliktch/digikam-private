/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-05)
 * Description : HotPixel settings view.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>
#include <QPolygon>
#include <QList>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "hotpixelcontainer.h"
#include "hotpixelprops.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT HotPixelSettings : public QWidget
{
    Q_OBJECT

public:

    explicit HotPixelSettings(QWidget* const parent);
    ~HotPixelSettings() override;

    HotPixelContainer defaultSettings()  const;
    void resetToDefault();

    HotPixelContainer settings()         const;
    void setSettings(const HotPixelContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    QString configGroupName()            const;

Q_SIGNALS:

    void signalSettingsChanged();
    void signalHotPixels(const QPolygon& pointList);

private Q_SLOTS:

    void slotAddBlackFrame();
    void slotBlackFrameSelected(const QList<HotPixelProps>& hpList, const QUrl& url);
    void slotBlackFrameRemoved(const QUrl& url);
    void slotClearBlackFrameList();

private:

    void loadBlackFrame(const QUrl& url, bool selected);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
