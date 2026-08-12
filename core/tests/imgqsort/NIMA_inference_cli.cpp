/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 07/07/2022
 * Description : a command line tool to calculate quality level using NIMA model
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C++ includes

#include <iostream>
#include <string>
#include <filesystem>

// Qt includes

#include <QApplication>
#include <QDebug>

// OpenCV includes

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/dnn.hpp"

using namespace std;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    cv::Mat img = cv::imread(argv[1]);

    if (img.empty())
    {
        qDebug() << "Parse image to calculate quality score";
        qDebug() << "Usage: <image file>";

        return -1;
    }

    cv::dnn::Net net = cv::dnn::readNetFromTensorflow(argv[2]);

    if (net.empty())
    {
        qDebug() << "Parse net to calculate quality score";
        qDebug() << "Usage: <image file>";

        return -1;
    }

    cv::Mat blob     = cv::dnn::blobFromImage(img, 1, cv::Size(224, 224), cv::Scalar(0, 0, 0), false, false);
    net.setInput(blob);
    cv::Mat out      = net.forward();
    float tmp[10]    = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    cv::Mat scale(1, 10, out.type(), tmp);
    cv::Scalar score = cv::sum(out.mul(scale));

    std::cout << "score : " << score[0] << "\n";
}
