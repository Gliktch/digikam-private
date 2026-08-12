/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * SPDX-FileCopyrightText: 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupMisc : public QScrollArea
{
    Q_OBJECT

public:

    enum MiscTab
    {
        Behavior = 0,
        Appearance,
        Grouping,
        SpellCheck,
        Localize,
        System
    };

public:

    explicit SetupMisc(QWidget* const parent = nullptr);
    ~SetupMisc() override;

    void setActiveTab(MiscTab tab);
    MiscTab activeTab() const;

    bool checkSettings();
    void applySettings();

private:

    void readSettings();

    void setupBehavior();
    void setupAppearance();
    void setupGrouping();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
