/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face detection CLI tool
 *
 * SPDX-FileCopyrightText: 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QImage>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QElapsedTimer>
#include <QRectF>
#include <QList>
#include <QScrollArea>
#include <QPainter>
#include <QtGlobal>

// Local includes

#include "digikam_debug.h"
#include "facedetector.h"

using namespace Digikam;

void detectFaces(const QString& imagePath)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Loading " << imagePath;
    QImage img(imagePath);
    QImage imgScaled(img.scaled(416, 416, Qt::KeepAspectRatio));

    FaceDetector detector;
    qCDebug(DIGIKAM_TESTS_LOG) << "Detecting faces";

    QElapsedTimer timer;
    unsigned int elapsedDetection = 0;

    timer.start();
    QList<QRectF> faces = detector.detectFaces(imagePath);
    elapsedDetection = timer.elapsed();

    qCDebug(DIGIKAM_TESTS_LOG) << "(Input CV) Found" << faces.size() << "faces, in" << elapsedDetection << "ms";

    if (faces.isEmpty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "No faces found";
        return;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "Coordinates of detected faces:";

    for (const QRectF& r : std::as_const(faces))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << r;
    }

    QWidget* const mainWidget     = new QWidget;

    QScrollArea* const scrollArea = new QScrollArea;
    scrollArea->setWidget(mainWidget);
    scrollArea->setWidgetResizable(true);

    QHBoxLayout* const layout     = new QHBoxLayout(mainWidget);
    QLabel* const fullImage       = new QLabel;
    fullImage->setScaledContents(true);
    layout->addWidget(fullImage);

    QPainter painter(&imgScaled);
    QPen paintPen(Qt::green);
    paintPen.setWidth(1);
    painter.setPen(paintPen);

    for (const QRectF& rr : std::as_const(faces))
    {
        QLabel* const label = new QLabel;
        label->setScaledContents(false);
        QRect rectDraw      = FaceDetector::toAbsoluteRect(rr, imgScaled.size());
        QRect r             = FaceDetector::toAbsoluteRect(rr, img.size());
        QImage part         = img.copy(r);
        label->setPixmap(QPixmap::fromImage(part.scaled(qMin(img.size().width(), 50),
                                                        qMin(img.size().width(), 50),
                                                        Qt::KeepAspectRatio)));
        layout->addWidget(label);
        painter.drawRect(rectDraw);
    }

    // Only setPixmap after finishing drawing bboxes around detected faces.

    fullImage->setPixmap(QPixmap::fromImage(imgScaled));

    scrollArea->show();
    scrollArea->setWindowTitle(imagePath);
    qApp->processEvents(); // Dirty hack.
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Bad Arguments!!!\nUsage:" << argv[0] << "<image1> <image2>...";
        return 0;
    }

    QApplication app(argc, argv);

    for (int i = 1 ; i < argc ; ++i)
    {
        detectFaces(QString::fromLocal8Bit(argv[i]));
    }

    app.exec();

    return 0;
}
