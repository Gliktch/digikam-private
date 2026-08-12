/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFlags>
#include <QMap>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"
#include "versionfileoperation.h"
#include "versionmanagersettings.h"
#include "versionnamingscheme.h"

namespace Digikam
{

class DIGIKAM_EXPORT VersionManager
{
public:

    enum FileNameType
    {
        CurrentVersionName,
        NewVersionName
    };

public:

    explicit VersionManager();
    virtual ~VersionManager();

    void setSettings(const VersionManagerSettings& settings);
    VersionManagerSettings settings() const;

    void setNamingScheme(VersionNamingScheme* scheme);
    VersionNamingScheme* namingScheme() const;

    VersionFileOperation operation(FileNameType request, const VersionFileInfo& loadedFile,
                                   const DImageHistory& initialResolvedHistory,
                                   const DImageHistory& currentHistory);

    VersionFileOperation operationNewVersionInFormat(const VersionFileInfo& loadedFile,
                                   const QString& format,
                                   const DImageHistory& initialResolvedHistory,
                                   const DImageHistory& currentHistory);

    VersionFileOperation operationNewVersionAs(const VersionFileInfo& loadedFile,
                                               const VersionFileInfo& saveLocation,
                                               const DImageHistory& initialResolvedHistory,
                                               const DImageHistory& currentHistory);

    virtual QString toplevelDirectory(const QString& path);

    virtual QStringList workspaceFileFormats() const;

private:

    /// @note disabled
    VersionManager(const VersionManager&)            = delete;
    VersionManager& operator=(const VersionManager&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
