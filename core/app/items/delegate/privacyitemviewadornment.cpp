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

#include "privacyitemviewadornment.h"

// Qt includes

#include <QGlobalStatic>
#include <QPainter>
#include <QReadLocker>
#include <QReadWriteLock>
#include <QVector>
#include <QWriteLocker>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "privacyactionpolicy.h"
#include "privacyruntime.h"

namespace Digikam
{

namespace
{

class PrivacyItemViewAdornmentData
{
public:

    QReadWriteLock                                   lock;
    QSharedPointer<const PrivacyActionStateProvider> stateProvider;
};

Q_GLOBAL_STATIC(PrivacyItemViewAdornmentData, adornmentData)

QSharedPointer<const PrivacyActionStateProvider> effectiveStateProvider()
{
    QSharedPointer<const PrivacyActionStateProvider> provider;

    {
        QReadLocker locker(&adornmentData->lock);
        provider = adornmentData->stateProvider;
    }

    return provider ? provider
                    : PrivacyStartupRecovery::actionStateProvider();
}

} // namespace

void PrivacyItemViewAdornment::setStateProvider(
    const QSharedPointer<const PrivacyActionStateProvider>& provider)
{
    QSharedPointer<const PrivacyActionStateProvider> previous;

    {
        QWriteLocker locker(&adornmentData->lock);
        previous.swap(adornmentData->stateProvider);
        adornmentData->stateProvider = provider;
    }
}

void PrivacyItemViewAdornment::resetStateProvider()
{
    setStateProvider(QSharedPointer<const PrivacyActionStateProvider>());
}

PrivacyItemViewState PrivacyItemViewAdornment::stateForItem(qlonglong imageId)
{
    const QSharedPointer<const PrivacyActionStateProvider> provider =
        effectiveStateProvider();
    PrivacyActionItemState state;

    if ((imageId <= 0) || !provider ||
        !provider->stateForItem(imageId, &state) || !state.isValid())
    {
        return PrivacyItemViewState::ProtectedLocked;
    }

    if (!state.protectedItem)
    {
        return PrivacyItemViewState::Unprotected;
    }

    return (state.access == PrivacyItemAccess::Unlocked)
         ? PrivacyItemViewState::ProtectedUnlocked
         : PrivacyItemViewState::ProtectedLocked;
}

QString PrivacyItemViewAdornment::statusText(PrivacyItemViewState state)
{
    switch (state)
    {
        case PrivacyItemViewState::ProtectedLocked:
        {
            return i18nc("accessible privacy state", "Protected item, locked");
        }

        case PrivacyItemViewState::ProtectedUnlocked:
        {
            return i18nc("accessible privacy state", "Protected item, unlocked");
        }

        case PrivacyItemViewState::Unprotected:
        default:
        {
            return QString();
        }
    }
}

QString PrivacyItemViewAdornment::withStatusToolTip(const QString& toolTip,
                                                    PrivacyItemViewState state)
{
    const QString status = statusText(state);

    if (status.isEmpty())
    {
        return toolTip;
    }

    const QString escaped = status.toHtmlEscaped();
    const QString row = QLatin1String("<tr><td colspan=\"2\"><b>") +
                        escaped + QLatin1String("</b></td></tr>");
    const qsizetype tableStart = toolTip.indexOf(QLatin1String("<table"));

    if (tableStart >= 0)
    {
        const qsizetype insertion = toolTip.indexOf(QLatin1Char('>'), tableStart);

        if (insertion >= 0)
        {
            QString result = toolTip;
            result.insert(insertion + 1, row);

            return result;
        }
    }

    return toolTip.isEmpty() ? status
                             : (status + QLatin1Char('\n') + toolTip);
}

PrivacyItemViewAdornmentLayout PrivacyItemViewAdornment::layout(
    const QRectF& itemRect,
    qreal devicePixelRatio)
{
    PrivacyItemViewAdornmentLayout result;

    if (!itemRect.isValid() || itemRect.isEmpty())
    {
        return result;
    }

    const qreal dpr = qBound<qreal>(1.0, devicePixelRatio, 8.0);
    const qreal shortest = qMin(itemRect.width(), itemRect.height());
    const qreal scale = qBound<qreal>(0.65, shortest / 64.0, 1.8);
    const qreal oneDevicePixel = 1.0 / dpr;

    result.outerBorderWidth = qMax(oneDevicePixel, 1.8 * scale);
    result.innerBorderWidth = qMax(oneDevicePixel, 1.0 * scale);
    result.borderRect = itemRect.adjusted(result.outerBorderWidth / 2.0,
                                           result.outerBorderWidth / 2.0,
                                          -result.outerBorderWidth / 2.0,
                                          -result.outerBorderWidth / 2.0);

    const qreal innerInset = qMax(2.5 * scale,
                                  result.outerBorderWidth + oneDevicePixel);
    result.innerBorderRect = result.borderRect.adjusted(innerInset,
                                                        innerInset,
                                                       -innerInset,
                                                       -innerInset);

    const qreal badgeExtent = qMin(qMax(12.0 * scale, shortest * 0.28),
                                   shortest * 0.50);
    const qreal badgeInset = qMax(2.0 * scale, result.outerBorderWidth);
    result.lockBadgeRect = QRectF(result.borderRect.right() - badgeInset - badgeExtent,
                                  result.borderRect.top() + badgeInset,
                                  badgeExtent,
                                  badgeExtent);

    return result;
}

void PrivacyItemViewAdornment::paint(QPainter* painter,
                                     const QRectF& itemRect,
                                     PrivacyItemViewState state,
                                     const QPalette& palette,
                                     qreal devicePixelRatio)
{
    if (!painter || !painter->isActive() ||
        (state == PrivacyItemViewState::Unprotected))
    {
        return;
    }

    const PrivacyItemViewAdornmentLayout metrics = layout(itemRect,
                                                           devicePixelRatio);

    if (!metrics.borderRect.isValid() || metrics.borderRect.isEmpty())
    {
        return;
    }

    const QColor accent = palette.color(QPalette::Highlight);
    const QColor contrast = palette.color(QPalette::HighlightedText);
    const QColor outline = palette.color(QPalette::Text);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen outerPen(accent, metrics.outerBorderWidth, Qt::SolidLine,
                  Qt::SquareCap, Qt::MiterJoin);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(outerPen);
    painter->drawRect(metrics.borderRect);

    if (metrics.innerBorderRect.isValid() && !metrics.innerBorderRect.isEmpty())
    {
        QPen innerPen(outline, metrics.innerBorderWidth, Qt::DashLine,
                      Qt::FlatCap, Qt::MiterJoin);
        innerPen.setDashPattern(QVector<qreal>() << 2.0 << 1.5);
        painter->setPen(innerPen);
        painter->drawRect(metrics.innerBorderRect);
    }

    if (state == PrivacyItemViewState::ProtectedLocked)
    {
        const QRectF badge = metrics.lockBadgeRect;
        const qreal radius = badge.width() * 0.18;
        painter->setPen(QPen(outline, metrics.innerBorderWidth));
        painter->setBrush(accent);
        painter->drawRoundedRect(badge, radius, radius);

        const qreal unit = badge.width() / 10.0;
        const QRectF body(badge.left() + (2.2 * unit),
                          badge.top()  + (4.4 * unit),
                          5.6 * unit,
                          3.8 * unit);
        const QRectF shackle(badge.left() + (3.0 * unit),
                             badge.top()  + (1.7 * unit),
                             4.0 * unit,
                             5.0 * unit);

        QPen glyphPen(contrast, qMax(metrics.innerBorderWidth, 1.2 * unit),
                      Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(glyphPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(shackle, 0, 180 * 16);
        painter->setPen(Qt::NoPen);
        painter->setBrush(contrast);
        painter->drawRoundedRect(body, 0.8 * unit, 0.8 * unit);

        painter->setBrush(accent);
        painter->drawEllipse(QPointF(body.center().x(),
                                     body.top() + (1.5 * unit)),
                             0.55 * unit, 0.55 * unit);
    }

    painter->restore();
}

} // namespace Digikam
