/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class - Services methods.
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "contextmenuhelper_p.h"

namespace Digikam
{

void ContextMenuHelper::addServicesMenu(const QList<QUrl>& selectedItems)
{
    setSelectedItems(selectedItems);

#ifdef Q_OS_MAC

    QList<QUrl> appUrls = DServiceMenu::MacApplicationsForFiles(selectedItems);

    if (!appUrls.isEmpty())
    {
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action: context menu", "Open With"));

        for (const QUrl& aurl : std::as_const(appUrls))
        {
            QAction* const action = servicesMenu->addAction(DServiceMenu::MacApplicationBundleName(aurl));
            action->setIcon(DServiceMenu::MacApplicationBundleIcon(aurl));
            action->setData(aurl);
        }

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }

#else // LINUX and WINDOWS

#   ifdef HAVE_KIO

    KService::List offers = DServiceMenu::servicesForOpenWith(selectedItems);

    if (!offers.isEmpty())
    {
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action: context menu", "Open With"));

        for (const KService::Ptr& service : std::as_const(offers))
        {
            QString name          = service->name().replace(QLatin1Char('&'), QLatin1String("&&"));
            QAction* const action = servicesMenu->addAction(name);
            action->setIcon(QIcon::fromTheme(service->icon()));
            action->setData(service->name());
            d->servicesMap[name]  = service;
        }

        servicesMenu->addSeparator();
        servicesMenu->addAction(i18nc("@action: open item with other application", "Other..."));

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }
    else
    {
        QAction* const serviceAction = new QAction(i18nc("@action: context menu", "Open With..."), this);
        addAction(serviceAction);

        connect(serviceAction, SIGNAL(triggered()),
                this, SLOT(slotOpenWith()));
    }

#   else

    QList<DServiceInfo> offers = DServiceMenu::servicesForOpen(selectedItems);

    if (!offers.isEmpty())
    {
        QMenu* const servicesMenu    = new QMenu(d->parent);
        qDeleteAll(servicesMenu->actions());

        QAction* const serviceAction = servicesMenu->menuAction();
        serviceAction->setText(i18nc("@action: context menu", "Open With"));

        for (const DServiceInfo& sinfo : std::as_const(offers))
        {
            QAction* const action = servicesMenu->addAction(sinfo.name);
            action->setIcon(DServiceMenu::getIconFromService(sinfo));
            action->setData(sinfo.name);
            d->newServicesMap[sinfo.name] = sinfo;
        }

#   ifdef Q_OS_WIN

        if (selectedItems.size() == 1)
        {
            servicesMenu->addSeparator();
            servicesMenu->addAction(i18nc("@action: open item with other application", "Other..."));
        }

#   endif // Q_OS_WIN

        addAction(serviceAction);

        connect(servicesMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotOpenWith(QAction*)));
    }

#   ifdef Q_OS_WIN

    else
    {
        if (selectedItems.size() == 1)
        {
            QAction* const serviceAction = new QAction(i18nc("@action: context menu", "Open With..."), this);
            addAction(serviceAction);

            connect(serviceAction, SIGNAL(triggered()),
                    this, SLOT(slotOpenWith()));
        }
    }

#   endif // Q_OS_WIN

#   endif // HAVE_KIO

#endif // Q_OS_MAC

}

void ContextMenuHelper::slotOpenWith()
{
    // call the slot with an "empty" action

    slotOpenWith(nullptr);
}

void ContextMenuHelper::slotOpenWith(QAction* action)
{

#ifdef Q_OS_MAC

    QList<QUrl> list = d->selectedItems;
    QUrl aurl        = action ? action->data().toUrl() : QUrl();

    if (!aurl.isEmpty())
    {
        DServiceMenu::MacOpenFilesWithApplication(list, aurl);
    }

#else // LINUX and WINDOWS

    QList<QUrl> list = d->selectedItems;
    QString name     = action ? action->data().toString() : QString();

#   ifdef HAVE_KIO

    KService::Ptr service;

    if (name.isEmpty())
    {
        QPointer<KOpenWithDialog> dlg = new KOpenWithDialog(list);

        if (dlg->exec() != KOpenWithDialog::Accepted)
        {
            delete dlg;

            return;
        }

        service = dlg->service();

        if (!service)
        {
            // User entered a custom command

            if (!dlg->text().isEmpty())
            {
                DServiceMenu::runFiles(dlg->text(), list);
            }

            delete dlg;

            return;
        }

        delete dlg;
    }
    else
    {
        service = d->servicesMap[name];
    }

    DServiceMenu::runFiles(service, list);

#   else

    if (name.isEmpty())
    {

#   ifdef Q_OS_WIN

        // See Bug #380065 for details.

        if (list.size() == 1)
        {
            d->lpVerbWin          = QLatin1String("openas");
            d->lpFileWin          = QDir::toNativeSeparators(list.first().toLocalFile());

            SHELLEXECUTEINFOW sei = {};
            sei.cbSize            = sizeof(sei);
            sei.fMask             = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOASYNC;
            sei.nShow             = SW_SHOWNORMAL;
            sei.lpVerb            = (LPCWSTR)d->lpVerbWin.utf16();
            sei.lpFile            = (LPCWSTR)d->lpFileWin.utf16();

            ShellExecuteEx(&sei);

            qCDebug(DIGIKAM_GENERAL_LOG) << "ShellExecuteEx::openas return code:" << GetLastError();
        }

#   endif // Q_OS_WIN

        return;
    }

    DServiceMenu::runFiles(d->newServicesMap[name], list);

#   endif // HAVE_KIO

#endif // Q_OS_MAC

}

void ContextMenuHelper::slotOpenInFileManager()
{
    DFileOperations::openInFileManager(d->selectedItems);
}

} // namespace Digikam
