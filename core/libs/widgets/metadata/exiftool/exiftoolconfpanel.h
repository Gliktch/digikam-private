/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-29
 * Description : ExifTool configuration panel.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "searchtextbar.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifToolConfPanel : public QWidget
{
    Q_OBJECT

public:

    explicit ExifToolConfPanel(QWidget* const parent = nullptr);
    ~ExifToolConfPanel() override;

    QString exifToolDirectory() const;
    void setExifToolDirectory(const QString& dir);

Q_SIGNALS:

    void signalExifToolSettingsChanged(bool available);

public Q_SLOTS:

    void slotStartFoundExifTool();

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);
    void slotExifToolBinaryFound(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
