/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup Misc tab.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupMisc : public QScrollArea
{
    Q_OBJECT

public:

    enum SortOrder
    {
        SortByDate = 0,
        SortByName,
        SortByFileSize
    };

    enum MiscTab
    {
        Behavior = 0,
        Appearance,
        SpellCheck,
        Localize,
        System
    };

public:

    explicit ShowfotoSetupMisc(QWidget* const parent = nullptr);
    ~ShowfotoSetupMisc()      override;

    void setActiveTab(MiscTab tab);
    MiscTab activeTab() const;

    bool checkSettings();
    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
