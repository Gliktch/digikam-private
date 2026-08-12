/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser - private container
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "imagequalityparser.h"

// C++ includes

#include <cmath>
#include <cfloat>
#include <cstdio>

// Qt includes

#include <QTextStream>
#include <QFile>
#include <QImage>
#include <QScopedPointer>
#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrentRun>

// Local includes

#include "digikam_opencv.h"
#include "digikam_debug.h"
#include "mixerfilter.h"
#include "nrfilter.h"
#include "nrestimate.h"
#include "exposurecontainer.h"
#include "noise_detector.h"
#include "exposure_detector.h"
#include "compression_detector.h"
#include "blur_detector.h"
#include "aesthetic_detector.h"
#include "imagequalitythread.h"
#include "imagequalitycalculator.h"

// To switch on/off log trace file.
// #define TRACE 1

using namespace cv;

namespace Digikam
{

class Q_DECL_HIDDEN ImageQualityParser::Private
{
public:

    explicit Private()
        : calculator(new ImageQualityCalculator())
    {
    }

    ~Private()
    {
        delete calculator;
    }

public:

    DImg                    image;                  ///< original image

    ImageQualitySettings    imq;

    ImageQualityCalculator* calculator = nullptr;

    PickLabel*              label      = nullptr;

    volatile bool           running    = true;
};

} // namespace Digikam
