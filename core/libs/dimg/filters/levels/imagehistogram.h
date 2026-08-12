/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-07-21
 * Description : image histogram manipulation methods.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QEvent>
#include <QThread>

// Local includes

#include "digikam_export.h"
#include "dynamicthread.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT ImageHistogram : public DynamicThread
{
    Q_OBJECT

public:

    explicit ImageHistogram(const DImg& img, QObject* const parent = nullptr);
    ~ImageHistogram() override;

    /**
     * Started computation: synchronous or threaded.
     */
    void calculate();
    void calculateInThread();

    /**
     * Stop threaded computation.
     */
    void stopCalculation();
    bool isCalculating()                               const;

    /**
     * Methods to access the histogram data.
     */
    bool   isSixteenBit()                              const;
    bool   isValid()                                   const;

    double getCount(int channel, int start, int end)   const;
    double getMean(int channel, int start, int end)    const;
    double getPixels()                                 const;
    double getStdDev(int channel, int start, int end)  const;
    double getValue(int channel, int bin)              const;
    double getMaximum(int channel, int start, int end) const;

    int    getHistogramSegments()                      const;
    int    getMaxSegmentIndex()                        const;
    int    getMedian(int channel, int start, int end)  const;

Q_SIGNALS:

    void calculationFinished(bool success);

    /**
     * when calculation in thread is initiated, from other thread
     */
    void calculationAboutToStart();

    /**
     * emitted from calculation thread
     */
    void calculationStarted();

protected:

    void run() override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
