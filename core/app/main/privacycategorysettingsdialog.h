/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * SPDX-FileCopyrightText: 2026 by digiKam Private contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// C++ includes

#include <memory>

// Qt includes

#include <QDialog>

namespace Digikam
{

class PrivacyCategorySettingsDialog : public QDialog
{
public:

    explicit PrivacyCategorySettingsDialog(QWidget* parent = nullptr);
    ~PrivacyCategorySettingsDialog() override;

protected:

    void reject() override;

private:

    class Private;
    std::unique_ptr<Private> d;
};

} // namespace Digikam
