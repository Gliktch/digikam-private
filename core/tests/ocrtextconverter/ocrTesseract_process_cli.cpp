/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : CLI tool for OCR Text Converter plugin
 *
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ocrtesseractengine.h"

// Qt includes

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QApplication>
#include <QDir>

using namespace DigikamGenericTextConverterPlugin;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QScopedPointer<OcrTesseractEngine> ocrEngine (new OcrTesseractEngine());

    ocrEngine->setInputFile(QLatin1String("./data/scanned_img.jpg"));
    ocrEngine->setOutputFile(QLatin1String("./"));

    int check = ocrEngine->runOcrProcess();
    Q_UNUSED(check);

    return app.exec();
}
