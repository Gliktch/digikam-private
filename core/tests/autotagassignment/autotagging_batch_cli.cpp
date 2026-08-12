/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-13
 * Description : Integrated testing tool for bencmark deployed model and hyper parameters
 *               for object detection
 * SPDX-FileCopyrightText: 2023 by Quoc Hung TRAN <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ include

#include <iostream>

// Qt includes

#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QLineEdit>
#include <QPushButton>
#include <QHash>
#include <QFileInfo>
#include <QRandomGenerator>

// Local includes

#include "digikam_debug.h"
#include "dnnbasedetectormodel.h"
#include "autotagsassign.h"

using namespace Digikam;

QCommandLineParser* parseOptions(const QCoreApplication& app)
{
    QCommandLineParser* const parser = new QCommandLineParser();
    parser->addOption(QCommandLineOption(QStringList() << QLatin1String("dataset"),
                                                          QLatin1String("Data set folder"),
                                                          QLatin1String("path relative to data folder")));
    parser->addHelpOption();
    parser->process(app);

    return parser;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // Options for commandline parser

    QCommandLineParser* const parser = parseOptions(app);

    if (!parser->isSet(QLatin1String("dataset")))
    {
        qWarning(DIGIKAM_TESTS_LOG) << "Data set is not set !!!";
    }

    QDir dataset(parser->value(QLatin1String("dataset")));
    QFileInfoList filesInfo = dataset.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QScopedPointer<AutotagsAssign> autotagsEngine (new AutotagsAssign(AutotagsScanSettings::RESNET50));
    QList<QList<QString> > tags;

    int batch_size = 1;
    int sizeData   = 54;

    // 54 else 0...53
    // i = 0, 0...9 || i = 40, 40...49 || i = 50, 50...53

    for (int i = 0 ; i <= (sizeData - sizeData % batch_size - batch_size) ; i += batch_size)
    {
        QList<QString> bacthImgs;

        for (int j = 0 ; j < batch_size ; j++)
        {
            bacthImgs.push_back(filesInfo[i + j].absoluteFilePath());
        }

        tags += autotagsEngine->generateTagsList(bacthImgs, batch_size);
    }

    QList<QString> bacthImgs;

    for (int i = (sizeData - sizeData % batch_size) ; i < sizeData ; i++)
    {
        bacthImgs.push_back(filesInfo[i].absoluteFilePath());
    }

    tags += autotagsEngine->generateTagsList(bacthImgs, batch_size);

    for (int i = 0 ; i < sizeData ; i++)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Path:" << filesInfo[i].absoluteFilePath();
        qCDebug(DIGIKAM_TESTS_LOG) << tags[i] << Qt::endl;
    }

    return app.exec();
}
