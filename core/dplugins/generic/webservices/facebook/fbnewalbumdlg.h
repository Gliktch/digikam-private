/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QDialog>

// Local includes

#include "wsnewalbumdialog.h"

class QComboBox;

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

class FbAlbum;

class FbNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit FbNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~FbNewAlbumDlg();

    void getAlbumProperties();
    void getAlbumProperties(FbAlbum& album);

private:

    QComboBox* m_privacyCoB;

    friend class FbWindow;
};

} // namespace DigikamGenericFaceBookPlugin
