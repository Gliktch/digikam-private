/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-08-09
 * Description : reusable privacy state adornment for item views
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPalette>
#include <QRectF>
#include <QSharedPointer>
#include <QString>

// Local includes

#include "digikam_export.h"

class QPainter;

namespace Digikam
{

class PrivacyActionStateProvider;

enum class PrivacyItemViewState
{
    Unprotected      = 1,
    ProtectedLocked  = 2,
    ProtectedUnlocked = 3
};

class DIGIKAM_GUI_EXPORT PrivacyItemViewAdornmentLayout
{
public:

    QRectF borderRect;
    QRectF innerBorderRect;
    QRectF lockBadgeRect;
    qreal  outerBorderWidth = 0.0;
    qreal  innerBorderWidth = 0.0;
};

/**
 * Paint-only privacy treatment for catalogue item tiles. It never mutates the
 * supplied thumbnail, proxy, derivative or cache object: callers invoke it on
 * the destination painter after drawing presentation pixels.
 */
class DIGIKAM_GUI_EXPORT PrivacyItemViewAdornment
{
public:

    /**
     * Installs a memory-backed state provider. A null provider restores the
     * process runtime provider. Missing, invalid or conflicting state fails
     * closed as protected-and-locked.
     */
    static void setStateProvider(
        const QSharedPointer<const PrivacyActionStateProvider>& provider);
    static void resetStateProvider();

    static PrivacyItemViewState stateForItem(qlonglong imageId);
    static QString statusText(PrivacyItemViewState state);
    static QString withStatusToolTip(const QString& toolTip,
                                     PrivacyItemViewState state);

    static PrivacyItemViewAdornmentLayout layout(const QRectF& itemRect,
                                                  qreal devicePixelRatio = 1.0);

    static void paint(QPainter* painter,
                      const QRectF& itemRect,
                      PrivacyItemViewState state,
                      const QPalette& palette,
                      qreal devicePixelRatio = 1.0);
};

} // namespace Digikam
