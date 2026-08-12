/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-07
 * Description : a tool to export images to Smugmug web service
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
#include <QGroupBox>
#include <QComboBox>

namespace DigikamGenericSmugPlugin
{

class SmugAlbum;

class SmugNewAlbumDlg : public QDialog
{
    Q_OBJECT

public:

    explicit SmugNewAlbumDlg(QWidget* const parent);
    ~SmugNewAlbumDlg() override;

    void getAlbumProperties(SmugAlbum& album);

/*
    Categories are deprecated
    QComboBox* categoryCombo()    const;
    QComboBox* subCategoryCombo() const;
*/

    QComboBox* templateCombo()    const;
    QGroupBox* privateGroupBox()  const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericSmugPlugin
