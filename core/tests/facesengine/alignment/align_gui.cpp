/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face alignment CLI tool
 *
 * SPDX-FileCopyrightText: 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>
#include <QLabel>
#include <QGraphicsScene>

// Local includes

#include "qtopencv.h"
#include "funnelreal.h"
#include "digikam_debug.h"

using namespace Digikam;

QStringList toPaths(char** const argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex ; i < argc ; ++i)
    {
        files << QString::fromLocal8Bit(argv[i]);
    }

    return files;
}

QList<cv::Mat> toImages(const QStringList& paths)
{
    QList<cv::Mat> images;

    for (const QString& path : std::as_const(paths))
    {
        QByteArray s = path.toLocal8Bit();
        images << cv::imread(std::string(s.data()));
    }

    return images;
}

// --------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN OpenCVSideBySideDisplayAlign
{
public:

    explicit OpenCVSideBySideDisplayAlign(int rows, int uiSize = 200)
        : bigImage  (cv::Mat::zeros(uiSize * rows, 2 * uiSize, CV_8UC3)),
          uiSize    (uiSize)
    {
    }

    void add(const cv::Mat& left, const cv::Mat& right)
    {
        // Draw images side-by-side for later display

        QSize size(left.cols, left.rows);
        size.scale(uiSize, uiSize, Qt::KeepAspectRatio);
        cv::Size scaleSize(size.width(), size.height());

        const int top = currentRow * uiSize;
        cv::Mat scaledLeft;
        cv::Mat scaledRight;
        cv::resize(left,  scaledLeft,  scaleSize);
        cv::resize(right, scaledRight, scaleSize);

        if (scaledLeft.channels() == 1)
        {
            cv::cvtColor(scaledLeft, scaledLeft, CV_GRAY2BGR);
        }

        if (scaledRight.channels() == 1)
        {
            cv::cvtColor(scaledRight, scaledRight, CV_GRAY2BGR);
        }

        scaledLeft.copyTo(bigImage.colRange(0, scaledLeft.cols).rowRange(top, top + scaledLeft.rows));
        scaledRight.copyTo(bigImage.colRange(uiSize, uiSize + scaledRight.cols).rowRange(top, top + scaledRight.rows));

        ++currentRow;
    }

    void show()
    {
        QLabel label;
        label.setPixmap(QtOpenCV::cvMatToQPixmap(bigImage));
        label.show();
    }

public:

    cv::Mat   bigImage;
    const int uiSize;
    int       currentRow = 0;
};

// --------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Bad Arguments!!!\nUsage: " << argv[0] << " align <image1> <image2> ... ";
        return 0;
    }

    QApplication app(argc, argv);

    QStringList paths     = toPaths(argv, 1, argc);
    QList<cv::Mat> images = toImages(paths);

    QElapsedTimer timer;
    timer.start();

    FunnelReal funnel;
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup of Aligner took " << timer.restart();

    OpenCVSideBySideDisplayAlign display(images.size());

    for (const cv::Mat& image : std::as_const(images))
    {
        cv::Mat aligned = funnel.align(image);
        display.add(image, aligned);
    }

    int elapsed = timer.elapsed();
    qCDebug(DIGIKAM_TESTS_LOG) << "Alignment took " << elapsed << " for " << images.size() << " , "
                               << ((float)elapsed / images.size()) << " per image";

    display.show();
    app.exec();

    return 0;
}
