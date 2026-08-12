/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "option.h"
#include "ruledialog.h"

namespace Ui
{
    class SequenceNumberOptionDialogWidget;
}

namespace Digikam
{

class SequenceNumberDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit SequenceNumberDialog(Rule* const parent, QWidget* const widget);
    ~SequenceNumberDialog()       override;

public:

    Ui::SequenceNumberOptionDialogWidget* const ui = nullptr;

protected:

    void showEvent(QShowEvent* e) override;

private:

    /// @note disabled
    explicit SequenceNumberDialog(QWidget*)                      = delete;
    SequenceNumberDialog(const SequenceNumberDialog&)            = delete;
    SequenceNumberDialog& operator=(const SequenceNumberDialog&) = delete;
};

// --------------------------------------------------------

class SequenceNumberOption : public Option
{
    Q_OBJECT

public:

    explicit SequenceNumberOption(QWidget* const widget);
    ~SequenceNumberOption()                                      override = default;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override;

private Q_SLOTS:

    void slotTokenTriggered(const QString& token)                override;

private:

    /// @note disabled
    explicit SequenceNumberOption(QObject*)                      = delete;
    SequenceNumberOption(const SequenceNumberOption&)            = delete;
    SequenceNumberOption& operator=(const SequenceNumberOption&) = delete;
};

} // namespace Digikam
