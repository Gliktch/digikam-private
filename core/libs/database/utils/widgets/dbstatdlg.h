/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-28
 * Description : database statistics dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "infodlg.h"
#include "coredbconstants.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT DBStatDlg : public InfoDlg
{
    Q_OBJECT

public:

    explicit DBStatDlg(QWidget* const parent);
    ~DBStatDlg()    override = default;

private Q_SLOTS:

    void slotHelp() override;

private:

    int generateItemsList(DatabaseItem::Category category, const QString& title);
};

} // namespace Digikam
