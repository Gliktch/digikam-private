/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

class SetupDatabase : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupDatabase(QWidget* const parent = nullptr);
    ~SetupDatabase() override;

    void applySettings();

private:

    void readSettings();
    void createUpdateBox();

private Q_SLOTS:

    void slotUpgradeUniqueHashes();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
