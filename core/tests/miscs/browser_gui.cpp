/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a stand alone tool to browse a web page.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes
#include "digikam_config.h"

#include <QApplication>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "webbrowserdlg.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (argc == 1)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "browser - web page url to show";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: url top open";
        return -1;
    }

    WebBrowserDlg browser(QUrl(QString::fromUtf8(argv[1])), nullptr);
    browser.show();

    return a.exec();
}
