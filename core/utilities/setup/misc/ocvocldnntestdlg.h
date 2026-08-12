/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-04-26
 * Description : Test OpenCV/OpenCL for DNN usage
 *
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025-2026 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT OpenCVOpenCLDNNTestDlg : public QDialog
{
    Q_OBJECT

public:

    explicit OpenCVOpenCLDNNTestDlg(QWidget* const parent);

    ~OpenCVOpenCLDNNTestDlg()       override;

private:

Q_SIGNALS:

    void signalTestFinished(bool result);
    void signalNotification(const QString& message);

private Q_SLOTS:

    void slotStart();
    void slotCancel();
    void slotClose();
    void slotHelp();
    void slotTestFinished(bool result);
    void slotNotification(const QString& message);

private:

    bool runTest();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
