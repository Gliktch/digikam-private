/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupRaw : public QScrollArea
{
    Q_OBJECT

public:

    enum RAWTab
    {
        RAWBehavior = 0,
        RAWDefaultSettings
    };

public:

    explicit ShowfotoSetupRaw(QWidget* const parent = nullptr);
    ~ShowfotoSetupRaw()      override;

    void setActiveTab(RAWTab tab);
    RAWTab activeTab() const;

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
