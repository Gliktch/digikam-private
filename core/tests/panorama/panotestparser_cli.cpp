/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-24
 * Description : a test program for the pto parser
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "ptofile.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    QString ptoFile(QString::fromLocal8Bit(argv[1]));

    PTOFile file(QLatin1String("2014.0"));
    file.openFile(ptoFile);

    PTOType* const ptoData = file.getPTO();
    delete ptoData;

    return 0;
}
