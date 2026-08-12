/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import filters configuration dialog
 *
 * SPDX-FileCopyrightText: 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "importfilter.h"

namespace Digikam
{

class ImportFilterDlg : public QDialog
{
    Q_OBJECT

public:

    explicit ImportFilterDlg(QWidget* const parent = nullptr);
    ~ImportFilterDlg() override;

    void setData(const Filter& filter);
    void getData(Filter* const filter);

protected Q_SLOTS:

    void fileNameCheckBoxClicked();
    void pathCheckBoxClicked();
    void mimeCheckBoxClicked();
    void mimeButtonClicked();

private Q_SLOTS:

    void slotHelp();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
