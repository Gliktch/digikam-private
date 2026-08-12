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

#include "mlpipelinepackagenotify.h"

namespace Digikam
{

MLPipelinePackageNotify::MLPipelinePackageNotify()
    : QSharedData()
{
}

MLPipelinePackageNotify::MLPipelinePackageNotify(const QString& _name,
                                                 const QString& _path,
                                                 const QString& _displayData,
                                                 int _processed,
                                                 const DImg& _thumbnail)
    : QSharedData(),
      name       (_name),
      path       (_path),
      displayData(_displayData),
      processed  (_processed)
{
    if (!_thumbnail.isNull())
    {
        thumbnail = QIcon(_thumbnail.smoothScale(48, 48, Qt::KeepAspectRatio).convertToPixmap());
    }
}

MLPipelinePackageNotify::MLPipelinePackageNotify(const QString& _name,
                                                 const QString& _path,
                                                 const QString& _displayData,
                                                 int _processed,
                                                 const QIcon& _thumbnail)
    : QSharedData(),
      name       (_name),
      path       (_path),
      displayData(_displayData),
      processed  (_processed),
      thumbnail  (_thumbnail)
{
}

MLPipelinePackageNotify::MLPipelinePackageNotify(const MLPipelinePackageNotify& other)
    : QSharedData(),
      name       (other.name),
      path       (other.path),
      displayData(other.displayData),
      processed  (other.processed),
      thumbnail  (other.thumbnail)
{
}

MLPipelinePackageNotify& MLPipelinePackageNotify::operator=(const MLPipelinePackageNotify& other)
{
    name        = other.name;
    path        = other.path;
    displayData = other.displayData;
    processed   = other.processed;
    thumbnail   = other.thumbnail;

    return *this;
}

} // namespace Digikam
