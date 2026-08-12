/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-12
 * Description : parse settings class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QFileInfo>
#include <QString>

// Local includes

#include "iteminfo.h"
#include "parseresults.h"
#include "advancedrenamemanager.h"

namespace Digikam
{

class ParseSettings
{
public:

    /// default constructor
    ParseSettings()
    {
        init();
    };

    explicit ParseSettings(const QString& _parseString)
        : parseString(_parseString)
    {
        init();
    };

    /// ItemInfo constructor
    explicit ParseSettings(const ItemInfo& info)
    {
        init(info);
    };

    ParseSettings(const QString& _parseString, const ItemInfo& info)
        : parseString(_parseString)
    {
        init(info);
    };

    // --------------------------------------------------------

    bool isValid() const
    {
        QFileInfo fi(fileUrl.toLocalFile());

        return fi.isReadable();
    };

public:

    QUrl                     fileUrl;
    QString                  parseString;
    QString                  str2Modify;
    QDateTime                creationTime;
    ParseResults             results;
    ParseResults             invalidModifiers;
    ParseResults::ResultsKey currentResultsKey;

    int                      startIndex                 = 1;
    int                      cutFileName                = 0;
    bool                     useOriginalFileExtension   = true;
    AdvancedRenameManager*   manager                    = nullptr;

private:

    void init()
    {
        startIndex               = 1;
        cutFileName              = 0;
        useOriginalFileExtension = true;
        manager                  = nullptr;
        str2Modify.clear();
    }

    void init(const ItemInfo& info)
    {
        init();
        fileUrl = info.fileUrl();
    }
};

} // namespace Digikam
