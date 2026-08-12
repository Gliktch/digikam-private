/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2024-11-10
 * Description : Foundation for all machine learning pipelines
 *
 * SPDX-FileCopyrightText: 2024-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QIcon>
#include <QMetaType>
#include <QExplicitlySharedDataPointer>


// Local includes

#include "digikam_export.h"
#include "dimg.h"

namespace Digikam
{

class DIGIKAM_EXPORT MLPipelinePackageNotify : public QSharedData
{

public:

    typedef QExplicitlySharedDataPointer<MLPipelinePackageNotify> Ptr;

public:

    MLPipelinePackageNotify();

    explicit MLPipelinePackageNotify(const QString& _name,
                                     const QString& _path,
                                     const QString& _displayData,
                                     int _processed,
                                     const DImg& _thumbnail);

    explicit MLPipelinePackageNotify(const QString& _name,
                                     const QString& _path,
                                     const QString& _displayData,
                                     int _processed,
                                     const QIcon& _thumbnail);

    ~MLPipelinePackageNotify() = default;

    MLPipelinePackageNotify(const MLPipelinePackageNotify& other);
    MLPipelinePackageNotify& operator=(const MLPipelinePackageNotify&);

public:

    QString name;
    QString path;
    QString displayData;
    int     processed       = 0;
    QIcon   thumbnail;
};

} // namespace Digikam

/// To be able to use MLPipelinePackageNotify::Ptr with Qt signal/slots.
Q_DECLARE_METATYPE(Digikam::MLPipelinePackageNotify::Ptr)
