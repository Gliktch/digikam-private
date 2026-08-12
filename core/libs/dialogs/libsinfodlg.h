/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : shared libraries list dialog common to digiKam and Showfoto
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMap>
#include <QTreeWidgetItem>

// Local includes

#include "digikam_export.h"
#include "infodlg.h"

namespace cv::ocl
{
    class Device;
}

namespace Digikam
{

class DIGIKAM_EXPORT LibsInfoDlg : public InfoDlg
{
    Q_OBJECT

public:

    explicit LibsInfoDlg(QWidget* const parent);
    ~LibsInfoDlg();

protected:

    QTreeWidgetItem* m_features   = nullptr;
    QTreeWidgetItem* m_libraries  = nullptr;
    QTreeWidgetItem* m_buildtools = nullptr;

protected:

    void populateFeatures();
    void populateLibraries();
    void populateBuildTools();
    void populateManifest();
    void populateOpenCV();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
