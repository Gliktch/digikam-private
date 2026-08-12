/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : general info list dialog
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QDialog>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT InfoDlg : public QDialog
{
    Q_OBJECT

public:

    explicit InfoDlg(QWidget* const parent);
    ~InfoDlg();

    virtual void setInfoMap(const QMap<QString, QString>& list);

    QTabWidget*       tabView()    const;
    QTreeWidget*      listView()   const;
    QWidget*          mainWidget() const;
    QDialogButtonBox* buttonBox()  const;

private Q_SLOTS:

    virtual void slotHelp();            // cppcheck-suppress virtualCallInConstructor
    virtual void slotCopy2ClipBoard();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
