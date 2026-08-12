/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-01-05
 * Description : an unit test to check text online-translators.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QTest>
#include <QList>
#include <QObject>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "donlinetranslator.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    DOnlineTts tts;
    tts.generateUrls(QLatin1String("Hello World!"), DOnlineTranslator::Google, DOnlineTranslator::English);
    QList<QUrl> urls = tts.media();

    qCDebug(DIGIKAM_TESTS_LOG) << urls;

    return 0;
}
