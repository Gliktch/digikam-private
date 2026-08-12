/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendingmanager.h"

// Local includes

#include "expoblendingwizard.h"
#include "expoblendingdlg.h"
#include "expoblendingthread.h"
#include "alignbinary.h"
#include "enfusebinary.h"

namespace DigikamGenericExpoBlendingPlugin
{

class Q_DECL_HIDDEN ExpoBlendingManager::Private
{
public:

    Private() = default;

    QList<QUrl>             inputUrls;

    ExpoBlendingItemUrlsMap preProcessedUrlsMap;

    ExpoBlendingThread*     thread      = nullptr;

    DPlugin*                plugin      = nullptr;

    AlignBinary             alignBinary;
    EnfuseBinary            enfuseBinary;

    ExpoBlendingWizard*     wizard      = nullptr;
    ExpoBlendingDlg*        dlg         = nullptr;
};

QPointer<ExpoBlendingManager> ExpoBlendingManager::internalPtr = QPointer<ExpoBlendingManager>();

ExpoBlendingManager::ExpoBlendingManager(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->thread = new ExpoBlendingThread(this);

    connect(&d->enfuseBinary, SIGNAL(signalEnfuseVersion(double)),
            this, SLOT(slotSetEnfuseVersion(double)));

    if (d->enfuseBinary.isValid())
    {
        slotSetEnfuseVersion(d->enfuseBinary.getVersion());
    }
}

ExpoBlendingManager::~ExpoBlendingManager()
{
    delete d->thread;
    delete d->wizard;
    delete d->dlg;
    delete d;
}

ExpoBlendingManager* ExpoBlendingManager::instance()
{
    if (ExpoBlendingManager::internalPtr.isNull())
    {
        ExpoBlendingManager::internalPtr = new ExpoBlendingManager();
    }

    return ExpoBlendingManager::internalPtr;
}

bool ExpoBlendingManager::isCreated()
{
    return (!internalPtr.isNull());
}

bool ExpoBlendingManager::checkBinaries()
{
    if (!d->alignBinary.recheckDirectories())
    {
        return false;
    }

    if (!d->enfuseBinary.recheckDirectories())
    {
        return false;
    }

    return true;
}

AlignBinary& ExpoBlendingManager::alignBinary() const
{
    return d->alignBinary;
}

EnfuseBinary& ExpoBlendingManager::enfuseBinary() const
{
    return d->enfuseBinary;
}

void ExpoBlendingManager::setItemsList(const QList<QUrl>& urls)
{
    d->inputUrls = urls;
}

QList<QUrl>& ExpoBlendingManager::itemsList() const
{
    return d->inputUrls;
}

void ExpoBlendingManager::setPlugin(DPlugin* const plugin)
{
    d->plugin = plugin;
}

void ExpoBlendingManager::setPreProcessedMap(const ExpoBlendingItemUrlsMap& urls)
{
    d->preProcessedUrlsMap = urls;
}

ExpoBlendingItemUrlsMap& ExpoBlendingManager::preProcessedMap() const
{
    return d->preProcessedUrlsMap;
}

ExpoBlendingThread* ExpoBlendingManager::thread() const
{
    return d->thread;
}

void ExpoBlendingManager::run()
{
    startWizard();
}

void ExpoBlendingManager::cleanUp()
{
    d->thread->cleanUpResultFiles();
}

void ExpoBlendingManager::startWizard()
{
    if      (d->wizard && (d->wizard->isMinimized() || !d->wizard->isHidden()))
    {
        d->wizard->showNormal();       // krazy:exclude=qmethods
        d->wizard->activateWindow();
        d->wizard->raise();
    }
    else if (d->dlg && (d->dlg->isMinimized() || !d->dlg->isHidden()))
    {
        d->dlg->showNormal();          // krazy:exclude=qmethods
        d->dlg->activateWindow();
        d->dlg->raise();
    }
    else
    {
        delete d->wizard;
        delete d->dlg;
        d->dlg = nullptr;

        d->wizard = new ExpoBlendingWizard(this);
        d->wizard->setPlugin(d->plugin);

        connect(d->wizard, SIGNAL(accepted()),
                this, SLOT(slotStartDialog()));

        d->wizard->show();
    }
}

void ExpoBlendingManager::slotStartDialog()
{
    d->inputUrls = d->wizard->itemUrls();

    d->dlg = new ExpoBlendingDlg(this);
    d->dlg->show();
}

void ExpoBlendingManager::slotSetEnfuseVersion(double version)
{
    d->thread->setEnfuseVersion(version);
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendingmanager.cpp"
