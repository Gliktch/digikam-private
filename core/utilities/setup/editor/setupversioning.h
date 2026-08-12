/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : setup tab for image versioning
 *
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupVersioning : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupVersioning(QWidget* const parent = nullptr);
    ~SetupVersioning() override;

    void applySettings();

    static void losslessFormatToolTip(QString& formatHelp,
                                      bool hasJXLSupport,
                                      bool hasWEBPSupport,
                                      bool hasAVIFSupport);

private:

    void readSettings();

private Q_SLOTS:

    void showNonDestructiveInformation();
    void showFormatInformation();
    void showSnapshotInformation();
    void showViewInformation();
    void enableToggled(bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
