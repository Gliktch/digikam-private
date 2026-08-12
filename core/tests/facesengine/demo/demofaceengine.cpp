/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : GUI demo program for FacesEngine
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Alex Jironkin <alexjironkin at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>

// Local includes

#include "demomainwindow.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;
using namespace FaceEngineDemo;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.
    DbEngineParameters prm = DbEngineParameters::parametersFromConfig();
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);

    MainWindow w;
    w.show();

    return a.exec();
}
