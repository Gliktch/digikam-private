/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-08-09
 * Description : unit tests for privacy item-view adornments
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QSharedPointer>
#include <QTest>

// Local includes

#include "privacyactionpolicy.h"
#include "privacyitemviewadornment.h"

using namespace Digikam;

namespace
{

class SyntheticStateProvider final : public PrivacyActionStateProvider
{
public:

    bool stateForItem(qlonglong imageId,
                      PrivacyActionItemState* state) const override
    {
        if (!state || (imageId == 4))
        {
            return false;
        }

        *state = PrivacyActionItemState();

        if (imageId == 5)
        {
            state->protectedItem = true;

            return true;
        }

        if (imageId == 3)
        {
            return true;
        }

        state->protectedItem     = true;
        state->itemUuid          = QLatin1String("22222222-2222-4222-8222-222222222222");
        state->categoryUuid      = QLatin1String("11111111-1111-4111-8111-111111111111");
        state->access            = (imageId == 2) ? PrivacyItemAccess::Unlocked
                                                  : PrivacyItemAccess::Locked;
        state->publicRootState   = PrivacyRootRuntimeState::VerifiedAvailable;
        state->originalRootState = PrivacyRootRuntimeState::VerifiedAvailable;
        state->checkoutRootState = PrivacyRootRuntimeState::VerifiedAvailable;
        state->itemGeneration    = 1;

        return true;
    }
};

QImage syntheticSource()
{
    QImage source(48, 48, QImage::Format_ARGB32_Premultiplied);

    for (int y = 0 ; y < source.height() ; ++y)
    {
        QRgb* const line = reinterpret_cast<QRgb*>(source.scanLine(y));

        for (int x = 0 ; x < source.width() ; ++x)
        {
            line[x] = qRgba((x * 5) % 255, (y * 7) % 255,
                            ((x + y) * 3) % 255, 255);
        }
    }

    return source;
}

QPalette lightPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(24, 95, 180));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Active, QPalette::Text, QColor(20, 20, 20));

    return palette;
}

QPalette darkPalette()
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(110, 190, 255));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, Qt::black);
    palette.setColor(QPalette::Active, QPalette::Text, QColor(235, 235, 235));

    return palette;
}

struct RenderResult
{
    QImage baseline;
    QImage adorned;
};

RenderResult render(const QImage& source,
                    PrivacyItemViewState state,
                    const QPalette& palette,
                    int logicalExtent,
                    qreal dpr)
{
    const QSize pixelSize(qRound(logicalExtent * dpr),
                          qRound(logicalExtent * dpr));
    QImage baseline(pixelSize, QImage::Format_ARGB32_Premultiplied);
    baseline.setDevicePixelRatio(dpr);
    baseline.fill(QColor(80, 80, 80));

    const qreal margin = logicalExtent / 8.0;
    const QRectF itemRect(margin, margin,
                          logicalExtent - (2.0 * margin),
                          logicalExtent - (2.0 * margin));

    {
        QPainter painter(&baseline);
        painter.drawImage(itemRect, source);
    }

    QImage adorned = baseline;

    {
        QPainter painter(&adorned);
        PrivacyItemViewAdornment::paint(&painter, itemRect, state,
                                        palette, dpr);
    }

    return { baseline, adorned };
}

int changedPixelCount(const QImage& first, const QImage& second)
{
    if ((first.size() != second.size()) || (first.format() != second.format()))
    {
        return -1;
    }

    int changed = 0;

    for (int y = 0 ; y < first.height() ; ++y)
    {
        const QRgb* const firstLine = reinterpret_cast<const QRgb*>(first.constScanLine(y));
        const QRgb* const secondLine = reinterpret_cast<const QRgb*>(second.constScanLine(y));

        for (int x = 0 ; x < first.width() ; ++x)
        {
            changed += (firstLine[x] != secondLine[x]) ? 1 : 0;
        }
    }

    return changed;
}

} // namespace

class PrivacyItemViewAdornmentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init();
    void cleanup();

    void testInjectedStateAndFailClosed();
    void testLockedAndUnlockedRenderingPreservesSource();
    void testUnprotectedDoesNotPaint();
    void testThemeScaleAndDevicePixelRatio();
    void testAccessibleStatusToolTip();
};

void PrivacyItemViewAdornmentTest::init()
{
    PrivacyItemViewAdornment::setStateProvider(
        QSharedPointer<SyntheticStateProvider>(new SyntheticStateProvider));
}

void PrivacyItemViewAdornmentTest::cleanup()
{
    PrivacyItemViewAdornment::resetStateProvider();
}

void PrivacyItemViewAdornmentTest::testInjectedStateAndFailClosed()
{
    QCOMPARE(PrivacyItemViewAdornment::stateForItem(1),
             PrivacyItemViewState::ProtectedLocked);
    QCOMPARE(PrivacyItemViewAdornment::stateForItem(2),
             PrivacyItemViewState::ProtectedUnlocked);
    QCOMPARE(PrivacyItemViewAdornment::stateForItem(3),
             PrivacyItemViewState::Unprotected);

    // Provider failure, invalid ids and conflicting runtime state are never
    // interpreted as unprotected.

    QCOMPARE(PrivacyItemViewAdornment::stateForItem(4),
             PrivacyItemViewState::ProtectedLocked);
    QCOMPARE(PrivacyItemViewAdornment::stateForItem(5),
             PrivacyItemViewState::ProtectedLocked);
    QCOMPARE(PrivacyItemViewAdornment::stateForItem(-1),
             PrivacyItemViewState::ProtectedLocked);
}

void PrivacyItemViewAdornmentTest::testLockedAndUnlockedRenderingPreservesSource()
{
    QImage source = syntheticSource();
    const QImage original = source;
    const RenderResult locked = render(source,
                                       PrivacyItemViewState::ProtectedLocked,
                                       lightPalette(), 80, 1.0);
    const RenderResult unlocked = render(source,
                                         PrivacyItemViewState::ProtectedUnlocked,
                                         lightPalette(), 80, 1.0);

    QVERIFY(source == original);
    QVERIFY(changedPixelCount(locked.baseline, locked.adorned) > 0);
    QVERIFY(changedPixelCount(unlocked.baseline, unlocked.adorned) > 0);
    QVERIFY(locked.adorned != unlocked.adorned);
}

void PrivacyItemViewAdornmentTest::testUnprotectedDoesNotPaint()
{
    const QImage source = syntheticSource();
    const RenderResult unprotected = render(source,
                                             PrivacyItemViewState::Unprotected,
                                             lightPalette(), 80, 1.0);

    QVERIFY(unprotected.baseline == unprotected.adorned);
}

void PrivacyItemViewAdornmentTest::testThemeScaleAndDevicePixelRatio()
{
    const QImage source = syntheticSource();
    const RenderResult light = render(source,
                                      PrivacyItemViewState::ProtectedLocked,
                                      lightPalette(), 80, 1.0);
    const RenderResult dark = render(source,
                                     PrivacyItemViewState::ProtectedLocked,
                                     darkPalette(), 80, 1.0);
    const RenderResult highDpi = render(source,
                                        PrivacyItemViewState::ProtectedLocked,
                                        darkPalette(), 80, 2.0);
    const PrivacyItemViewAdornmentLayout small =
        PrivacyItemViewAdornment::layout(QRectF(0, 0, 32, 32), 1.0);
    const PrivacyItemViewAdornmentLayout large =
        PrivacyItemViewAdornment::layout(QRectF(0, 0, 128, 128), 2.0);

    QVERIFY(light.adorned != dark.adorned);
    QVERIFY(changedPixelCount(highDpi.baseline, highDpi.adorned) > 0);
    QVERIFY(large.lockBadgeRect.width() > small.lockBadgeRect.width());
    QVERIFY(large.outerBorderWidth > small.outerBorderWidth);
    QVERIFY(QRectF(0, 0, 128, 128).contains(large.lockBadgeRect));
}

void PrivacyItemViewAdornmentTest::testAccessibleStatusToolTip()
{
    const QString original =
        QLatin1String("<qt><table><tr><td>Fixture</td></tr></table></qt>");
    const QString lockedStatus = PrivacyItemViewAdornment::statusText(
                                     PrivacyItemViewState::ProtectedLocked);
    const QString unlockedStatus = PrivacyItemViewAdornment::statusText(
                                       PrivacyItemViewState::ProtectedUnlocked);
    const QString locked = PrivacyItemViewAdornment::withStatusToolTip(
                               original, PrivacyItemViewState::ProtectedLocked);
    const QString unlocked = PrivacyItemViewAdornment::withStatusToolTip(
                                 original, PrivacyItemViewState::ProtectedUnlocked);

    QVERIFY(!lockedStatus.isEmpty());
    QVERIFY(!unlockedStatus.isEmpty());
    QVERIFY(locked.contains(lockedStatus.toHtmlEscaped()));
    QVERIFY(unlocked.contains(unlockedStatus.toHtmlEscaped()));
    QCOMPARE(PrivacyItemViewAdornment::withStatusToolTip(
                 original, PrivacyItemViewState::Unprotected), original);
}

QTEST_GUILESS_MAIN(PrivacyItemViewAdornmentTest)

#include "privacyitemviewadornment_utest.moc"
