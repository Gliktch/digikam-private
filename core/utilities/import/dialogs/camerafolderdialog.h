/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-24
 * Description : a dialog to select a camera folders.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDialog>

namespace Digikam
{

class CameraFolderItem;

class CameraFolderDialog : public QDialog
{
    Q_OBJECT

public:

    CameraFolderDialog(QWidget* const parent, const QMap<QString, int>& map,
                       const QString& cameraName, const QString& rootPath);
    ~CameraFolderDialog() override;

    QString selectedFolderPath() const;

private Q_SLOTS:

    void slotFolderPathSelectionChanged(CameraFolderItem* item);
    void slotHelp();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
