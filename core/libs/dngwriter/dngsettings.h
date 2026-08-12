/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG save settings widgets
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNGSettings : public QWidget
{
    Q_OBJECT

public:

    explicit DNGSettings(QWidget* const parent = nullptr);
    ~DNGSettings() override;

    void setCompressLossLess(bool b);
    bool compressLossLess()         const;

    void setPreviewMode(int mode);
    int  previewMode()              const;

    void setBackupOriginalRawFile(bool b);
    bool backupOriginalRawFile()    const;

    void setDefaultSettings();

public Q_SLOTS:

    /// To handle changes from host application Setup dialog.
    void slotSetupChanged();

Q_SIGNALS:

    void signalSettingsChanged();
    void signalSetupExifTool();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
