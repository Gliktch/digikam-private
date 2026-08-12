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

#include "imageshacknewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QIcon>
#include <QApplication>
#include <QPushButton>

namespace DigikamGenericImageShackPlugin
{

ImageShackNewAlbumDlg::ImageShackNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

QString ImageShackNewAlbumDlg::getAlbumTitle()
{
    return getTitleEdit()->text();
}

} // namespace DigikamGenericImageShackPlugin

#include "moc_imageshacknewalbumdlg.cpp"
