/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-11-13
 * Description : threaded image analys class.
 *               this class is dedicated to run algorithm in a separated thread
 *               over an image to process analys. No image data are changed.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT DImgThreadedAnalyser : public DImgThreadedFilter
{
    Q_OBJECT

public:

    /**
     * @brief Constructs a filter without argument.
     *        You need to call setupFilter() and startFilter()
     *        to start the threaded computation.
     *        To run filter without to use multithreading, call startFilterDirectly().
     *
     * @warning Versioning is not supported in this class
     */
    explicit DImgThreadedAnalyser(QObject* const parent = nullptr, const QString& name = QString());

    /**
     * @brief Constructs an image analyser with all arguments (ready to use).
     * The given original image will be copied.
     * You need to call startFilter() to start the threaded computation.
     * To run analyser without to use multithreading, call startFilterDirectly().
     */
    explicit DImgThreadedAnalyser(DImg* const orgImage, QObject* const parent = nullptr,
                                  const QString& name = QString());

    ~DImgThreadedAnalyser() override;

private:

    FilterAction filterAction() override
    {
        // return null object
        return FilterAction();
    };

    void readParameters(const FilterAction&) override
    {
         // Do nothing.
    };

    QString filterIdentifier() const override
    {
        // return null object
        return QString();
    };

    QList<int> supportedVersions() const     override
    {
        // return null object
        return QList<int>();
    };

    void prepareDestImage()                  override
    {
        // No destination image is required here.
    };

    void filterImage()                       override
    {
        startAnalyse();
    };

protected:

    /**
     * @brief Main image analys method. Override in subclass.
     */
    virtual void startAnalyse() = 0;
};

} // namespace Digikam
