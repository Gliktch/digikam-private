/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : Web Service Tool dialog
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// KDE includes

#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "digikam_export.h"
#include "dplugindialog.h"

class QAbstractButton;
class QPushButton;

namespace Digikam
{

class DIGIKAM_EXPORT WSToolDialog : public DPluginDialog
{
    Q_OBJECT

public:

    explicit WSToolDialog(QWidget* const parent, const QString& objName);
    ~WSToolDialog() override;

    void setMainWidget(QWidget* const widget);

    void setRejectButtonMode(QDialogButtonBox::StandardButton button);

    QPushButton* startButton() const;

    void addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role);

private Q_SLOTS:

    void slotCloseClicked();

Q_SIGNALS:

    void cancelClicked();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
