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

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc < 3)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Bad arguments !!!\nUsage: "
                                   << argv[0]
                                   << "<image_path>"
                                   << "<output_image_path>";
        return 0;
    }

    QScopedPointer<DNNYoloDetector> yoloDetector (new DNNYoloDetector());
    QImage img(QString::fromUtf8(argv[1]));
    cv::Mat cvImage = cv::imread(argv[1]);

    if (cvImage.empty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Image Path is not available";
        return 0;
    }

    QHash <QString, QVector<QRect> > detectedBoxes = yoloDetector->detectObjects(cvImage);

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

    img.save(QLatin1String(argv[2]), "PNG");
}
