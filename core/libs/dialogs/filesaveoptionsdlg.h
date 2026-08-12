/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-14
 * Description : a dialog to display image file save options.
 *
 * SPDX-FileCopyrightText:      2009 by David Eriksson <meldavid at acc umu se>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "filesaveoptionsbox.h"

class QWidget;

namespace Digikam
{

class DIGIKAM_EXPORT FileSaveOptionsDlg : public QDialog
{
    Q_OBJECT

public:

    FileSaveOptionsDlg(QWidget* const parent, FileSaveOptionsBox* const options);
    ~FileSaveOptionsDlg() = default;
};

} // namespace Digikam
