/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Camera management
 *
 * SPDX-FileCopyrightText: 2002-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::autoDetect()
{
    // Called from main if command line option is set, or via DBus

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Auto-Detecting Camera..."));
    }

    QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
}

void DigikamApp::downloadFrom(const QString& cameraGuiPath)
{
    // Called from main if command line option is set, or via DBus

    if (!cameraGuiPath.isEmpty())
    {
        if (d->splashScreen)
        {
            d->splashScreen->setMessage(i18n("Opening Download Dialog..."));
        }

        Q_EMIT queuedOpenCameraUiFromPath(cameraGuiPath);
    }
}

void DigikamApp::downloadFromUdi(const QString& udi)
{
    // Called from main if command line option is set, or via DBus

    if (!udi.isEmpty())
    {
        if (d->splashScreen)
        {
            d->splashScreen->setMessage(i18n("Opening Download Dialog..."));
        }

        Q_EMIT queuedOpenSolidDevice(udi);
    }
}

void DigikamApp::loadCameras()
{
    KActionCollection* const ac = actionCollection();

    d->cameraMenu->setTitle(i18n("Cameras"));
    d->cameraMenu->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));
    d->usbMediaMenu->setTitle(i18n("USB Storage Devices"));
    d->usbMediaMenu->setIcon(QIcon::fromTheme(QLatin1String("drive-removable-media")));
    d->cardReaderMenu->setTitle(i18n("Card Readers"));
    d->cardReaderMenu->setIcon(QIcon::fromTheme(QLatin1String("media-flash-sd-mmc")));

    ac->addAction(QLatin1String("cameras"),     d->cameraMenu->menuAction());
    ac->addAction(QLatin1String("usb_media"),   d->usbMediaMenu->menuAction());
    ac->addAction(QLatin1String("card_reader"), d->cardReaderMenu->menuAction());

    // -----------------------------------------------------------------

    d->addImagesAction = new QAction(QIcon::fromTheme(QLatin1String("document-import")), i18n("Add Images..."), this);
    d->addImagesAction->setWhatsThis(i18n("Adds new items to an Album."));
    connect(d->addImagesAction, SIGNAL(triggered()), this, SLOT(slotImportAddImages()));
    ac->addAction(QLatin1String("import_addImages"), d->addImagesAction);
    ac->setDefaultShortcut(d->addImagesAction, Qt::ALT | Qt::SHIFT | Qt::Key_I);

    // -----------------------------------------------------------------

    d->addFoldersAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")), i18n("Add Folders..."), this);
    d->addFoldersAction->setWhatsThis(i18n("Adds new folders to Album library."));
    connect(d->addFoldersAction, SIGNAL(triggered()), this, SLOT(slotImportAddFolders()));
    ac->addAction(QLatin1String("import_addFolders"), d->addFoldersAction);

    // -- fill manually added cameras ----------------------------------

    d->cameraList->load();

    // -- scan Solid devices -------------------------------------------

    fillSolidMenus();

    connectToSolidNotifiers();
}

void DigikamApp::updateCameraMenu()
{
    d->cameraMenu->clear();
    const auto acs = d->solidCameraActionGroup->actions();

    for (QAction* const action : acs)
    {
        d->cameraMenu->addAction(action);
    }

    d->cameraMenu->addSeparator();

    const auto macs = d->manualCameraActionGroup->actions();

    for (QAction* const action : macs)
    {
        // remove duplicate entries, prefer manually added cameras

        const auto sacs = d->solidCameraActionGroup->actions();

        for (QAction* const actionSolid : sacs)
        {
            if (CameraNameHelper::sameDevices(actionSolid->iconText(), action->iconText()))
            {
                d->cameraMenu->removeAction(actionSolid);
                d->solidCameraActionGroup->removeAction(actionSolid);
            }
        }

        d->cameraMenu->addAction(action);
    }

    d->cameraMenu->addSeparator();
    d->cameraMenu->addAction(actionCollection()->action(QLatin1String("camera_add")));
}

void DigikamApp::slotSetupCamera()
{
    Setup::execSinglePage(this, Setup::CameraPage);
}

void DigikamApp::slotOpenManualCamera(QAction* action)
{
    CameraType* const ctype = d->cameraList->find(action->data().toString());

    if (ctype)
    {
        // check not to open two dialogs for the same camera

        if (ctype->currentImportUI() && !ctype->currentImportUI()->isClosed())
        {
            // show and raise dialog

            ctype->currentImportUI()->unminimizeAndActivateWindow();
        }
        else
        {
            // the ImportUI will delete itself when it has finished

            ImportUI* const cgui = new ImportUI(ctype->title(), ctype->model(),
                                                ctype->port(), ctype->path(), ctype->startingNumber());

            ctype->setCurrentImportUI(cgui);

            cgui->show();

            connect(cgui, SIGNAL(signalLastDestination(QUrl)),
                    d->view, SLOT(slotSelectAlbum(QUrl)));
        }
    }
}

void DigikamApp::slotCameraAdded(CameraType* ctype)
{
    if (!ctype)
    {
        return;
    }

    QAction* const cAction = new QAction(QIcon::fromTheme(QLatin1String("camera-photo")),
                                         ctype->title(), d->manualCameraActionGroup);
    cAction->setData(ctype->title());
    actionCollection()->addAction(ctype->title(), cAction);

    ctype->setAction(cAction);
    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotCameraRemoved(QAction* cAction)
{
    if (cAction)
    {
        d->manualCameraActionGroup->removeAction(cAction);
    }

    updateCameraMenu();
    updateQuickImportAction();
}

void DigikamApp::slotCameraAutoDetect()
{
    bool retry                    = false;
    const CameraType* const ctype = d->cameraList->autoDetect(retry);

    if (!ctype && retry)
    {
        QTimer::singleShot(0, this, SLOT(slotCameraAutoDetect()));
        return;
    }

    if (ctype && ctype->action())
    {
        ctype->action()->activate(QAction::Trigger);
    }
}

void DigikamApp::slotOpenCameraUiFromPath(const QString& path)
{
    if (path.isEmpty())
    {
        return;
    }

    // the ImportUI will delete itself when it has finished

    ImportUI* const cgui = new ImportUI(i18n("Images found in %1", path),
                                        QLatin1String("directory browse"),
                                        QLatin1String("Fixed"), path, 1);
    cgui->show();

    connect(cgui, SIGNAL(signalLastDestination(QUrl)),
            d->view, SLOT(slotSelectAlbum(QUrl)));
}

void DigikamApp::downloadImages(const QString& folder)
{
    if (!folder.isNull())
    {
        // activate window when called by media menu and DCOP

        unminimizeAndActivateWindow();

        Q_EMIT queuedOpenCameraUiFromPath(folder);
    }
}

void DigikamApp::cameraAutoDetect()
{
    // activate window when called by media menu and DCOP

    unminimizeAndActivateWindow();

    slotCameraAutoDetect();
}

} // namespace Digikam
