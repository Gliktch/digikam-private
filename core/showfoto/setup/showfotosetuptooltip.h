/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupToolTip : public QScrollArea
{
    Q_OBJECT

public:

    explicit ShowfotoSetupToolTip(QWidget* const parent = nullptr);
    ~ShowfotoSetupToolTip() override;

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
