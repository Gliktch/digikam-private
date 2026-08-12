/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-04-07
 * Description : Solid hardware list dialog
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

class DIGIKAM_EXPORT SolidHardwareDlg : public InfoDlg
{
    Q_OBJECT

public:

    explicit SolidHardwareDlg(QWidget* const parent);
    ~SolidHardwareDlg()         override;

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);
    void slotCopy2ClipBoard()   override;
    void slotPopulateDevices();

    void slotDeviceAdded(const QString&);
    void slotDeviceRemoved(const QString&);

private:

    QTreeWidget* currentTreeView() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
