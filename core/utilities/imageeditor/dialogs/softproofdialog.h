/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-16
 * Description : Dialog to adjust soft proofing settings
 *
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class SoftProofDialog : public QDialog
{
    Q_OBJECT

public:

    explicit SoftProofDialog(QWidget* const parent);
    ~SoftProofDialog()                    override;

    bool shallEnableSoftProofView() const;

protected:

    void readSettings();
    void writeSettings();

protected Q_SLOTS:

    void updateGamutCheckState();
    void updateOkButtonState();

private Q_SLOTS:

    void slotHelp();
    void slotOk();
    void slotProfileInfo();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
