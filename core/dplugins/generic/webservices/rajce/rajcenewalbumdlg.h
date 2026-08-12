/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "wsnewalbumdialog.h"

class QCheckBox;

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

class RajceNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit RajceNewAlbumDlg(QWidget* const parent = nullptr);
    ~RajceNewAlbumDlg() override;

public:

    QString albumName()        const;
    QString albumDescription() const;
    bool    albumVisible()     const;

private:

    QCheckBox* m_albumVisible = nullptr;
};

} // namespace DigikamGenericRajcePlugin
