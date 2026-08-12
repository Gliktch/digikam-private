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

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "searchtextbar.h"

// To use connect with SearchTextBar as argument
using namespace Digikam;

class SearchTextBarTest : public QObject
{
    Q_OBJECT

public:

    explicit SearchTextBarTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testHighlighting();
    void testSearchTextSettings();
    void newSearchTextSettings(const SearchTextSettings& settings);

private:

    SearchTextSettings lastSearchTextSettings;
    int                callCount;
};
