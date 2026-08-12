/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a dialog to play video.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt include

#include <QDialog>
#include <QWidget>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VidPlayerDlg : public QDialog
{
    Q_OBJECT

public:

    explicit VidPlayerDlg(const QString& file, QWidget* const parent = nullptr);
    ~VidPlayerDlg() override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
