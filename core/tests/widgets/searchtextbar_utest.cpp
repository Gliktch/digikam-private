/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-12
 * Description : a test for the searchtextbar
 *
 * SPDX-FileCopyrightText: 2010 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchtextbar_utest.h"

// Qt includes

#include <QStandardItemModel>
#include <QTest>
#include <qtestkeyboard.h>

// Local includes


using namespace Digikam;
using namespace QTest;

QTEST_MAIN(SearchTextBarTest)

SearchTextBarTest::SearchTextBarTest(QObject* const parent)
    : QObject(parent)
{
    callCount = 0;
}

void SearchTextBarTest::testHighlighting()
{
    SearchTextBar textBar(nullptr, QLatin1String("test"));
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    // highlighting shall only occur if text is entered
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    textBar.setText(QLatin1String("test"));

    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::HAS_RESULT);
    textBar.slotSearchResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NO_RESULT);

    textBar.setHighlightOnResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);
    textBar.slotSearchResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    textBar.setHighlightOnResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::HAS_RESULT);
    textBar.slotSearchResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NO_RESULT);

    // re-eetting highlighting to true must not change highlight state
    textBar.setHighlightOnResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NO_RESULT);
}

void SearchTextBarTest::testSearchTextSettings()
{
    SearchTextBar textBar(nullptr, QLatin1String("test"));

    SearchTextSettings defaultSettings;
    QCOMPARE(textBar.searchTextSettings(), defaultSettings);

    callCount = 0;

    connect(&textBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(newSearchTextSettings(SearchTextSettings)));

    const QString textEntered = QLatin1String("hello world");
    keyClicks(&textBar, textEntered);

    QTest::qWait(2000);

    QCOMPARE(textBar.searchTextSettings().caseSensitive, defaultSettings.caseSensitive);
    QCOMPARE(textBar.searchTextSettings().text, textEntered);

    QVERIFY(callCount >= 1);
    QCOMPARE(lastSearchTextSettings.caseSensitive, defaultSettings.caseSensitive);
    QCOMPARE(lastSearchTextSettings.text, textEntered);

    /** @todo test case modifications, but how to click context menu? */
}

void SearchTextBarTest::newSearchTextSettings(const SearchTextSettings& settings)
{
    lastSearchTextSettings = settings;
    callCount++;
}

#include "moc_searchtextbar_utest.cpp"
