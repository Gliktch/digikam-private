/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-28
 * Description : a dialog for showing the advancedrename tooltip
 *
 * SPDX-FileCopyrightText: 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

namespace Digikam
{

class TooltipDialog : public QDialog
{
    Q_OBJECT

public:

    explicit TooltipDialog(QWidget* const parent);
    ~TooltipDialog() override;

    void setTooltip(const QString& tooltip);
    void clearTooltip();

private:

    /// @note disabled
    TooltipDialog(const TooltipDialog&)            = delete;
    TooltipDialog& operator=(const TooltipDialog&) = delete;

private Q_SLOTS:

    void slotHelp();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
