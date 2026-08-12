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

// Qt includes

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QProcess* const ocrProcess = new QProcess();
    ocrProcess->setProcessChannelMode(QProcess::SeparateChannels);
    QStringList args;

    // ------------------------- IN/OUT ARGUMENTS -------------------------

    QString pathImage       = QLatin1String("../../core/tests/ocrtextconverter/data/");
    QString pathToSaveText  = QLatin1String("../../core/tests/ocrtextconverter/data/");
    QString inputFileName   = pathImage.append(QLatin1String("scanned_img.jpg"));
    QString outputTextName  = pathToSaveText.append(QLatin1String("scanned_img"));

    // add configuration image

    if (!inputFileName.isEmpty())
    {
        args << inputFileName;
    }

    // output base name

    if (!outputTextName.isEmpty())
    {
        args << outputTextName;
    }
    else
    {
        args << QLatin1String("stdout");
    }

    // ----------------------------- OPTIONS -----------------------------

    QString lang       = QLatin1String("eng");
    QString psm        = QLatin1String("3");
    QString oem        = QLatin1String("3");
    QString dpi        = QLatin1String("");
    QUrl userWords     = QUrl(QLatin1String(" "));
    QUrl userPatterns  = QUrl(QLatin1String(" "));

    // User words

    if (!userWords.isValid())
    {
        args << QLatin1String("--user-words") << userWords.toLocalFile();
    }

    // User patterns

    if (!userPatterns.isValid())
    {
        args << QLatin1String("--user-patterns") << userPatterns.toLocalFile();
    }

    // page Segmentation mode

    if (!psm.isEmpty())
    {
        args << QLatin1String("--psm") << psm;
    }

    // OCR engine mode

    if (!oem.isEmpty())
    {
        args << QLatin1String("--oem") << oem;
    }

    // Language

    if (!lang.isEmpty())
    {
        args << QLatin1String("-l") << lang;
    }

    // Specify DPI for input image
    // Tesseract work best with 300 dpi for input image

    if (!dpi.isEmpty())
    {
        args << QLatin1String("--dpi") << dpi;
    }

    // ------------------  Running tesseract process ------------------

    const QString cmd = QLatin1String("tesseract");

    ocrProcess->setProgram(cmd);
    ocrProcess->setArguments(args);

    qDebug() << "Running OCR : "
             << ocrProcess->program()
             << ocrProcess->arguments();

    ocrProcess->start();

    bool successFlag = (ocrProcess->waitForFinished(-1) && (ocrProcess->exitStatus() == QProcess::NormalExit));

    if (!successFlag)
    {
        qWarning() << "Error starting OCR Process";
        return 0;
    }

    QString output   = QString::fromLocal8Bit(ocrProcess->readAllStandardOutput());

    qDebug() << output;

    delete ocrProcess;

    return 0;
}
