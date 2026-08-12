/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-04
 * Description : a command line tool to list macOS application bundle
 *               suitable to open a type mime..
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QUrl>
#include <QList>

// Local includes

#include "digikam_debug.h"
#include "dservicemenu.h"

using namespace Digikam;

int main(int /*argc*/, char** /*argv*/)
{
    QString suffix;
    QList<QUrl> list;

    suffix = QLatin1String("jpg");
    list   = DServiceMenu::MacApplicationForFileExtension(suffix);

    qCDebug(DIGIKAM_TESTS_LOG) << "macOS Bundle urls for" << suffix;
    qCDebug(DIGIKAM_TESTS_LOG) << list;

    suffix = QLatin1String("jpeg");
    list   = DServiceMenu::MacApplicationForFileExtension(suffix);

    qCDebug(DIGIKAM_TESTS_LOG) << "macOS Bundle urls for" << suffix;
    qCDebug(DIGIKAM_TESTS_LOG) << list;

    suffix = QLatin1String("png");
    list   = DServiceMenu::MacApplicationForFileExtension(suffix);

    qCDebug(DIGIKAM_TESTS_LOG) << "macOS Bundle urls for" << suffix;
    qCDebug(DIGIKAM_TESTS_LOG) << list;

    suffix = QLatin1String("tiff");
    list   = DServiceMenu::MacApplicationForFileExtension(suffix);

    qCDebug(DIGIKAM_TESTS_LOG) << "macOS Bundle urls for" << suffix;
    qCDebug(DIGIKAM_TESTS_LOG) << list;

    suffix = QLatin1String("tif");
    list   = DServiceMenu::MacApplicationForFileExtension(suffix);

    qCDebug(DIGIKAM_TESTS_LOG) << "macOS Bundle urls for" << suffix;
    qCDebug(DIGIKAM_TESTS_LOG) << list;

    return 0;
}
