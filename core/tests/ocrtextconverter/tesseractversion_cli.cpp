/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to get Tesseract version string
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

// Local includes

#include "digikam_debug.h"
#include "tesseractbinary.h"
#include "dbinarysearch.h"

using namespace Digikam;
using namespace DigikamGenericTextConverterPlugin;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QWidget* const host            = new QWidget(nullptr);
    TesseractBinary tbin;
    DBinarySearch* const binWidget = new DBinarySearch(host);
    binWidget->addBinary(tbin);

    binWidget->allBinariesFound();

    QVBoxLayout* const vlay        = new QVBoxLayout(host);
    vlay->addWidget(binWidget);
    host->show();

    app.exec();

    return 0;
}
