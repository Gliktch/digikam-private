/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-01
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2010      by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2025 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRadioButton>
#include <QDateTimeEdit>
#include <QDialog>
#include <QComboBox>
#include <QCloseEvent>

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace DigikamGenericImageShackPlugin
{

class ImageShackNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit ImageShackNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~ImageShackNewAlbumDlg() override = default;

    QString getAlbumTitle();
};

} // namespace DigikamGenericImageShackPlugin
