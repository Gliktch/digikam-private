/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-01
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2010      by Jens Mueller <tschenser at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

namespace DigikamGenericGoogleServicesPlugin
{

class GSFolder;

class GSNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit GSNewAlbumDlg(QWidget* const parent,
                           const QString& serviceName,
                           const QString& toolName);
    ~GSNewAlbumDlg() override;

    void getAlbumProperties(GSFolder& album);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericGoogleServicesPlugin
