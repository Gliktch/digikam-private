/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : A tab to display import item information
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"
#include "camiteminfo.h"
#include "dexpanderbox.h"

namespace Digikam
{

class ImportItemPropertiesTab : public DExpanderBox
{
    Q_OBJECT

public:

    explicit ImportItemPropertiesTab(QWidget* const parent);
    ~ImportItemPropertiesTab() override;

    void setCurrentItem(const CamItemInfo& itemInfo = CamItemInfo(),
                        DMetadata* const meta = nullptr);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
