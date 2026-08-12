/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMainWindow>
#include <QMessageBox>

// Local includes

#include "mediawiki_edit.h"
#include "mediawiki_iface.h"
#include "mediawiki_login.h"
#include "mediawiki_queryinfo.h"
#include "mediawiki_queryrevision.h"

namespace Ui
{
    class MainWindow;
}

using namespace MediaWiki;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget* const parent = nullptr);
    ~MainWindow() override;

private Q_SLOTS:

    void slot_plainTextEdit_textChanged();
    void slot_mPageEdit_textChanged(const QString&);
    void slot_mWikiEdit_textChanged(const QString&);
    void slot_pushButton1_clicked();
    void slot_pushButton2_clicked();
    void loginHandle(KJob* login);
    void revisionHandle(const QList<Revision>& revisions);
    void editError(KJob* job);
    void revisionError(KJob* job);

private:

    Ui::MainWindow* ui      = nullptr;
    Iface*          mwIface = nullptr;
};
