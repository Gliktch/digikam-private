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
#include "dnnyolodetector.h"

using namespace Digikam;

QCommandLineParser* parseOptions(const QCoreApplication& app)
{
    QCommandLineParser* const parser = new QCommandLineParser();
    parser->addOption(QCommandLineOption(QLatin1String("dataset"),
                                         QLatin1String("Data set folder"),
                                         QLatin1String("path relative to data folder")));
    parser->addHelpOption();
    parser->process(app);

    return parser;
}

void showObjects(const QString& imagePath, const QHash<QString, QVector<QRect> >& detectedBoxes)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Loading" << imagePath;
    QImage img(imagePath);

    QWidget* const mainWidget     = new QWidget;
    QScrollArea* const scrollArea = new QScrollArea;
    scrollArea->setWidget(mainWidget);
    scrollArea->setWidgetResizable(true);

    QHBoxLayout* const layout     = new QHBoxLayout(mainWidget);
    QLabel* const fullImage       = new QLabel;
    fullImage->setScaledContents(true);
    layout->addWidget(fullImage);

    // Draw rect objects

    QPainter painter(&img);
    QPen paintPen(Qt::red);
    paintPen.setWidth(1);
    painter.setPen(paintPen);

    for (QHash<QString, QVector<QRect> >::const_iterator it = detectedBoxes.constBegin() ;
        it != detectedBoxes.constEnd() ; ++it)
    {
        for (auto rectDraw : it.value())
        {
            painter.drawRect(rectDraw);
        }
    }

    // Only setPixmap after finishing drawing bboxes around detected objects

    fullImage->setPixmap(QPixmap::fromImage(img));

    scrollArea->show();
    scrollArea->setWindowTitle(imagePath);
    qApp->processEvents(); // dirty hack
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

    std::vector<cv::Mat> cvImages;

    for (int j = 0 ; j < filesInfo.size() ; j++)
    {
        cv::Mat cvImage = cv::imread(filesInfo[j].absoluteFilePath().toStdString());
        cvImages.push_back(cvImage);
    }

    int batch_size = 16;
    QList<QHash<QString, QVector<QRect> > > results;
    QScopedPointer<DNNYoloDetector> yoloDetector (new DNNYoloDetector());

    int s = filesInfo.size() - filesInfo.size() % batch_size;

    for (int i = 0 ; i < (s - batch_size + 1) ; i += batch_size)
    {
        QElapsedTimer timer;
        timer.start();
        std::vector<cv::Mat> cvBatchImages;

        for (int j = 0 ; j < batch_size ; j++)
        {
            cvBatchImages.push_back(cvImages[i+j]);
        }

        // std::vector<cv::Mat> outs = yoloDetector->preprocess(cvBatchImages);

        QList<QHash<QString, QVector<QRect> > > resBatch = yoloDetector->detectObjects(cvBatchImages);
        int elapsed = timer.elapsed();

        qCDebug(DIGIKAM_TESTS_LOG) << "detected took:" << elapsed << " ms";
/*
        qCDebug(DIGIKAM_TESTS_LOG) << yoloDetector->showInferenceTime();
*/
        results += resBatch;
    }

    for (int i = 0 ; i < s ; i++)
    {
        if ((i < filesInfo.size()) && (i < results.size()))
        {
            showObjects(filesInfo[i].absoluteFilePath(), results[i]);
        }
    }

    return app.exec();
}
