/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-08-09
 * Description : Generic range boxes, i.e. ranges where a minimum and maximum can be given.
 *
 * SPDX-FileCopyrightText: 2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DIntRangeBox : public QWidget
{
    Q_OBJECT

public:

    explicit DIntRangeBox(QWidget* const parent = nullptr);
    ~DIntRangeBox() override;

    /**
     * This method sets the lower and upper threshold of possible interval minimum and maximum values.
     * @param min the lowest value to which the interval can be expanded.
     * @param max the highest value to which the interval can be expanded.
     */
    void setRange(int min, int max);

    /**
     * This method sets the minimum and maximum of the interval.
     * @param min The minimum value of the interval.
     * @param max The maximum value of the interval.
     */
    void setInterval(int min, int max);

    /**
     * This method sets the suffix for the minimum and maximum value boxes.
     * @param suffix The suffix.
     */
    void setSuffix(const QString& suffix);

    /**
     * This method enables or disables the embedded spinboxes.
     * @param enabled If the interval boxes should be enabled.
     */
    void setEnabled(bool enabled);

    /**
     * This method returns the minimum value of the interval.
     * @returns the minimum value.
     */
    int minValue();

    /**
     * This method returns the maximum value of the interval.
     * @returns the maximum value.
     */
    int maxValue();

Q_SIGNALS:

    void minChanged(int);
    void maxChanged(int);

private Q_SLOTS:

    void slotMinimumChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
