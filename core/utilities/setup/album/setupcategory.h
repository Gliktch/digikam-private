/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : album category setup tab.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupCategory : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupCategory(QWidget* const parent = nullptr);
    ~SetupCategory() override;

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotCategorySelectionChanged();
    void slotAddCategory();
    void slotDelCategory();
    void slotRepCategory();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
