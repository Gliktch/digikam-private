/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-17-04
 * Description : time adjust images list.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDateTime>
#include <QMap>
#include <QUrl>

// Local includes

#include "ditemslist.h"
#include "timeadjustsettings.h"

using namespace Digikam;

namespace DigikamGenericTimeAdjustPlugin
{

class TimeAdjustList : public DItemsList
{
    Q_OBJECT

public:

    /* The different columns in a list. */
    enum FieldType
    {
        TIMESTAMP_USED     = DItemsListView::User1,
        TIMESTAMP_UPDATED  = DItemsListView::User2,
        STATUS             = DItemsListView::User3
    };

    enum ProcessingStatus
    {
        CLRSTATUS_ERROR = 1 << 0,
        NOPROCESS_ERROR = 1 << 1,
        META_TIME_ERROR = 1 << 2,
        FILE_TIME_ERROR = 1 << 3,
        EXIF_TOOL_ERROR = 1 << 4
    };

public:

    explicit TimeAdjustList(QWidget* const parent);
    ~TimeAdjustList() override = default;

    void setStatus(const QUrl& url,
                   const QDateTime&,
                   const QDateTime&,
                   int status = CLRSTATUS_ERROR);

    void setWaitStatus();
};

} // namespace DigikamGenericTimeAdjustPlugin
