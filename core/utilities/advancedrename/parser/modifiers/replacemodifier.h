/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
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

class QCheckBox;

namespace Ui
{
class ReplaceModifierDialogWidget;
}

namespace Digikam
{

class ReplaceDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit ReplaceDialog(Rule* const parent, QWidget* const widget);
    ~ReplaceDialog()                                                                      override;

public:

    Ui::ReplaceModifierDialogWidget* const ui = nullptr;

private:

    /// @note disabled
    explicit ReplaceDialog(QWidget*)               = delete;
    ReplaceDialog(const ReplaceDialog&)            = delete;
    ReplaceDialog& operator=(const ReplaceDialog&) = delete;
};

// --------------------------------------------------------

class ReplaceModifier : public Modifier
{
    Q_OBJECT

public:

    explicit ReplaceModifier(QWidget* const widget);
    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private Q_SLOTS:

    void slotTokenTriggered(const QString& token)                                         override;

private:

    /// @note disabled
    explicit ReplaceModifier(QObject*)                 = delete;
    ReplaceModifier(const ReplaceModifier&)            = delete;
    ReplaceModifier& operator=(const ReplaceModifier&) = delete;
};

} // namespace Digikam
