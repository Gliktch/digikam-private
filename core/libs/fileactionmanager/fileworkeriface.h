/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : file worker interface
 *
 * SPDX-FileCopyrightText: 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "fileactionmngr.h"
#include "fileactionimageinfolist.h"
#include "iteminfo.h"
#include "workerobject.h"

namespace Digikam
{

class MetadataHub;

class FileWorkerInterface : public WorkerObject         // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

public Q_SLOTS:

    virtual void writeOrientationToFiles(const FileActionItemInfoList&, int) {};
    virtual void writeMetadataToFiles(const FileActionItemInfoList&)         {};
    virtual void writeMetadata(const FileActionItemInfoList&, int)           {};
    virtual void transform(const FileActionItemInfoList&, int)               {};

Q_SIGNALS:

    void imageDataChanged(const QString& path, bool removeThumbnails, bool notifyCache);
    void imageChangeFailed(const QString& message, const QStringList& fileNames);
};

// ---------------------------------------------------------------------------------------------

class FileActionMngrFileWorker : public FileWorkerInterface
{
    Q_OBJECT

public:

    explicit FileActionMngrFileWorker(FileActionMngr::Private* const dd)
        : d(dd)
    {
    }

public:

    void writeOrientationToFiles(const FileActionItemInfoList& infos, int orientation) override;
    void writeMetadataToFiles(const FileActionItemInfoList& infos)                     override;
    void writeMetadata(const FileActionItemInfoList& infos, int flags)                 override;
    void transform(const FileActionItemInfoList& infos, int orientation)               override;

private:

    FileActionMngr::Private* const d = nullptr;
};

} // namespace Digikam
