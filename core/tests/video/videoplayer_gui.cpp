/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a stand alone tool to play a video file.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QUrl>

// Local includes

#include "vidplayerdlg.h"
#include "digikam_debug.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    if (argc == 1)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "player - video file to play";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: video file";

        return -1;
    }

    VidPlayerDlg player(QString::fromUtf8(argv[1]));
    player.show();
    player.resize(800, 600);

    return a.exec();
}
