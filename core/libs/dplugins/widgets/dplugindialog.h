/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-12-31
 * Description : digiKam plugin main dialog
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>
#include <QDialogButtonBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DPlugin;

class DIGIKAM_EXPORT DPluginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit DPluginDialog(QWidget* const parent, const QString& objName);
    ~DPluginDialog() override;

    void setPlugin(DPlugin* const tool);

private Q_SLOTS:

    void slotAboutPlugin();
    void slotOnlineHandbook();

protected:

    void showEvent(QShowEvent*) override;

    QDialogButtonBox* m_buttons = nullptr;

private:

    DPlugin*          m_tool    = nullptr;

    Q_DISABLE_COPY(DPluginDialog)
};

} // namespace Digikam
