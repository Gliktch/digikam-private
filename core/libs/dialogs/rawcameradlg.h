/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-04-07
 * Description : Raw camera list dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "infodlg.h"
#include "searchtextbar.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RawCameraDlg : public InfoDlg
{
    Q_OBJECT

public:

    explicit RawCameraDlg(QWidget* const parent);
    ~RawCameraDlg() override;

private:

    void updateHeader(int results=0);

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
