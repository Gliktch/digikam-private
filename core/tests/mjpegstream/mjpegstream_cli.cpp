/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-29
 * Description : stand alone test for MJPEG Server
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QMap>
#include <QProgressDialog>

// Local includes

#include "digikam_debug.h"
#include "dfiledialog.h"
#include "mjpegservermngr.h"
#include "metaengine.h"
#include "dpluginloader.h"

using namespace Digikam;
using namespace DigikamGenericMjpegStreamPlugin;

int main(int argc, char* argv[])
{
    QApplication   app(argc, argv);
    QList<QUrl>    list;
    MjpegServerMap map;

    MetaEngine::initializeExiv2();
    DPluginLoader::instance()->init();

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if (argc <= 1)
    {
        QStringList files = DFileDialog::getOpenFileNames(nullptr, QString::fromLatin1("Select Files to Share With Media Server"),
                                                          QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).constFirst(),
                                                          QLatin1String("Image Files (*.png *.jpg *.tif *.bmp *.gif)"));

        for (const QString& f : std::as_const(files))
        {
            list.append(QUrl::fromLocalFile(f));
        }
    }
    else
    {
        for (int i = 1 ; i < argc ; i++)
        {
            list.append(QUrl::fromLocalFile(QString::fromLocal8Bit(argv[i])));
        }
    }

    if (!list.isEmpty())
    {
        map.insert(QLatin1String("Test Collection"), list);
        MjpegServerMngr::instance()->setCollectionMap(map);
    }
    else
    {
        if (!MjpegServerMngr::instance()->load())
        {
            DPluginLoader::instance()->cleanUp();

            return -1;
        }
    }

    if (MjpegServerMngr::instance()->startMjpegServer())
    {
        QProgressDialog* const pdlg = new QProgressDialog(nullptr);
        pdlg->setLabelText(QLatin1String("Sharing files on the network"));
        pdlg->setMinimumDuration(0);
        pdlg->setCancelButtonText(QLatin1String("Close"));
        pdlg->setMaximum(0);
        pdlg->setMinimum(0);
        pdlg->setValue(0);
        pdlg->exec();
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Failed to start the MJPEG Server...";
    }

    MjpegServerMngr::instance()->save();
    MjpegServerMngr::instance()->cleanUp();

    DPluginLoader::instance()->cleanUp();

    return 0;
}
