/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-29
 * Description : Database cleaner.
 *
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>
#include <QString>
#include <QWidget>
#include <QList>

// Local includes

#include "identity.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"
#include "maintenancetool.h"

namespace Digikam
{

class DbShrinkDialog : public QDialog
{
    Q_OBJECT

public:

    explicit DbShrinkDialog(QWidget* const parent);
    ~DbShrinkDialog() override;

    void setActive(const int pos);
    void setIcon(const int pos, const QIcon& icon);

public Q_SLOTS:

    int exec()        override;

private Q_SLOTS:

    void slotProgressTimerDone();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
