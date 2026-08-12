/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for displaying only a range of a token result
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "modifier.h"
#include "ruledialog.h"

namespace Ui
{
    class RangeModifierDialogWidget;
}

namespace Digikam
{

class RangeDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit RangeDialog(Rule* const parent, QWidget* const widget);
    ~RangeDialog()                                                                        override;

public:

    Ui::RangeModifierDialogWidget* const ui = nullptr;

private Q_SLOTS:

    void slotToTheEndChecked(bool checked);

private:

    /// @note disabled
    explicit RangeDialog(QWidget*)             = delete;
    RangeDialog(const RangeDialog&)            = delete;
    RangeDialog& operator=(const RangeDialog&) = delete;
};

// --------------------------------------------------------

class RangeModifier : public Modifier
{
    Q_OBJECT

public:

    explicit RangeModifier(QWidget* const widget);
    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private Q_SLOTS:

    void slotTokenTriggered(const QString& token)                                         override;

private:

    /// @note disabled
    explicit RangeModifier(QObject*)               = delete;
    RangeModifier(const RangeModifier&)            = delete;
    RangeModifier& operator=(const RangeModifier&) = delete;
};

} // namespace Digikam
