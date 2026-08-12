/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : stand alone test application for plugin configuration view.
 *
 * SPDX-FileCopyrightText: 2018-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt Includes

#include <QApplication>

// Local includes

#include "dpluginloader.h"
#include "dpluginsetup.h"
#include "dpluginconfviewgeneric.h"
#include "digikam_debug.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    DPluginLoader* const dpl = DPluginLoader::instance();
    dpl->init();
    dpl->registerGenericPlugins(qApp);

    DPluginSetup view;
    view.setPluginConfView(new DPluginConfViewGeneric(&view));
    view.show();
    view.resize(1024, 640);

    app.exec();

    view.applySettings();

    return 0;
}
